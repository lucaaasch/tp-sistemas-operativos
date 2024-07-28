#include "../include/consola.h"

bool isPlanificacionPausada = false;
t_list *pcbsEnSistema;
int pidNuevo = 1;
int cantAEsperar = 1;

void separarPalabras(char *linea, char **primeraPalabra, char **segundaPalabra) {
	char *token = strtok(linea, " \n");

	if (token != NULL) {

		*primeraPalabra = strdup(token);

		token = strtok(NULL, " \n");

		if (token != NULL) {
			*segundaPalabra = strdup(token);
		}
	}
}

void leerConsola(char **completo, char **msj, char **param) {
	*completo = readline("> ");
	separarPalabras(*completo, msj, param);
	free(*completo);
}

void liberarMensaje(char *msj) {
	free(msj);
}

void inicializarArrayRecursos(int *array, int tamanio) {
	for (int i = 0; i < tamanio; i++) {
		array[i] = 0;
	}
}

t_pcb* crearPcb() {
	t_pcb *nuevoPcb = malloc(sizeof(t_pcb));
	nuevoPcb->contextoEjecucion = malloc(sizeof(t_contextoEjecucion));
	nuevoPcb->contextoEjecucion->PID = pidNuevo;
	nuevoPcb->contextoEjecucion->PC = 0;
	nuevoPcb->quantum = cfgKernel->QUANTUM;
	nuevoPcb->estado = NEW;
	nuevoPcb->recursosAsignados = malloc(sizeof(int) * contarRecursos(cfgKernel->INSTANCIAS_RECURSOS));
	inicializarArrayRecursos(nuevoPcb->recursosAsignados, contarRecursos(cfgKernel->INSTANCIAS_RECURSOS));
	//CREO REGISTROS
	nuevoPcb->contextoEjecucion->registrosGenerales = malloc(sizeof(t_registrosGenerales));
	nuevoPcb->contextoEjecucion->registrosGenerales->AX = (uint8_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->BX = (uint8_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->CX = (uint8_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->DX = (uint8_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->EAX = (uint32_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->EBX = (uint32_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->ECX = (uint32_t) 0;
	nuevoPcb->contextoEjecucion->registrosGenerales->EDX = (uint32_t) 0;
	//CREOR REGISTROS DE MEMORIA
	nuevoPcb->contextoEjecucion->registrosMemoria = malloc(sizeof(t_registrosMemoria));
	nuevoPcb->contextoEjecucion->registrosMemoria->SI = (uint32_t) 0;
	nuevoPcb->contextoEjecucion->registrosMemoria->DI = (uint32_t) 0;
	pidNuevo++;
	return nuevoPcb;
}

void enviarPathAMemoria(char *path, t_pcb *pcb) {
	t_paquete *paquetePseudocodigo = crearPaquete(CREAR_PROCESO);
	int pathSize = strlen(path);
	agregarAPaquete(paquetePseudocodigo, &(pcb->contextoEjecucion->PID), sizeof(int));
	agregarAPaquete(paquetePseudocodigo, &pathSize, sizeof(int));
	agregarAPaquete(paquetePseudocodigo, path, pathSize * sizeof(char));
	enviarPaquete(paquetePseudocodigo, socketMemoria);
	eliminarPaquete(paquetePseudocodigo);
}

t_mensajeDeConsola descifrarMensaje(char *mensajeDeConsola) {
	if (strcmp(mensajeDeConsola, "INICIAR_PROCESO") == 0) {
		return INICIAR_PROCESO;
	}
	if (strcmp(mensajeDeConsola, "FINALIZAR_PROCESO") == 0) {
		return FINALIZAR_PROCESO;
	}
	if (strcmp(mensajeDeConsola, "DETENER_PLANIFICACION") == 0) {
		return DETENER_PLANIFICACION;
	}
	if (strcmp(mensajeDeConsola, "INICIAR_PLANIFICACION") == 0) {
		return INICIAR_PLANIFICACION;
	}
	if (strcmp(mensajeDeConsola, "PROCESO_ESTADO") == 0) {
		return PROCESO_ESTADO;
	}
	if (strcmp(mensajeDeConsola, "EJECUTAR_SCRIPT") == 0) {
		return EJECUTAR_SCRIPT;
	}
	if (strcmp(mensajeDeConsola, "MULTIPROGRAMACION") == 0) {
		return MULTIPROGRAMACION;
	} else
		return ERROR;
}

void iniciarProceso(char *path) {
	//CREO EL NUEVO PCB
	t_pcb *pcbCreado = crearPcb();
	//agrego el pcb a la lista de pcbs en sistema
	pthread_mutex_lock(&mutexListaPcbsEnSistema);
	list_add(pcbsEnSistema, pcbCreado);
	pthread_mutex_unlock(&mutexListaPcbsEnSistema);
	//ENVIO EL PATH A MEMORIA PARA QUE SE ENCARGUE DE ABRIR EL ARCHIVO DE PSEUDOCODIGO
	enviarPathAMemoria(path, pcbCreado);
	//LO PONGO EN LA COLA DE NEW
	recibirOperacion(socketMemoria);
	ingresarANew(pcbCreado);
}

void iniciarPlanificacion() {
	if (isPlanificacionPausada) {
		isPlanificacionPausada = false;
		sem_post(&semPausaPlanificacionLargo);
		sem_post(&semPausaPlanificacionCorto);
		sem_post(&semPausaMotivoDesalojo);
		sem_post(&semPausaBlockedAReady);
		sem_post(&semPausarReadyAExec);
		sem_post(&semPausarNewAReady);
		sem_post(&semPausarExit);
	}
}

void detenerPlanificacion() {
	isPlanificacionPausada = true;
}

// Devuelve un booleano indicando si encontró y eliminó el PCB
bool buscarYEliminarPcbEnLista(t_list *lista, t_pcb *pcbBuscado) {
	bool pcbEncontrado = false;
	int i;

	// Recorrer la lista para encontrar el PCB buscado
	int listSize = list_size(lista);
	for (i = 0; i < listSize; i++) {
		t_pcb *pcbAux = list_get(lista, i);
		if (pcbBuscado == pcbAux) {
			// Si encontramos el PCB, lo eliminamos
			list_remove(lista, i);
			pcbEncontrado = true;
			break;  // Salimos del bucle ya que hemos encontrado el PCB
		}
	}

	return pcbEncontrado;
}

//devuelve un booleano indicando si encontro el pcb
bool buscarYEliminarPcbEnCola(t_queue *cola, t_pcb *pcbBuscado) {
	t_queue *colaAux = queue_create();
	bool pcbEncontrado = false;
	int sizeCola = queue_size(cola);
	for (int i = 0; i < sizeCola ; i++) {
		t_pcb *pcbAux = queue_pop(cola);
		if (pcbBuscado != pcbAux) {
			queue_push(colaAux, (void*) pcbAux);
		} else {
			pcbEncontrado = true;
		}
	}
	// Pasar elementos de colaAux de vuelta a cola
	while (!queue_is_empty(colaAux)) {
		queue_push(cola, (void*) queue_pop(colaAux));
	}
	queue_destroy(colaAux);
	return pcbEncontrado;
}

t_pcb* buscarPcbEnSistemaPorPid(int pidBuscado) {
	int listSize = list_size(pcbsEnSistema);
	for (int i = 0; i < listSize; i++) {
		t_pcb *pcb = (t_pcb*) list_get(pcbsEnSistema, i);
		if (pcb->contextoEjecucion->PID == pidBuscado) {
			return pcb;
		}
	}
	return NULL;  // No se encontró el elemento
}

void finalizarProceso(char *pidAux) {
	detenerPlanificacion();
	int pid = atoi(pidAux);
	t_pcb *pcb = buscarPcbEnSistemaPorPid(pid);
	bool pcbEncontrado;

	if (pcb != NULL) {
		switch (pcb->estado) {
		case EXEC:
			enviarInterrupcion(INTERRUPTED_BY_USER);
			break;
		case READY:
			pcbEncontrado = false;
			if (strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {
				pcbEncontrado = buscarYEliminarPcbEnCola(colaPrioritariaReady, pcb);
			}
			if (!pcbEncontrado) {
				pcbEncontrado = buscarYEliminarPcbEnLista(listaEstadoReady, pcb);
			}
			break;
		case NEW:
			buscarYEliminarPcbEnCola(colaEstadoNew, pcb);
			break;
		case BLOCKED:
			pcbEncontrado = false;
			int i = 0;
			int queueSizeRecursos = queue_size(*colasRecursos);
			while (!pcbEncontrado && i < queueSizeRecursos) {
				pcbEncontrado = buscarYEliminarPcbEnCola(colasRecursos[i], pcb);
				i++;
			}
			i = 0;
			int listSizeInterfacesActivas = list_size(interfacesActivas);
			while (!pcbEncontrado && i < listSizeInterfacesActivas) {
				t_propiedadesEntradaSalida *entradaSalida = list_get(interfacesActivas, i);
				pcbEncontrado = buscarYEliminarPcbEnCola(entradaSalida->colaBlocked, pcb);
				i++;
			}
			buscarYEliminarPcbEnCola(colaEstadoBlocked, pcb);
			break;
		case EXIT:
			break;
		}
		iniciarPlanificacion();
		if (pcb->estado != EXEC) {
			terminarProceso(pcb, INTERRUPTED_BY_USER);
		}
	} else {
		log_warning(auxLogger, "No existe el proceso que se quiere terminar");
		iniciarPlanificacion();
	}
}

void cambiarSemMultiprogramacion(int diferenciaGrados, int cantidadPcbsSistema, int nuevoGrado) {
	if (diferenciaGrados > 0) {
		for (int i = 0; i < diferenciaGrados; i++) {
			sem_post(&semMultiprogramacion);
		}
	} else if (diferenciaGrados < 0) {
		int cantWaits;
		if (cantidadPcbsSistema >= nuevoGrado) {
			cantAEsperar = nuevoGrado - cantidadPcbsSistema;
			cantWaits = nuevoGrado;
			for (int i = 0; i < cantWaits; i++) {
				sem_wait(&semMultiprogramacion);
			}
		} else {
			cantWaits = -diferenciaGrados;
			for (int i = 0; i < cantWaits; i++) {
				sem_wait(&semMultiprogramacion);
			}
		}
	}
}

void cambiarGradoMultiprogramacion(char *comando) {
	detenerPlanificacion();
	int nuevoGradoMultiprogramacion = atoi(comando);
	int gradoAnterior = (int) cfgKernel->GRADO_MULTIPROGRAMACION;
	cfgKernel->GRADO_MULTIPROGRAMACION = nuevoGradoMultiprogramacion;
	int diferenciaGrados = nuevoGradoMultiprogramacion - gradoAnterior;
	int cantidadPcbsSistema = gradoAnterior - valorSemaforo(&semMultiprogramacion);
	log_info(auxLogger, "Se cambio el grado de multiprogramacion. Grado anterior %i, grado actual %i", gradoAnterior,
			nuevoGradoMultiprogramacion);
	cambiarSemMultiprogramacion(diferenciaGrados, cantidadPcbsSistema, nuevoGradoMultiprogramacion);
	iniciarPlanificacion();
}

char* imprimirPidsLista(t_list *lista) {
	char *stringAImprimir = malloc(6*sizeof(char)*list_size(lista)+6);
	if (list_size(lista) == 0) {
		strcpy(stringAImprimir, "VACIO");
	}
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_pcb *pcb = (t_pcb*) list_get(lista, i);
		char *pid = malloc(6*sizeof(char));
		if (i == list_size(lista) - 1) { //es el ultimo elemento, no le pongo la coma
			sprintf(pid, "%d", pcb->contextoEjecucion->PID);
		} else {
			sprintf(pid, "%d, ", pcb->contextoEjecucion->PID);
		}
		if (i == 0) { //es el primer elemento, no concateno stringAImprimir, la sobreescribo
			strcpy(stringAImprimir, pid);
		} else {
			strcat(stringAImprimir, pid);
		}
	free(pid);
	}
	return stringAImprimir;
}

// Función para imprimir PIDs de una cola
char* imprimirPidsCola(t_queue *cola) {
	char *stringAImprimir = malloc(6*sizeof(char)*queue_size(cola) + 6); //el +6 es por si esta vacio
	t_queue *colaAuxiliar = queue_create();
	int i = 0;
	if (queue_is_empty(cola)) {
		strcpy(stringAImprimir, "VACIO");
	}
	while (!queue_is_empty(cola)) {
		t_pcb *pcb = (t_pcb*) queue_pop(cola);

		char *pid = malloc(sizeof(char)*6);
		if (queue_size(cola) == 0) { //es el ultimo elemento, no le pongo la coma
			sprintf(pid, "%d", pcb->contextoEjecucion->PID);
		} else {
			sprintf(pid, "%d, ", pcb->contextoEjecucion->PID);
		}
		if (i == 0) { //es el primer elemento, no concateno stringAImprimir, la sobreescribo
			strcpy(stringAImprimir, pid);
		} else {
			strcat(stringAImprimir, pid);
		}
		i++;

		queue_push(colaAuxiliar, pcb);
		free(pid);
	}
	while (!queue_is_empty(colaAuxiliar)) {
		queue_push(cola, queue_pop(colaAuxiliar));
	}
	queue_destroy(colaAuxiliar);
	return stringAImprimir;
}

void mostrarProcesoPorEstado() {
	char* colaNew = imprimirPidsCola(colaEstadoNew);
	char* colaReady = imprimirPidsLista(listaEstadoReady);
	char* colaBlocked = imprimirPidsCola(colaEstadoBlocked);
	char* colaExit = imprimirPidsLista(listaEstadoExit);
	log_info(auxLogger, "Pcbs en NEW: %s", colaNew);
	if (strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "VRR") == 0) {
		char* colaReadyPrioritaria = imprimirPidsCola(colaPrioritariaReady);
		log_info(auxLogger, "Pcbs en READY PRIORITARIA: %s", colaReadyPrioritaria);
		free (colaReadyPrioritaria);
	}
	log_info(auxLogger, "Pcbs en READY: %s", colaReady);
	if (pcbEnExec==NULL) {
		log_info(auxLogger, "Pcb en EXEC: VACIO");
	} else {
		log_info(auxLogger, "Pcb en EXEC: %i", pcbEnExec->contextoEjecucion->PID);
	}
	log_info(auxLogger, "Pcbs en BLOCKED: %s", colaBlocked);
	log_info(auxLogger, "Pcbs en EXIT: %s", colaExit);

	free (colaBlocked);
	free (colaNew);
	free (colaReady);
	free (colaExit);
}

void ejecutarComando(char *path, char *comando) {
	switch (descifrarMensaje(comando)) {
	case INICIAR_PROCESO:
		iniciarProceso(path);
		break;
	case FINALIZAR_PROCESO:
		finalizarProceso(path);
		break;
	case INICIAR_PLANIFICACION:
		iniciarPlanificacion();
		break;
	case DETENER_PLANIFICACION:
		detenerPlanificacion();
		break;
	case PROCESO_ESTADO:
		mostrarProcesoPorEstado();
		break;
	case MULTIPROGRAMACION:
		cambiarGradoMultiprogramacion(path);
		break;
	default:
		break;
	}
}
void ejecutarScript(void *pathAux) {
	char *path = (char*) pathAux;
	char *lineaScript;
	char *aux = "/home/utnso/c-comenta-pruebas";
	char *pathCompleto = malloc(strlen(aux) + strlen(path) + 1);
	strcpy(pathCompleto, aux);
	strcat(pathCompleto, path);
	FILE *fileScript = fopen(pathCompleto, "r");
	free(pathCompleto);
	char *comando;
	char *archivo;
	size_t largo = 0;
	if (fileScript == NULL) {
		log_error(auxLogger, "No se pudo abrir el archivo de script.");
	}
	while (!feof(fileScript)) {
		getline(&lineaScript, &largo, fileScript);
		separarPalabras(lineaScript, &comando, &archivo);
		ejecutarComando(archivo, comando);
	}
	fclose(fileScript);
}

void consola() {
	pcbsEnSistema = list_create();
	while (1) {
		char *ingresoPorConsola;
		char *mensaje;
		char *path;
		leerConsola(&ingresoPorConsola, &mensaje, &path);
		switch (descifrarMensaje(mensaje)) {

		case INICIAR_PROCESO:
			iniciarProceso(path);
			break;
		case FINALIZAR_PROCESO:
			finalizarProceso(path);
			break;
		case INICIAR_PLANIFICACION:
			iniciarPlanificacion();
			break;
		case DETENER_PLANIFICACION:
			detenerPlanificacion();
			break;
		case PROCESO_ESTADO:
			mostrarProcesoPorEstado();
			break;
		case MULTIPROGRAMACION:
			cambiarGradoMultiprogramacion(path);
			break;
		case EJECUTAR_SCRIPT:
			pthread_t hiloEjecutarScript;
			pthread_create(&hiloEjecutarScript, NULL, (void*) ejecutarScript, (void*) path);
			pthread_detach(hiloEjecutarScript);
			break;
		default:
			break;
		}

	}

}

