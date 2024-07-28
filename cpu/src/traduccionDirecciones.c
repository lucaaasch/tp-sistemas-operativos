#include "../include/cpu.h"

t_list *tlb;

void agregarPorFIFO(entradaTLB *nuevaEntrada) {

	int cantidadEntradasTLB = cfgCpu->CANTIDAD_ENTRADAS_TLB;

	if (list_size(tlb) == cantidadEntradasTLB && cantidadEntradasTLB != 0) {
		entradaTLB *primerEntrada = list_remove(tlb, 0);
		if (primerEntrada != NULL) {
			free(primerEntrada);
		}
	}

	list_add(tlb, nuevaEntrada);
}

entradaTLB* entradaConMayorTiempo = NULL;

bool encontrarVictima(entradaTLB* victima){

	if(entradaConMayorTiempo == NULL || victima->ultimoAcceso < entradaConMayorTiempo->ultimoAcceso){
		entradaConMayorTiempo = victima;
	}
	return false;
}

void agregarPorLRU(entradaTLB *nuevaEntrada) {
	int cantidadEntradasTLB = cfgCpu->CANTIDAD_ENTRADAS_TLB;

	if (list_size(tlb) == cantidadEntradasTLB && cantidadEntradasTLB != 0) {
		entradaConMayorTiempo = NULL;
		list_find(tlb, (void*) encontrarVictima);

		if(entradaConMayorTiempo != NULL){
			list_remove_element(tlb, entradaConMayorTiempo);
			free(entradaConMayorTiempo);
		}
	}

	list_add(tlb, nuevaEntrada);
}

void agregarEntrada(int PID, int pagina, int marco) {

	entradaTLB *entradaNueva = malloc(sizeof(entradaTLB));
	entradaNueva->PID = PID;
	entradaNueva->pagina = pagina;
	entradaNueva->marco = marco;
	entradaNueva->ultimoAcceso = time(NULL);

	if (strcmp(cfgCpu->ALGORITMO_TLB, "FIFO") == 0) {
		agregarPorFIFO(entradaNueva);
	} else if (strcmp(cfgCpu->ALGORITMO_TLB, "LRU") == 0) {
		agregarPorLRU(entradaNueva);
	}
}

int elementosABuscar[2];

bool compararEntradas(void *elemento) {
	entradaTLB *entrada = (entradaTLB*) elemento;

	return (entrada->PID == elementosABuscar[0] && entrada->pagina == elementosABuscar[1]);
}

int buscarEnTLB(int PID, int pagina) {

	elementosABuscar[0] = PID;
	elementosABuscar[1] = pagina;

	entradaTLB *entrada = list_find(tlb, compararEntradas);

	if (entrada != NULL) {
		log_info(auxLogger, "PID: %d - TLB HIT - Pagina: %d", PID, pagina);

		entrada->ultimoAcceso = time(NULL); // Actualizo el timestamp
		return entrada->marco;
	} else {
		log_info(auxLogger, "PID: %d - TLB MISS - Pagina: %d", PID, pagina);
		return TLB_MISS;
	}
}

// MMU

void solicitarNumeroMarco(int numeroPagina) {

	int PID = contexto->PID;

	t_paquete *paqueteMarco = crearPaquete(SOLICITUD_NUMERO_MARCO);

	agregarAPaquete(paqueteMarco, &PID, sizeof(int));
	agregarAPaquete(paqueteMarco, &numeroPagina, sizeof(int));
	enviarPaquete(paqueteMarco, socketClienteMemoria);
	eliminarPaquete(paqueteMarco);
}

int recibirNumeroMarco() {

	int numeroMarco = 0;

	recibirOperacion(socketClienteMemoria);

	t_buffer *buffer = crearBuffer();

	buffer->stream = recibirBuffer(&(buffer->size), socketClienteMemoria);

	memcpy(&numeroMarco, buffer->stream, sizeof(int));

	liberarBuffer(buffer);

	return numeroMarco;
}

int obtenerDireccionFisica(int PID, int numeroPagina, int desplazamiento) {
	int numeroMarco = 0;
	if (cfgCpu->CANTIDAD_ENTRADAS_TLB != 0) {
		numeroMarco = buscarEnTLB(PID, numeroPagina);
	} else {
		numeroMarco = TLB_MISS;
	}
	if (numeroMarco == TLB_MISS) {
		solicitarNumeroMarco(numeroPagina);
		numeroMarco = recibirNumeroMarco();
		agregarEntrada(contexto->PID, numeroPagina, numeroMarco);
		log_info(auxLogger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d ", PID, numeroPagina, numeroMarco);
	} else {
		log_info(auxLogger, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d ", PID, numeroPagina, numeroMarco);
	}
	int direccionFisica = (numeroMarco * tamPagina) + desplazamiento;
	return direccionFisica;
}

int* traducirDirecciones(int direccionLogica, int sizeLectura, int *cantidadDireccionesFisicas) {

	int PID = contexto->PID;
	int numeroPagina = floor((double)direccionLogica / tamPagina);
	int desplazamiento = direccionLogica - (numeroPagina * tamPagina);

	int capacidadInicial = 5;
	int *direccionesFisicas = malloc(capacidadInicial * sizeof(int));

	int direccionFisica = obtenerDireccionFisica(PID, numeroPagina, desplazamiento);

	direccionesFisicas[0] = direccionFisica;

	sizeLectura -= tamPagina - desplazamiento;

	*cantidadDireccionesFisicas += 1;

	for (int i = 1; sizeLectura > 0; i++) {

		if (*cantidadDireccionesFisicas >= capacidadInicial) {
			capacidadInicial += 5;
			direccionesFisicas = realloc(direccionesFisicas, capacidadInicial * sizeof(int));
		}

		numeroPagina++;
		desplazamiento = 0;
		direccionesFisicas[i] = obtenerDireccionFisica(PID, numeroPagina, desplazamiento);
		*cantidadDireccionesFisicas += 1;
		sizeLectura -= tamPagina;
	}

	return direccionesFisicas;
}

