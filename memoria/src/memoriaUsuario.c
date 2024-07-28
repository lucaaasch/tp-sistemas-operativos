#include "../include/memoria.h"

pthread_mutex_t mutexEspacioUsuario;
pthread_mutex_t mutexBitMap;
pthread_mutex_t mutexTablasPaginas;

void crearTablaPaginas(int PID) {
	t_tablaPaginas *tablaPaginas = malloc(sizeof(t_tablaPaginas));
	tablaPaginas->PID = PID;
	tablaPaginas->entradasTablaPaginas = list_create();
	pthread_mutex_lock(&mutexTablasPaginas);
	list_add(tablasPaginas, (void*) tablaPaginas);
	pthread_mutex_unlock(&mutexTablasPaginas);
	log_info(auxLogger, "PID: %d - Tamaño: %d", PID, list_size(tablaPaginas->entradasTablaPaginas));
}

bool elMarcoEstaSiendoUsado(t_entradaTablaPaginas *entrada) {
	return entrada->bitValidez;
}

int* obtenerNumeroMarco(t_entradaTablaPaginas *entrada) {
	return &(entrada->numeroMarco);
}

void liberarMarco(int *marco) {
	bitMap[*marco] = 0;
}

t_tablaPaginas* buscarTablaSegunPID(int PID) {
	t_tablaPaginas* tablaPaginasSeleccionada;
	int listSize = list_size(tablasPaginas);
	pthread_mutex_lock(&mutexTablasPaginas);
	for(int i = 0; i < listSize; i++){
		tablaPaginasSeleccionada = (t_tablaPaginas*) list_get(tablasPaginas,i);
		if(tablaPaginasSeleccionada->PID == PID){
			pthread_mutex_unlock(&mutexTablasPaginas);
			return tablaPaginasSeleccionada;
		}
	}
	pthread_mutex_unlock(&mutexTablasPaginas);
	return NULL;
}

void liberarEspacioUsuario(int PID) {
	t_tablaPaginas *tablaPaginasSeleccionada = buscarTablaSegunPID(PID);
	if (list_size(tablaPaginasSeleccionada->entradasTablaPaginas) > 0) {
		t_list *entradasFiltradas = list_filter(tablaPaginasSeleccionada->entradasTablaPaginas, (void*) elMarcoEstaSiendoUsado);
		t_list *marcosUtilizados = list_map(entradasFiltradas, (void*) obtenerNumeroMarco);
		pthread_mutex_lock(&mutexBitMap);
		list_iterate(marcosUtilizados, (void*) liberarMarco);
		pthread_mutex_unlock(&mutexBitMap);
		list_destroy(entradasFiltradas);
		list_destroy(marcosUtilizados);
	}
	pthread_mutex_lock(&mutexTablasPaginas);
	list_remove_element(tablasPaginas, tablaPaginasSeleccionada);
	pthread_mutex_unlock(&mutexTablasPaginas);
	list_clean_and_destroy_elements(tablaPaginasSeleccionada->entradasTablaPaginas, (void*) free);
	log_info(auxLogger, "PID: %d - Tamaño: %d", PID, list_size(tablaPaginasSeleccionada->entradasTablaPaginas));
	list_destroy(tablaPaginasSeleccionada->entradasTablaPaginas);
	free(tablaPaginasSeleccionada);
}

int obtenerNumeroMarcoAccediendoATablaPaginas(int PID, int numeroPagina) {
	t_tablaPaginas *tablaPaginasSeleccionada = buscarTablaSegunPID(PID);
	t_entradaTablaPaginas *entradaSeleccionada = list_get(tablaPaginasSeleccionada->entradasTablaPaginas, numeroPagina);
	int numeroMarco = entradaSeleccionada->numeroMarco;
	log_info(auxLogger, "PID: %d - Pagina: %d - Marco: %d", PID, numeroPagina, numeroMarco);
	return numeroMarco;
}

int obtenerCantidadMarcosDisponibles() {
	int contadorMarcos = 0;
	int cantidadMarcosTotal = floor((double)cfgMemoria->TAM_MEMORIA / cfgMemoria->TAM_PAGINA);
	pthread_mutex_lock(&mutexBitMap);
	for (int i = 0; i < cantidadMarcosTotal; i++) {
		if (!bitMap[i]) {
			contadorMarcos++;
		}
	}
	pthread_mutex_unlock(&mutexBitMap);
	return contadorMarcos;
}

int obtenerCantidadMarcosUtilizadosPorProceso(t_tablaPaginas *tablaPaginas) {
	int marcosUtilizados = 0;
	if (list_size(tablaPaginas->entradasTablaPaginas) > 0) {
		t_list *entradasFiltradas = list_filter(tablaPaginas->entradasTablaPaginas, (void*) elMarcoEstaSiendoUsado);
		marcosUtilizados = list_size(entradasFiltradas);
		list_destroy(entradasFiltradas);
	}
	return marcosUtilizados;
}

void reducirTamanio(t_tablaPaginas *tablaPaginas, int marcosAReducir) {
	log_info(auxLogger, "PID: %d - Tamaño Actual: %d - Tamaño a Reducir: %d", tablaPaginas->PID,
			list_size(tablaPaginas->entradasTablaPaginas), marcosAReducir);
	for (int i = list_size(tablaPaginas->entradasTablaPaginas) - 1; marcosAReducir > 0; i--) {
		t_entradaTablaPaginas *entrada = list_get(tablaPaginas->entradasTablaPaginas, i);
		if (entrada->bitValidez) {
			liberarMarco(&(entrada->numeroMarco));
			entrada->bitValidez = 0;
			marcosAReducir--;
		}
	}
}

bool laPaginaEstaDesreferenciada(t_entradaTablaPaginas *entrada) {
	return !(entrada->bitValidez);
}

int obtenerMarcoLibre() {
	int cantidadMarcosTotal = floor((double)cfgMemoria->TAM_MEMORIA / cfgMemoria->TAM_PAGINA);
	for (int i = 0; i < cantidadMarcosTotal; i++) {
		if (!bitMap[i]) {
			return i;
		}
	}
	return -1;
}

t_entradaTablaPaginas* descubrirEntradaADireccionar(t_tablaPaginas *tablaPaginas) {
	t_entradaTablaPaginas *entrada = list_find(tablaPaginas->entradasTablaPaginas, (void*) laPaginaEstaDesreferenciada);
	if (entrada == NULL) {
		entrada = malloc(sizeof(t_entradaTablaPaginas));
		list_add(tablaPaginas->entradasTablaPaginas, entrada);
	}
	return entrada;
}

void asignarMarco(int marcoLibre, t_entradaTablaPaginas *entrada) {
	bitMap[marcoLibre] = 1;
	entrada->bitValidez = 1;
	entrada->numeroMarco = marcoLibre;
}

bool esPosibleAmpliar(int marcosDisponibles, int marcosAAmpliar) {
	return marcosDisponibles >= marcosAAmpliar;
}

bool ampliarTamanio(t_tablaPaginas *tablaPaginas, int marcosAAmpliar) {
	log_info(auxLogger, "PID: %d - Tamaño Actual: %d - Tamaño a Ampliar: %d", tablaPaginas->PID,
			list_size(tablaPaginas->entradasTablaPaginas), marcosAAmpliar);
	int marcosDisponibles = obtenerCantidadMarcosDisponibles();
	bool huboExito = 0;
	if (esPosibleAmpliar(marcosDisponibles, marcosAAmpliar)) {
		for (int i = 0; i < marcosAAmpliar; i++) {
			t_entradaTablaPaginas *entrada = descubrirEntradaADireccionar(tablaPaginas);
			int marcoLibre = obtenerMarcoLibre();
			asignarMarco(marcoLibre, entrada);
		}
		huboExito = 1;
	} else {
		huboExito = 0;
	}
	return huboExito;
}

bool redimensionar(int PID, int tamanio) {
	bool huboExito = 0;
	t_tablaPaginas *tablaPaginasSeleccionada = buscarTablaSegunPID(PID);
	int tamanioEnMarcos = ceil((double)tamanio / cfgMemoria->TAM_PAGINA);
	int marcosUtilizadosPorProceso = obtenerCantidadMarcosUtilizadosPorProceso(tablaPaginasSeleccionada);
	if (tamanioEnMarcos < marcosUtilizadosPorProceso) {
		reducirTamanio(tablaPaginasSeleccionada, marcosUtilizadosPorProceso - tamanioEnMarcos);
		huboExito = 1;
	} else if (tamanioEnMarcos > marcosUtilizadosPorProceso) {
		huboExito = ampliarTamanio(tablaPaginasSeleccionada, tamanioEnMarcos - marcosUtilizadosPorProceso);
	} else {
		huboExito = 1;
	}
	return huboExito;
}

int obtenerCantidadBytesAAcceder(int direccionFisica, int tamanioLectura) {
	int numeroMarco = floor((double)direccionFisica / cfgMemoria->TAM_PAGINA);
	int desplazamiento = direccionFisica - numeroMarco * cfgMemoria->TAM_PAGINA;
	int tamanioALeer = cfgMemoria->TAM_PAGINA - desplazamiento;
	if (tamanioALeer > tamanioLectura) {
		tamanioALeer = tamanioLectura;
	}
	return tamanioALeer;
}

/**
 * Se accede al espacio de memoria de usuario para realizar una lectura o escritura. El modo acceso en 1 significa lectura, en 0 escritura.
 */
bool accederAMemoria(int *direccionesFisicas, int cantidadDirecciones, int tamanioAcceso, void *buffer, bool modoAcceso) {
	int tamanioAccesoAux = tamanioAcceso;
	int tamanioAccedido = 0;
	for (int indiceDireccion = 0; tamanioAccesoAux > 0 && indiceDireccion < cantidadDirecciones; indiceDireccion++) {
		int direccionFisica = direccionesFisicas[indiceDireccion];
		int tamanioAAcceder = obtenerCantidadBytesAAcceder(direccionFisica, tamanioAccesoAux);
		pthread_mutex_lock(&mutexEspacioUsuario);
		if (modoAcceso) {
			memcpy(buffer + tamanioAccedido, espacioUsuario + direccionFisica, tamanioAAcceder);
		} else {
			memcpy(espacioUsuario + direccionFisica, buffer + tamanioAccedido, tamanioAAcceder);
		}
		pthread_mutex_unlock(&mutexEspacioUsuario);
		tamanioAccedido += tamanioAAcceder;
		tamanioAccesoAux -= tamanioAAcceder;
	}
	return tamanioAccedido == tamanioAcceso;

}

bool leerDeMemoria(int *direccionesFisicas, int cantidadDirecciones, int tamanioLectura, void *buffer) {
	const bool MODO_LECTURA = 1;
	return accederAMemoria(direccionesFisicas, cantidadDirecciones, tamanioLectura, buffer, MODO_LECTURA);
}

bool escribirEnMemoria(int *direccionesFisicas, int cantidadDirecciones, int tamanioEscritura, void *buffer) {
	const bool MODO_ESCRITURA = 0;
	return accederAMemoria(direccionesFisicas, cantidadDirecciones, tamanioEscritura, buffer, MODO_ESCRITURA);
}

