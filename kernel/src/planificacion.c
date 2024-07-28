#include "../include/planificacion.h"

pthread_mutex_t mutexColaEstadoNew;
pthread_mutex_t mutexListaEstadoReady;
pthread_mutex_t mutexPcbEnExec;
pthread_mutex_t mutexColaEstadoBlocked;
pthread_mutex_t mutexListaEstadoExit;
pthread_mutex_t mutexPcbCambio;
pthread_mutex_t mutexColaReadyPrioritaria;
pthread_mutex_t mutexhaySignal;
pthread_mutex_t mutexListaPcbsEnSistema;
pthread_mutex_t cambioMultiprogramacion;
pthread_mutex_t mutexCambioQuantum;
pthread_mutex_t mutexListaAEnviarSleep;
pthread_mutex_t mutexListaAEnviarRead;
pthread_mutex_t mutexListaAEnviarWrite;
pthread_mutex_t mutexListaAEnviarFS;

sem_t semHayProcesosEnNew;
sem_t semMultiprogramacion;
sem_t semProcesoEnExec;
sem_t semProcesoEnBlocked;
sem_t semHayProcesoEnReady;
sem_t semParaComunicacionFS;
sem_t semParaComunicacionSleep;
sem_t semParaComunicacionWrite;
sem_t semParaComunicacionRead;
sem_t **semaforosRecursos;
sem_t semLivelockSleep;
sem_t semLivelockWrite;
sem_t semLivelockRead;
sem_t semLivelockFS;


sem_t semPausaPlanificacionLargo;
sem_t semPausaPlanificacionCorto;
sem_t semPausaMotivoDesalojo;
sem_t semPausaBlockedAReady;
sem_t semPausarReadyAExec;
sem_t semPausarExit;
sem_t semPausarNewAReady;

t_queue *colaPrioritariaReady;
t_queue *colaEstadoNew;
t_list *listaEstadoReady;
t_pcb *pcbEnExec;
t_queue *colaEstadoBlocked;
t_list *listaEstadoExit;
t_queue **colasRecursos;
t_list *aEnviarSleep; //hay que liberarla junto a los t_envioPcbInterfaz
t_list *aEnviarWrite;
t_list *aEnviarRead;
t_list *aEnviarFS;

pthread_t algoritmoPlanificacion;
pthread_t hiloSleep;
pthread_t hiloRead;
pthread_t hiloWrite;
pthread_t hiloFS;
pthread_t planificadorCortoPlazo;
pthread_t hiloEnvioParametros;

int64_t tiempoTranscurrido;


bool haySignal = false;
bool finalizarProcesoConsola = false;

bool sleepUsada = false;
bool writeUsada = false;
bool readUsada = false;
bool fsUsada = false;

typedef struct {
	t_pcb *pcb;
	t_algoritmoPlanificacion *algoritmo;
} t_gestionInterrupciones;

//struct timespec start, end;
void inicializarSemaforosRecursos() {
	semaforosRecursos = malloc(contarRecursos(cfgKernel->INSTANCIAS_RECURSOS) * sizeof(sem_t));
	if (semaforosRecursos == NULL) {
		perror("Error al asignar memoria para semaforos");
		exit(EXIT_FAILURE);
	}

	int cantidadRecursos = contarRecursos(cfgKernel->INSTANCIAS_RECURSOS);
	for (int i = 0; i < cantidadRecursos; i++) {
		semaforosRecursos[i] = malloc(sizeof(sem_t));
		if (sem_init(semaforosRecursos[i], 0, instanciasRecursos[i]) != 0) {
			perror("Error inicializando el semáforo");
			exit(EXIT_FAILURE);
		}
	}
}
void inicializarColasRecuros() {
	colasRecursos = malloc(contarRecursos(cfgKernel->INSTANCIAS_RECURSOS) * sizeof(t_queue));
	int cantidadRecursos = contarRecursos(cfgKernel->INSTANCIAS_RECURSOS);
	for (int i = 0; i < cantidadRecursos; i++) {
		colasRecursos[i] = queue_create();
	}
}

void inicializarSemaforos() {
	pthread_mutex_init(&mutexColaEstadoNew, (void*) NULL);
	pthread_mutex_init(&mutexListaEstadoReady, (void*) NULL);
	pthread_mutex_init(&mutexPcbEnExec, (void*) NULL);
	pthread_mutex_init(&mutexColaEstadoBlocked, (void*) NULL);
	pthread_mutex_init(&mutexListaEstadoExit, (void*) NULL);
	pthread_mutex_init(&mutexPcbCambio, (void*) NULL);
	pthread_mutex_init(&mutexColaReadyPrioritaria, (void*) NULL);
	pthread_mutex_init(&mutexhaySignal, (void*) NULL);
	pthread_mutex_init(&mutexListaPcbsEnSistema, (void*) NULL);
	pthread_mutex_init(&mutexCambioQuantum, (void*) NULL);
	pthread_mutex_init(&mutexListaAEnviarSleep, (void*) NULL);
	pthread_mutex_init(&mutexListaAEnviarRead, (void*) NULL);
	pthread_mutex_init(&mutexListaAEnviarWrite, (void*) NULL);
	pthread_mutex_init(&mutexListaAEnviarFS, (void*) NULL);

	sem_init(&semHayProcesosEnNew, 0, 0);
	sem_init(&semHayProcesoEnReady, 0, 0);
	sem_init(&semMultiprogramacion, 0, cfgKernel->GRADO_MULTIPROGRAMACION);
	sem_init(&semProcesoEnExec, 0, 1);
	sem_init(&semProcesoEnBlocked, 0, 0);
	sem_init(&semParaComunicacionFS, 0, 1);
	sem_init(&semParaComunicacionSleep, 0, 1);
	sem_init(&semParaComunicacionWrite, 0, 1);
	sem_init(&semParaComunicacionRead, 0, 1);
	inicializarSemaforosRecursos();
	sem_init(&semLivelockSleep, 0, 0);
	sem_init(&semLivelockWrite, 0, 0);
	sem_init(&semLivelockRead, 0, 0);
	sem_init(&semLivelockFS, 0, 0);


	sem_init(&semPausaPlanificacionLargo, 0, 0);
	sem_init(&semPausaPlanificacionCorto, 0, 0);
	sem_init(&semPausaMotivoDesalojo, 0, 0);
	sem_init(&semPausaBlockedAReady, 0, 0);
	sem_init(&semPausarReadyAExec, 0, 0);
	sem_init(&semPausarNewAReady, 0, 0);
}

void inicializarEstados() {
	colaEstadoNew = queue_create();
	listaEstadoReady = list_create();
	colaEstadoBlocked = queue_create();
	listaEstadoExit = list_create();
	colaPrioritariaReady = queue_create();
	aEnviarSleep = list_create();

	aEnviarWrite = list_create();
	aEnviarRead = list_create();
	aEnviarFS = list_create();
	inicializarColasRecuros();
}

void inicializarPlanificacion() {
	inicializarSemaforos();
	inicializarEstados();
}

void cambiarEstadoPcb(t_pcb *pcb, t_estado estadoNuevo) {
	char *estadoActual = obtenerStringEstado(pcb->estado);
	char *nuevoEstado = obtenerStringEstado(estadoNuevo);
	pthread_mutex_lock(&mutexPcbCambio);
	log_info(auxLogger, "PID: %i - Estado Anterior: %s - Estado Actual: %s", pcb->contextoEjecucion->PID, estadoActual,
			nuevoEstado);
	pcb->estado = estadoNuevo;
	pthread_mutex_unlock(&mutexPcbCambio);
}

void* planificarLargoPlazo() {
	pthread_create(&planificadorCortoPlazo, NULL, planificarCortoPlazo, NULL);
	pthread_detach(planificadorCortoPlazo);
	while (1) {
		if (isPlanificacionPausada) {
			sem_wait(&semPausaPlanificacionLargo); //semaforo para poder pausar la planificacion desde consola
		}
		sem_wait(&semHayProcesosEnNew);
		t_pcb *pcb = ingresarNewAReady();
	}
}

void* planificarCortoPlazo() {
	t_pcb *pcb;
	while (1) {
		t_algoritmoPlanificacion algoritmoPlanificacion = obtenerAlgoritmoPlanificacion();
		if (isPlanificacionPausada) {
			sem_wait(&semPausaPlanificacionCorto); //semaforo para poder pausar la planificacion desde consola
		}
		if (!haySignal) {
			pcb = obtenerSiguienteReady(algoritmoPlanificacion);
			if (isPlanificacionPausada) {
				sem_wait(&semPausarReadyAExec); //semaforo para poder pausar la planificacion desde consola
			}
			agregarAExecute(pcb, algoritmoPlanificacion);
		}
		t_contextoEjecucion *nuevoContexto;
		int desplazamiento = 0;
		t_paquete *paqueteContextoActualizado = obtenerYActualizarContexto(pcb, &desplazamiento, nuevoContexto);
		gestionarDesalojo(pcb, paqueteContextoActualizado, desplazamiento);
		eliminarPaquete(paqueteContextoActualizado);
	}
}

t_algoritmoPlanificacion obtenerAlgoritmoPlanificacion() {
	t_algoritmoPlanificacion algoritmoPlanificacion;
	if (!strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "FIFO")) {
		algoritmoPlanificacion = FIFO;
	} else if (!strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "RR")) {
		algoritmoPlanificacion = RR;
	} else if (!strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "VRR")) {
		algoritmoPlanificacion = VRR;
	}
	return algoritmoPlanificacion;
}

// ------------------------------------------
//Planificador de largo plazo
//-------------------------------------------

void ingresarANew(t_pcb *pcb) {
	pthread_mutex_lock(&mutexColaEstadoNew);
	queue_push(colaEstadoNew, (void*) pcb);
	pthread_mutex_unlock(&mutexColaEstadoNew);
	log_info(auxLogger, "Se crea el proceso %d en NEW", pcb->contextoEjecucion->PID);
	sem_post(&semHayProcesosEnNew);
}

t_pcb* ingresarNewAReady() {
	if (isPlanificacionPausada) {
		sem_wait(&semPausarNewAReady); //semaforo para poder pausar la planificacion desde consola
	}
	sem_wait(&semMultiprogramacion);
	pthread_mutex_lock(&cambioMultiprogramacion);
	log_info(auxLogger, "El grado de multiprogramacion permite agregar un proceso a la cola de Ready");
	t_pcb *pcb = obtenerSiguienteNew();
	agregarAColaReady(pcb);
	pthread_mutex_unlock(&cambioMultiprogramacion);
	return pcb;
}

t_pcb* obtenerSiguienteNew() {
	pthread_mutex_lock(&mutexColaEstadoNew);
	t_pcb *pcb = queue_pop(colaEstadoNew);
	pthread_mutex_unlock(&mutexColaEstadoNew);
	return pcb;
}

void agregarAColaReady(t_pcb *pcb) {
	pthread_mutex_lock(&mutexListaEstadoReady);

	char *colaReady = imprimirPidsLista(listaEstadoReady);
	char *colaPrioridad = imprimirPidsCola(colaPrioritariaReady);
	if (pcb->quantum < cfgKernel->QUANTUM && !strcmp(cfgKernel->ALGORITMO_PLANIFICACION, "VRR")) {
		queue_push(colaPrioritariaReady, (void*) pcb);
		log_info(auxLogger, "Se agrego el proceso a %i a READY PRIORITARIO. Cola Ready Prioridad: %s",
				pcb->contextoEjecucion->PID, colaPrioridad);
	} else {
		list_add(listaEstadoReady, (void*) pcb);
		log_info(auxLogger, "Se agrego el proceso a %i a READY. Cola Ready: %s", pcb->contextoEjecucion->PID,
				colaReady);
//		log_info(logger, "Se agrego el proceso a %i a READY. Cola Ready: %s", pcb->contextoEjecucion->PID,
//				imprimirPidsLista(listaEstadoReady));
	}
	cambiarEstadoPcb(pcb, READY);
	free(colaReady);
	free(colaPrioridad);
	pthread_mutex_unlock(&mutexListaEstadoReady);
	sem_post(&semHayProcesoEnReady);
}

//TODO arreglar log, tener en cuenta el motivo de desalojo
void agregarAExit(t_pcb *pcb, t_opCode motivo) {
	if (isPlanificacionPausada) {
		sem_wait(&semPausarExit); //semaforo para poder pausar la planificacion desde consola
	}
	char *motivoFinalizacion = obtenerStringMotivoFinProceso(motivo);
	pthread_mutex_lock(&mutexListaEstadoExit);
	list_add(listaEstadoExit, (void*) pcb);
	log_info(auxLogger, "Finaliza el proceso %i - Motivo: %s", pcb->contextoEjecucion->PID, motivoFinalizacion);
	pthread_mutex_unlock(&mutexListaEstadoExit);
	actualizarSemMultiprogramacion(pcb);
}

void actualizarSemMultiprogramacion(t_pcb *pcb) {
	if (pcb->estado != NEW) {
		if (cantAEsperar >= 0) {
			sem_post(&semMultiprogramacion);
		} else {
			cantAEsperar++;
		}
	}
}

// ------------------------------------------
//Planificador de corto plazo
//-------------------------------------------

t_pcb* obtenerSiguienteReady(t_algoritmoPlanificacion algoritmo) {
	sem_wait(&semHayProcesoEnReady);
	t_pcb *pcb;
	switch (algoritmo) {
	case FIFO:
		pthread_mutex_lock(&mutexListaEstadoReady);
		pcb = list_remove(listaEstadoReady, 0);
		pthread_mutex_unlock(&mutexListaEstadoReady);
		break;
	case RR:
		pthread_mutex_lock(&mutexListaEstadoReady);
		pcb = list_remove(listaEstadoReady, 0);
		pthread_mutex_unlock(&mutexListaEstadoReady);
		break;
	case VRR:
		pthread_mutex_lock(&mutexListaEstadoReady);
		if (queue_size(colaPrioritariaReady) > 0) {
			pcb = queue_pop(colaPrioritariaReady);
		} else {
			pcb = list_remove(listaEstadoReady, 0);
		}
		pthread_mutex_unlock(&mutexListaEstadoReady);
		break;
	}
	return pcb;
}

void agregarAExecute(t_pcb *pcb, t_algoritmoPlanificacion algoritmo) {
	switch (algoritmo) {
	case FIFO:
		pthread_create(&algoritmoPlanificacion, NULL, (void*) planificarPorFifo, (void*) pcb);
		pthread_detach(algoritmoPlanificacion);
		break;
	case RR:
		pthread_create(&algoritmoPlanificacion, NULL, (void*) planificarPorRR, (void*) pcb);
		pthread_detach(algoritmoPlanificacion);
		break;
	case VRR:
		pthread_create(&algoritmoPlanificacion, NULL, (void*) planificarPorVRR, (void*) pcb);
		pthread_detach(algoritmoPlanificacion);
		break;
	}
}

void sacarDeExecute(t_pcb *pcb) {
	pthread_mutex_lock(&mutexPcbEnExec);
	pcbEnExec = NULL;
	log_info(auxLogger, "Se saca el proceso %i de Execute", pcb->contextoEjecucion->PID);
	pthread_mutex_unlock(&mutexPcbEnExec);
	sem_post(&semProcesoEnExec);
	if (isPlanificacionPausada) {
		sem_wait(&semPausaMotivoDesalojo); //semaforo para poder pausar la planificacion desde consola
	}
}

t_propiedadesEntradaSalida* buscarESPorNombre(t_list *lista, char *nombre) {
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_propiedadesEntradaSalida *propiedadesES = (t_propiedadesEntradaSalida*) list_get(lista, i);
		if (strcmp(propiedadesES->nombre, nombre) == 0) {
			return propiedadesES;
		}
	}
	return NULL;  // No se encontró el elemento
}

t_pcb* agregarABlocked(t_pcb *pcb, char *nombreInterfaz, t_parametrosInterfaz *parametros,
		t_propiedadesEntradaSalida **propiedadesES) {

	*propiedadesES = buscarESPorNombre(interfacesActivas, nombreInterfaz);

	if (*propiedadesES != NULL) {
		pthread_mutex_lock(&mutexColaEstadoBlocked);
		queue_push((*propiedadesES)->colaBlocked, (void*) pcb);
		queue_push(colaEstadoBlocked, (void*) pcb);
		log_info(auxLogger, "PID: %i - Bloqueado por: %s", pcb->contextoEjecucion->PID, nombreInterfaz);
		pthread_mutex_unlock(&mutexColaEstadoBlocked);
		sem_post(&semProcesoEnBlocked);
	} else {
		log_error(auxLogger, "No se encontro la E/S que bucaba el proceso %i", pcb->contextoEjecucion->PID);
		sacarDeExecute(pcb);
		terminarProceso(pcb, INVALID_INTERFACE);
	}
	return pcb;
}

t_pcb* agregarABlockedRecursos(t_pcb *pcb, int recurso, char *nombreRecurso) {
	pthread_mutex_lock(&mutexColaEstadoBlocked);
	queue_push(colasRecursos[recurso], (void*) pcb);
	queue_push(colaEstadoBlocked, (void*) pcb);
	log_info(auxLogger, "PID: %i - Bloqueado por: %s", pcb->contextoEjecucion->PID, nombreRecurso);
	sem_post(&semProcesoEnBlocked);
	pthread_mutex_unlock(&mutexColaEstadoBlocked);
	return pcb;
}

t_envioPcbInterfaz* obtenerYRemoverPrimeroLibre(t_list *lista) {
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_envioPcbInterfaz *elemento = (t_envioPcbInterfaz*) list_get(lista, i);
		if (elemento->propiedades->enUso == false) {
			list_remove(lista, i);
			return elemento;
		}
	}
	return NULL;
}
t_envioPcbFS* obtenerYRemoverPrimeroLibreFS(t_list *lista) {
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_envioPcbFS *elemento = (t_envioPcbFS*) list_get(lista, i);
		if (elemento->propiedades->enUso == false) {
			list_remove(lista, i);
			return elemento;
		}
	}
	return NULL;
}

void actualizarEnUso(t_list *lista, bool nuevoValor, char *nombreBuscado) {
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_envioPcbInterfaz *elemento = (t_envioPcbInterfaz*) list_get(lista, i);
		if (strcmp(elemento->propiedades->nombre, nombreBuscado) == 0) {
			elemento->propiedades->enUso = nuevoValor;
		}
	}
}

void actualizarEnUsoFS(t_list *lista, bool nuevoValor, char *nombreBuscado) {
	int listSize = list_size(lista);
	for (int i = 0; i < listSize; i++) {
		t_envioPcbFS *elemento = (t_envioPcbFS*) list_get(lista, i);
		if (strcmp(elemento->propiedades->nombre, nombreBuscado) == 0) {
			elemento->propiedades->enUso = nuevoValor;
		}
	}
}

void actualizarListaInterfaces(bool nuevoValor, char *nombreBuscado) {
	int listSize = list_size(interfacesActivas);
	for (int i = 0; i < listSize; i++) {
		t_propiedadesEntradaSalida *propiedadesES = (t_propiedadesEntradaSalida*) list_get(interfacesActivas, i);
		if (strcmp(propiedadesES->nombre, nombreBuscado) == 0) {
			propiedadesES->enUso = nuevoValor;
		}
	}
}

void envioParametrosAInterfaz(t_list *listaPcbAEnviar) {
	while (1) {
		if (algunaInterfazLibre(listaPcbAEnviar)) {

			pthread_mutex_lock(&mutexListaAEnviarSleep);
			t_envioPcbInterfaz *parametrosAEnviar = obtenerYRemoverPrimeroLibre(listaPcbAEnviar);
			actualizarEnUso(listaPcbAEnviar, true, parametrosAEnviar->propiedades->nombre);
			actualizarListaInterfaces(true, parametrosAEnviar->propiedades->nombre);
			pthread_mutex_unlock(&mutexListaAEnviarSleep);

			t_paquete *paquete = crearPaquete(parametrosAEnviar->codigo);
			agregarAPaquete(paquete, &(parametrosAEnviar->PID), sizeof(int));
			serializarParametros(paquete, parametrosAEnviar->paramInterfaz);
			enviarPaquete(paquete, parametrosAEnviar->propiedades->socketConexion);
			eliminarPaquete(paquete);
			t_manejoBlocked *pidYInterfaz = malloc(sizeof(t_manejoBlocked));
			pidYInterfaz->nombreInterfaz = malloc(parametrosAEnviar->paramInterfaz->largoInterfaz +1 );
			strcpy(pidYInterfaz->nombreInterfaz, parametrosAEnviar->paramInterfaz->interfaz);
			pidYInterfaz->pid = 0;
			pidYInterfaz->codigo = SYSCALL_IO_GEN_SLEEP;
			pthread_create(&hiloRead, NULL, (void*) esperarPIDADesbloquear, pidYInterfaz);
			pthread_detach(hiloRead);

			liberarParametrosInterfaz(parametrosAEnviar->paramInterfaz);
			free(parametrosAEnviar);
		}
		sem_wait(&semLivelockSleep);
		//Realizo un wait aca, su post se hace cuando se desbloquea una interfaz en esperarPIDADesbloquear, o cuando llega un
		//nuevo proceso (con la interfaz libre) a gestionDesalojo (IO Sleep)
	}

}

void envioParametrosAInterfazRead(t_list *listaPcbAEnviar) {
	while (1) {
		if (algunaInterfazLibreRead(listaPcbAEnviar)) {

			pthread_mutex_lock(&mutexListaAEnviarRead);
			t_envioPcbInterfaz *parametrosAEnviar = obtenerYRemoverPrimeroLibre(listaPcbAEnviar);
			actualizarListaInterfaces(true, parametrosAEnviar->propiedades->nombre);
			actualizarEnUso(listaPcbAEnviar, true, parametrosAEnviar->propiedades->nombre);
			pthread_mutex_unlock(&mutexListaAEnviarRead);

			t_paquete *paquete = crearPaquete(parametrosAEnviar->codigo);
			agregarAPaquete(paquete, &(parametrosAEnviar->PID), sizeof(int));
			serializarParametros(paquete, parametrosAEnviar->paramInterfaz);
			enviarPaquete(paquete, parametrosAEnviar->propiedades->socketConexion);
			eliminarPaquete(paquete);

			t_manejoBlocked *pidYInterfaz = malloc(sizeof(t_manejoBlocked));
			pidYInterfaz->nombreInterfaz = malloc(parametrosAEnviar->paramInterfaz->largoInterfaz +1 );
			strcpy(pidYInterfaz->nombreInterfaz, parametrosAEnviar->paramInterfaz->interfaz);
			pidYInterfaz->pid = 0;
			pidYInterfaz->codigo = SYSCALL_IO_STDIN_READ;
			pthread_create(&hiloRead, NULL, (void*) esperarPidPcbADesbloquearRead, pidYInterfaz);
			pthread_detach(hiloRead);

			liberarParametrosInterfaz(parametrosAEnviar->paramInterfaz);
			free(parametrosAEnviar);
		}
		sem_wait(&semLivelockRead);
	}

}

void envioParametrosAInterfazWrite(t_list *listaPcbAEnviar) {
	while (1) {
		if (algunaInterfazLibreWrite(listaPcbAEnviar)) {

			pthread_mutex_lock(&mutexListaAEnviarWrite);
			t_envioPcbInterfaz *parametrosAEnviar = obtenerYRemoverPrimeroLibre(listaPcbAEnviar);
			actualizarEnUso(listaPcbAEnviar, true, parametrosAEnviar->propiedades->nombre);
			actualizarListaInterfaces(true, parametrosAEnviar->propiedades->nombre);
			pthread_mutex_unlock(&mutexListaAEnviarWrite);

			t_paquete *paquete = crearPaquete(parametrosAEnviar->codigo);
			agregarAPaquete(paquete, &(parametrosAEnviar->PID), sizeof(int));
			serializarParametros(paquete, parametrosAEnviar->paramInterfaz);
			enviarPaquete(paquete, parametrosAEnviar->propiedades->socketConexion);
			eliminarPaquete(paquete);

			t_manejoBlocked *pidYInterfaz = malloc(sizeof(t_manejoBlocked));
			pidYInterfaz->nombreInterfaz = malloc(parametrosAEnviar->paramInterfaz->largoInterfaz + 1);
			strcpy(pidYInterfaz->nombreInterfaz, parametrosAEnviar->paramInterfaz->interfaz);
			pidYInterfaz->pid = 0;
			pidYInterfaz->codigo = SYSCALL_IO_STDOUT_WRITE;
			pthread_create(&hiloRead, NULL, (void*) esperarPidPcbADesbloquearWrite, pidYInterfaz);
			pthread_detach(hiloRead);

			liberarParametrosInterfaz(parametrosAEnviar->paramInterfaz);
			free(parametrosAEnviar);
		}
		sem_wait(&semLivelockWrite);
	}

}

void envioParametrosAFS(t_list *listaAEnviar) {
	while (1) {
		if (algunaInterfazLibreFS(listaAEnviar)) {
			pthread_mutex_lock(&mutexListaAEnviarFS);
			t_envioPcbFS *parametrosAEnviar = obtenerYRemoverPrimeroLibreFS(listaAEnviar);
			actualizarEnUsoFS(listaAEnviar, true, parametrosAEnviar->propiedades->nombre);
			actualizarListaInterfaces(true, parametrosAEnviar->propiedades->nombre);
			pthread_mutex_unlock(&mutexListaAEnviarFS);

			t_paquete *paquete = crearPaquete(parametrosAEnviar->codigo);
			agregarAPaquete(paquete, &(parametrosAEnviar->PID), sizeof(int));
			serializarParametrosFS(paquete, parametrosAEnviar->parametrosFS);
			enviarPaquete(paquete, parametrosAEnviar->propiedades->socketConexion);
			eliminarPaquete(paquete);

			t_manejoBlocked *manejoBlocked = malloc(sizeof(t_manejoBlocked));
			manejoBlocked->nombreInterfaz = malloc(parametrosAEnviar->parametrosFS->parametrosInterfaz->largoInterfaz + 1);
			strcpy(manejoBlocked->nombreInterfaz, parametrosAEnviar->parametrosFS->parametrosInterfaz->interfaz);
			manejoBlocked->pid = 0;
			manejoBlocked->codigo = SOLICITUD_IO_FS_CREATE;
			pthread_create(&hiloFS, NULL, (void*) esperarPIDADesbloquearFS, manejoBlocked);
			pthread_detach(hiloFS);

			liberarParametrosFS(parametrosAEnviar->parametrosFS);
			free(parametrosAEnviar);

		}
		sem_wait(&semLivelockFS);
	}
}

t_pcb* obtenerSiguienteBlocked(t_queue *cola) {
	t_pcb *pcb;
	if (isPlanificacionPausada && !finalizarProcesoConsola) {
		sem_wait(&semPausaBlockedAReady); //semaforo para poder pausar la planificacion desde consola
	}
	sem_wait(&semProcesoEnBlocked);
	pthread_mutex_lock(&mutexColaEstadoBlocked);
	pcb = queue_pop(cola);
	buscarYEliminarPcbEnCola(colaEstadoBlocked, pcb);
	log_info(auxLogger, "Se saca el proceso %i de la cola de Blocked", pcb->contextoEjecucion->PID);
	pthread_mutex_unlock(&mutexColaEstadoBlocked);
	return pcb;
}

//ALGORITMOS DE PLANIFICACION

void planificarPorFifo(void *pcb) {
	t_pcb *aux = (t_pcb*) pcb;
	sem_wait(&semProcesoEnExec);
	pthread_mutex_lock(&mutexPcbEnExec);
	pcbEnExec = aux;
	enviarContextoEjecucion(aux->contextoEjecucion, socketDispatch, CONTEXTO_EJECUCION);
	cambiarEstadoPcb(pcb, EXEC);
	pthread_mutex_unlock(&mutexPcbEnExec);
}

void planificarPorRR(void *pcb) {
	t_pcb *aux = (t_pcb*) pcb;
	sem_wait(&semProcesoEnExec);
	pthread_mutex_lock(&mutexPcbEnExec);
	pcbEnExec = aux;
	pthread_mutex_unlock(&mutexPcbEnExec);
	enviarContextoEjecucion(aux->contextoEjecucion, socketDispatch, CONTEXTO_EJECUCION);
	cambiarEstadoPcb(pcb, EXEC);
	manejarInterrupcionesYFinQuantum(aux);
}

void planificarPorVRR(void *pcb) {
	t_pcb *aux = (t_pcb*) pcb;
	sem_wait(&semProcesoEnExec);
	pthread_mutex_lock(&mutexPcbEnExec);
	pcbEnExec = aux;
	pthread_mutex_unlock(&mutexPcbEnExec);
	enviarContextoEjecucion(aux->contextoEjecucion, socketDispatch, CONTEXTO_EJECUCION);
	cambiarEstadoPcb(pcb, EXEC);
	manejarInterrupcionesYFinQuantum(aux);

}

void manejarInterrupcionesYFinQuantum(t_pcb *pcb) {
	t_temporal* tiempo = temporal_create();
	while (pcb->estado != EXIT) {
		tiempoTranscurrido = temporal_gettime(tiempo);
		if (tiempoTranscurrido>= pcb->quantum && pcb->estado == EXEC && pcb->estado != BLOCKED) {
			enviarInterrupcion(FIN_QUANTUM);
			pcb->quantum = cfgKernel->QUANTUM;
			actualizarQuantumRestante(pcb, algoritmoPlanificacion);
			pthread_exit((void*) 0);
		} else if (pcb->estado == BLOCKED) {
			actualizarQuantumRestante(pcb, algoritmoPlanificacion);
			pthread_exit((void*) 0);
		}
		usleep(10);
	}
}

void enviarInterrupcion(t_opCode motivoInterrupcion) {
	send(socketInterrupt, &motivoInterrupcion, sizeof(t_opCode), 0);

}

void liberarPcb(t_pcb *pcb) {
	liberarContexto(pcb->contextoEjecucion);
	int cantidadRecursos = contarRecursos(cfgKernel->INSTANCIAS_RECURSOS);
	for (int i = 0; i < cantidadRecursos; i++) {
		free(&(pcb->recursosAsignados[i]));
	}
	free(pcb->recursosAsignados);
	free(pcb);
}

void liberarColasBlockedRecursos() {
	int cantidadRecursos = contarRecursos(cfgKernel->INSTANCIAS_RECURSOS);
	for (int i = 0; i < cantidadRecursos; i++) {
		queue_destroy_and_destroy_elements(colasRecursos[i], (void*) liberarPcb);
	}
}

void liberarParametrosAEnviar(t_envioPcbInterfaz *envioAInterfaz) {
	free(envioAInterfaz->paramInterfaz->direccionesFisicas->listaDirecciones);
	free(envioAInterfaz->paramInterfaz->direccionesFisicas);
	free(envioAInterfaz->paramInterfaz->buffer);
	free(envioAInterfaz->paramInterfaz->interfaz);
	free(envioAInterfaz->propiedades->nombre);
	queue_destroy_and_destroy_elements(envioAInterfaz->propiedades->colaBlocked, (void*) liberarPcb);
	free(envioAInterfaz->paramInterfaz);
	free(envioAInterfaz->propiedades);
	free(envioAInterfaz);
}

void liberarParametrosAEnviarFS(t_parametrosFS *envioAFS) {
	free(envioAFS->parametrosInterfaz->direccionesFisicas->listaDirecciones);
	free(envioAFS->parametrosInterfaz->direccionesFisicas);
	free(envioAFS->parametrosInterfaz->buffer);
	free(envioAFS->parametrosInterfaz->interfaz);
	free(envioAFS->nombreArchivo);
	free(envioAFS);
}

//TODO DESTRUIR ESTRUCTURAS DE PLANIFICACION
void liberarEstructurasPlanificacion() {
	list_destroy_and_destroy_elements(listaEstadoReady, (void*) liberarPcb);
	list_destroy_and_destroy_elements(listaEstadoExit, (void*) liberarPcb);
	queue_destroy_and_destroy_elements(colaEstadoNew, (void*) liberarPcb);
	queue_destroy_and_destroy_elements(colaEstadoBlocked, (void*) liberarPcb);
	queue_destroy_and_destroy_elements(colaPrioritariaReady, (void*) liberarPcb);
	list_destroy_and_destroy_elements(listaEstadoExit, (void*) liberarPcb);
	list_destroy_and_destroy_elements(aEnviarSleep, (void*) liberarParametrosAEnviar); //hay que liberarla junto a los t_envioPcbInterfaz
	list_destroy_and_destroy_elements(aEnviarWrite, (void*) liberarParametrosAEnviar);
	list_destroy_and_destroy_elements(aEnviarRead, (void*) liberarParametrosAEnviar);
	list_destroy_and_destroy_elements(aEnviarFS, (void*) liberarParametrosAEnviarFS);
	liberarColasBlockedRecursos();

	pthread_mutex_destroy(&mutexColaEstadoNew);
	pthread_mutex_destroy(&mutexListaEstadoReady);
	pthread_mutex_destroy(&mutexPcbEnExec);
	pthread_mutex_destroy(&mutexColaEstadoBlocked);
	pthread_mutex_destroy(&mutexListaEstadoExit);
	pthread_mutex_destroy(&mutexPcbCambio);
	pthread_mutex_destroy(&mutexColaReadyPrioritaria);
	pthread_mutex_destroy(&mutexhaySignal);
	pthread_mutex_destroy(&mutexListaPcbsEnSistema);
	pthread_mutex_destroy(&cambioMultiprogramacion);
	pthread_mutex_destroy(&mutexCambioQuantum);
	pthread_mutex_destroy(&mutexListaAEnviarSleep);
	pthread_mutex_destroy(&mutexListaAEnviarRead);
	pthread_mutex_destroy(&mutexListaAEnviarWrite);
	pthread_mutex_destroy(&mutexListaAEnviarFS);

	sem_destroy(&semHayProcesosEnNew);
	sem_destroy(&semMultiprogramacion);
	sem_destroy(&semProcesoEnExec);
	sem_destroy(&semProcesoEnBlocked);
	sem_destroy(&semHayProcesoEnReady);
	sem_destroy(*semaforosRecursos);
	sem_destroy(&semPausaPlanificacionLargo);
	sem_destroy(&semPausaPlanificacionCorto);
	sem_destroy(&semPausaMotivoDesalojo);
	sem_destroy(&semPausaBlockedAReady);
	sem_destroy(&semPausarReadyAExec);
	sem_destroy(&semPausarExit);
	sem_destroy(&semPausarNewAReady);
	sem_destroy(&semLivelockSleep);
	sem_destroy(&semLivelockWrite);
	sem_destroy(&semLivelockRead);
	sem_destroy(&semLivelockFS);
}

t_paquete* obtenerYActualizarContexto(t_pcb *pcb, int *desplazamiento, t_contextoEjecucion *contexto) {
	//RECIBO EL MOTIVO DE DESALOJO
	t_opCode motivo = recibirOperacion(socketDispatch);
	if (esSyscall(motivo)) {
		cambiarEstadoPcb(pcb, BLOCKED);
	}
	//CREO EL BUFFER PARA RECIBIR EL CONTEXTO
	t_paquete *paquete = crearPaquete(motivo);
	//ESPERO A QUE ME LLEGUE EL CONTEXTO
	paquete->buffer->stream = recibirBuffer(&(paquete->buffer->size), socketDispatch);
	//DESERIALIZO EL NUEVO CONTEXTO
	contexto = deserializarContexto(paquete->buffer, desplazamiento);
	//ACTUALIZO EL PCB CON EL NUEVO CONTEXTO
	if (pcb->contextoEjecucion->PID == contexto->PID) {
		actualizarContexto(pcb, contexto);
	} else {
		log_error(auxLogger, "El pid esperado no es el correcto");
	}
	return paquete;
}

bool esSyscall(t_opCode codigo) {
	return codigo == SYSCALL_IO_GEN_SLEEP || codigo == SYSCALL_IO_STDIN_READ || codigo == SYSCALL_IO_STDOUT_WRITE
			|| codigo == SOLICITUD_IO_FS_CREATE || codigo == SOLICITUD_IO_FS_DELETE
			|| codigo == SOLICITUD_IO_FS_TRUNCATE || codigo == SOLICITUD_IO_FS_WRITE || codigo == SOLICITUD_IO_FS_READ;
}

void actualizarContexto(t_pcb *pcb, t_contextoEjecucion *contexto) {
	memcpy(&(pcb->contextoEjecucion->PC), &(contexto->PC), sizeof(uint32_t));
	memcpy(pcb->contextoEjecucion->registrosGenerales, contexto->registrosGenerales, sizeof(t_registrosGenerales));
	memcpy(pcb->contextoEjecucion->registrosMemoria, contexto->registrosMemoria, sizeof(t_registrosMemoria));
	liberarContexto(contexto);
}

//FIXME esta funcion deberia estar en comunicacion.c pero falla si no esta en el modulo donde se usa
t_parametrosInterfaz* deserializarParametrosInterfacesKernel(t_paquete *paquete, int *desplazamientoAux) {
	int *desplazamiento;
	if (desplazamientoAux == NULL) {
		*desplazamiento = 0;
	}
	desplazamiento = desplazamientoAux;

	t_parametrosInterfaz *parametros = malloc(sizeof(t_parametrosInterfaz));
	t_buffer *buffer = paquete->buffer;
	//Largointerfaz
	memcpy(&(parametros->largoInterfaz), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametros->interfaz = malloc(parametros->largoInterfaz * sizeof(char) +1);
	memcpy(parametros->interfaz, buffer->stream + *desplazamiento, parametros->largoInterfaz);
	parametros->interfaz[parametros->largoInterfaz] = '\0';
	*desplazamiento += parametros->largoInterfaz * sizeof(char);

	//txt (de ser necesario)
	memcpy(&(parametros->largoTexto), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametros->buffer = malloc(parametros->largoTexto * sizeof(char) +1);
	memcpy(parametros->buffer, buffer->stream + *desplazamiento, parametros->largoTexto );
	parametros->buffer[parametros->largoTexto] = '\0';
	*desplazamiento += parametros->largoTexto * sizeof(char);

	//tiempoSleep
	memcpy(&(parametros->tiempoSleep), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametros->direccionesFisicas = malloc(sizeof(t_direccionesFisicas));
	memcpy(&(parametros->direccionesFisicas->cantidadDirecciones), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	int cantidadDirecciones = parametros->direccionesFisicas->cantidadDirecciones;
	parametros->direccionesFisicas->listaDirecciones = malloc(cantidadDirecciones * sizeof(int));
	memcpy(parametros->direccionesFisicas->listaDirecciones, buffer->stream + *desplazamiento,
			cantidadDirecciones * sizeof(int));
	*desplazamiento += cantidadDirecciones * sizeof(int);

	memcpy(&(parametros->limiteEntrada), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	return parametros;
}

t_nombreRecurso* recibirNombreRecurso(t_paquete *paquete, int *desplazamientoAux) {

	int *desplazamiento;
	if (desplazamientoAux == NULL) {
		*desplazamiento = 0;
	}
	desplazamiento = desplazamientoAux;
	t_nombreRecurso *parametrosNombre = malloc(sizeof(t_nombreRecurso));
	t_buffer *buffer = paquete->buffer;

	memcpy(&(parametrosNombre->largoNombre), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametrosNombre->nombre = malloc(parametrosNombre->largoNombre * sizeof(char));
	memcpy(parametrosNombre->nombre, buffer->stream + *desplazamiento, parametrosNombre->largoNombre);
	parametrosNombre->nombre[parametrosNombre->largoNombre] = '\0';
	*desplazamiento += parametrosNombre->largoNombre * sizeof(char);

	return parametrosNombre;
}

bool algunaInterfazLibre(t_list *lista) {
	pthread_mutex_lock(&mutexListaAEnviarSleep);
	int listSize = list_size(lista);
	if (listSize > 0) {
		for (int i = 0; i < listSize; i++) {
			t_envioPcbInterfaz *elemento = (t_envioPcbInterfaz*) list_get(lista, i);
			if (elemento->propiedades->enUso == false) {
				pthread_mutex_unlock(&mutexListaAEnviarSleep);
				return true;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaAEnviarSleep);
	return false;
}

bool algunaInterfazLibreRead (t_list *lista) {
	pthread_mutex_lock(&mutexListaAEnviarRead);
	int listSize = list_size(lista);
	if (listSize > 0) {
		for (int i = 0; i < listSize; i++) {
			t_envioPcbInterfaz *elemento = (t_envioPcbInterfaz*) list_get(lista, i);
			if (elemento->propiedades->enUso == false) {
				pthread_mutex_unlock(&mutexListaAEnviarRead);
				return true;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaAEnviarRead);
	return false;
}

bool algunaInterfazLibreWrite (t_list *lista) {
	pthread_mutex_lock(&mutexListaAEnviarWrite);
	int listSize = list_size(lista);
	if (listSize > 0) {
		for (int i = 0; i < listSize; i++) {
			t_envioPcbInterfaz *elemento = (t_envioPcbInterfaz*) list_get(lista, i);
			if (elemento->propiedades->enUso == false) {
				pthread_mutex_unlock(&mutexListaAEnviarWrite);
				return true;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaAEnviarWrite);
	return false;
}

bool algunaInterfazLibreFS(t_list *lista) {
	pthread_mutex_lock(&mutexListaAEnviarFS);
	int listSize = list_size(lista);
	if (listSize > 0) {
		for (int i = 0; i < listSize; i++) {
			t_envioPcbFS *elemento = (t_envioPcbFS*) list_get(lista, i);
			if (elemento->propiedades->enUso == false) {
				pthread_mutex_unlock(&mutexListaAEnviarFS);
				return true;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaAEnviarFS);
	return false;
}

void gestionarDesalojo(t_pcb *pcb, t_paquete *paquete, int desplazamiento) {
	pthread_mutex_lock(&mutexhaySignal);
	haySignal = false;
	pthread_mutex_unlock(&mutexhaySignal);
	switch (paquete->codigo_operacion) {
	case FIN_QUANTUM:
		log_info(auxLogger, "PID: %d - Desalojado por fin de Quantum", pcb->contextoEjecucion->PID);
		sacarDeExecute(pcb);
		pcb->quantum = cfgKernel->QUANTUM;
		agregarAColaReady(pcb);
		break;
	case SYSCALL_EXIT:
		sacarDeExecute(pcb);
		terminarProceso(pcb, SYSCALL_EXIT);
		break;
	case ERROR_OUT_OF_MEMORY:
		sacarDeExecute(pcb);
		terminarProceso(pcb, ERROR_OUT_OF_MEMORY);
		break;
	case SYSCALL_IO_GEN_SLEEP:
		sacarDeExecute(pcb);
		t_parametrosInterfaz *parametros = deserializarParametrosInterfacesKernel(paquete, &desplazamiento);
		t_propiedadesEntradaSalida *propiedadesES = NULL;
		pcb = agregarABlocked(pcb, parametros->interfaz, parametros, &propiedadesES);
		if (pcb->estado == BLOCKED) {
			pthread_mutex_lock(&mutexListaAEnviarSleep);
			t_envioPcbInterfaz *parametrosParaEnvioPcbInterfaz = malloc(sizeof(t_envioPcbInterfaz));
			parametrosParaEnvioPcbInterfaz->propiedades = propiedadesES;
			parametrosParaEnvioPcbInterfaz->paramInterfaz = parametros;
			parametrosParaEnvioPcbInterfaz->codigo = SYSCALL_IO_GEN_SLEEP;
			parametrosParaEnvioPcbInterfaz->PID = pcb->contextoEjecucion->PID;
			list_add(aEnviarSleep, (void*) parametrosParaEnvioPcbInterfaz);
			if(!parametrosParaEnvioPcbInterfaz->propiedades->enUso && sleepUsada){
				sem_post(&semLivelockSleep);
			}
			pthread_mutex_unlock(&mutexListaAEnviarSleep);
			if (!sleepUsada) {
				pthread_create(&hiloEnvioParametros, NULL, (void*) envioParametrosAInterfaz, aEnviarSleep);
				pthread_detach(hiloEnvioParametros);
			}
			sleepUsada = true;
		}

		break;

	case SYSCALL_IO_STDIN_READ:
		sacarDeExecute(pcb);
		t_parametrosInterfaz *param = deserializarParametrosInterfacesKernel(paquete, &desplazamiento);
		t_propiedadesEntradaSalida *propiedadesES1 = NULL;
		pcb = agregarABlocked(pcb, param->interfaz, param, &propiedadesES1);
		if (pcb->estado == BLOCKED) {
			pthread_mutex_lock(&mutexListaAEnviarRead);
			t_envioPcbInterfaz *parametrosParaEnvioPcbInterfaz = malloc(sizeof(t_envioPcbInterfaz));
			parametrosParaEnvioPcbInterfaz->propiedades = propiedadesES1;
			parametrosParaEnvioPcbInterfaz->paramInterfaz = param;
			parametrosParaEnvioPcbInterfaz->codigo = SYSCALL_IO_STDIN_READ;
			parametrosParaEnvioPcbInterfaz->PID = pcb->contextoEjecucion->PID;
			list_add(aEnviarRead, (void*) parametrosParaEnvioPcbInterfaz);
			if(!parametrosParaEnvioPcbInterfaz->propiedades->enUso && readUsada){
							sem_post(&semLivelockRead);
						}
			pthread_mutex_unlock(&mutexListaAEnviarRead);
			if (!readUsada) {
				pthread_create(&hiloEnvioParametros, NULL, (void*) envioParametrosAInterfazRead, aEnviarRead);
				pthread_detach(hiloEnvioParametros);
			}
			readUsada = true;
		}
		break;
	case SYSCALL_IO_STDOUT_WRITE:
		sacarDeExecute(pcb);
		t_parametrosInterfaz *parame = deserializarParametrosInterfacesKernel(paquete, &desplazamiento);
		t_propiedadesEntradaSalida *propiedadesES2 = NULL;
		pcb = agregarABlocked(pcb, parame->interfaz, parame, &propiedadesES2);
		if (pcb->estado == BLOCKED) {
			pthread_mutex_lock(&mutexListaAEnviarWrite);
			t_envioPcbInterfaz *parametrosParaEnvioPcbInterfaz = malloc(sizeof(t_envioPcbInterfaz));
			parametrosParaEnvioPcbInterfaz->propiedades = propiedadesES2;
			parametrosParaEnvioPcbInterfaz->paramInterfaz = parame;
			parametrosParaEnvioPcbInterfaz->codigo = SYSCALL_IO_STDOUT_WRITE;
			parametrosParaEnvioPcbInterfaz->PID = pcb->contextoEjecucion->PID;
			list_add(aEnviarWrite, (void*) parametrosParaEnvioPcbInterfaz);
			if(!parametrosParaEnvioPcbInterfaz->propiedades->enUso && writeUsada){
										sem_post(&semLivelockWrite);
									}
			pthread_mutex_unlock(&mutexListaAEnviarWrite);
			if (!writeUsada) {
				pthread_create(&hiloEnvioParametros, NULL, (void*) envioParametrosAInterfazWrite, aEnviarWrite);
				pthread_detach(hiloEnvioParametros);
			}
			writeUsada = true;
		}
		break;
	case SYSCALL_WAIT:
		t_nombreRecurso *parametrosNombre = recibirNombreRecurso(paquete, &desplazamiento);
		char *recurso = parametrosNombre->nombre;
		int posicionRecurso = encontrarRecurso(recurso, cfgKernel->RECURSOS);
		if (posicionRecurso != -1) {
			if (valorSemaforo(semaforosRecursos[posicionRecurso]) > 0) {
				pthread_mutex_lock(&mutexhaySignal);
				haySignal = true;
				pthread_mutex_unlock(&mutexhaySignal);
				sem_wait(semaforosRecursos[posicionRecurso]);
				pcb->recursosAsignados[posicionRecurso]++;
				actualizarQuantumRestante(pcb, algoritmoPlanificacion);
				enviarOperacion(CONTINUAR_EJECUCION, socketDispatch);
			} else {
				enviarOperacion(DETENER_EJECUCION, socketDispatch);
				sacarDeExecute(pcb);
				actualizarQuantumRestante(pcb, algoritmoPlanificacion);
				cambiarEstadoPcb(pcb, BLOCKED);
				pcb = agregarABlockedRecursos(pcb, posicionRecurso, recurso);
			}
		} else {
			enviarOperacion(DETENER_EJECUCION, socketDispatch);
			sacarDeExecute(pcb);
			terminarProceso(pcb, INVALID_RESOURCE);
		}
		free(parametrosNombre->nombre);
		free(parametrosNombre);
		break;
	case SYSCALL_SIGNAL:
		pthread_mutex_lock(&mutexhaySignal);
		haySignal = true;
		pthread_mutex_unlock(&mutexhaySignal);
		t_nombreRecurso *parametrosNombre1 = recibirNombreRecurso(paquete, &desplazamiento);
		char *recurso1 = parametrosNombre1->nombre;
		int posicionRecurso1 = encontrarRecurso(recurso1, cfgKernel->RECURSOS);
		if (posicionRecurso1 != -1) {
			enviarOperacion(CONTINUAR_EJECUCION, socketDispatch);
			actualizarQuantumRestante(pcb, algoritmoPlanificacion);
			sem_post(semaforosRecursos[posicionRecurso1]);
			pcb->recursosAsignados[posicionRecurso1]--;
			if (queue_size(colasRecursos[posicionRecurso1]) > 0) {
				t_pcb *pcbAReady1 = obtenerSiguienteBlocked(colasRecursos[posicionRecurso1]);
				pcbAReady1->recursosAsignados[posicionRecurso1]++;
				sem_wait(semaforosRecursos[posicionRecurso1]);
				agregarAColaReady(pcbAReady1);
				cambiarEstadoPcb(pcbAReady1, READY);
			}
		} else {
			enviarOperacion(DETENER_EJECUCION, socketDispatch);
			sacarDeExecute(pcb);
			terminarProceso(pcb, INVALID_RESOURCE);
			haySignal = false;
		}
		free(parametrosNombre1->nombre);
		free(parametrosNombre1);
		break;
	case SOLICITUD_IO_FS_CREATE:
	case SOLICITUD_IO_FS_DELETE:
	case SOLICITUD_IO_FS_TRUNCATE:
	case SOLICITUD_IO_FS_WRITE:
	case SOLICITUD_IO_FS_READ:
		sacarDeExecute(pcb);
		t_parametrosFS *parametrosFS = deserializarParametrosFS(paquete, desplazamiento);
		t_propiedadesEntradaSalida *propiedadesFS = NULL;
		pcb = agregarABlocked(pcb, parametrosFS->parametrosInterfaz->interfaz, parametrosFS->parametrosInterfaz,
				&propiedadesFS);
		if (pcb->estado == BLOCKED) {
			pthread_mutex_lock(&mutexListaAEnviarFS);
			t_envioPcbFS *parametrosEnvioFS = malloc(sizeof(t_envioPcbFS));
			parametrosEnvioFS->propiedades = propiedadesFS;
			parametrosEnvioFS->parametrosFS = parametrosFS;
			parametrosEnvioFS->codigo = paquete->codigo_operacion;
			parametrosEnvioFS->PID = pcb->contextoEjecucion->PID;
			list_add(aEnviarFS, (void*) parametrosEnvioFS);
			if(!parametrosEnvioFS->propiedades->enUso && fsUsada){
				sem_post(&semLivelockFS);
			}
			pthread_mutex_unlock(&mutexListaAEnviarFS);
			if (!fsUsada) {
				pthread_create(&hiloEnvioParametros, NULL, (void*) envioParametrosAFS, aEnviarFS);
				pthread_detach(hiloEnvioParametros);
			}
			fsUsada = true;
		}
		break;
	case INTERRUPTED_BY_USER:
		sacarDeExecute(pcb);
		terminarProceso(pcb, INTERRUPTED_BY_USER);
		break;
	default:
		log_error(auxLogger, "Se desconoce el motivo de desalojo");
		break;
	}
}

void esperarPIDADesbloquear(t_manejoBlocked *pidYInterfaz) {
	t_propiedadesEntradaSalida *propiedadesES = buscarESPorNombre(interfacesActivas, pidYInterfaz->nombreInterfaz);
	pidYInterfaz->pid = recibirPID(propiedadesES->socketConexion);

	pthread_mutex_lock(&mutexListaAEnviarSleep);
	actualizarEnUso(aEnviarSleep, false, pidYInterfaz->nombreInterfaz);
	actualizarListaInterfaces(false, pidYInterfaz->nombreInterfaz);
	sem_post(&semLivelockSleep);
	pthread_mutex_unlock(&mutexListaAEnviarSleep);

	t_pcb *pcbEnBlocked = obtenerSiguienteBlocked(propiedadesES->colaBlocked);

	if (pidYInterfaz->pid == pcbEnBlocked->contextoEjecucion->PID) {
		agregarAColaReady(pcbEnBlocked);
	}

	free(pidYInterfaz->nombreInterfaz);
	free(pidYInterfaz);
}

void esperarPIDADesbloquearFS(t_manejoBlocked *pidYInterfaz) {
	t_propiedadesEntradaSalida *propiedadesES = buscarESPorNombre(interfacesActivas, pidYInterfaz->nombreInterfaz);
	pidYInterfaz->pid = recibirPID(propiedadesES->socketConexion);

	pthread_mutex_lock(&mutexListaAEnviarFS);
	actualizarEnUso(aEnviarFS, false, pidYInterfaz->nombreInterfaz);
	actualizarListaInterfaces(false, pidYInterfaz->nombreInterfaz);
	sem_post(&semLivelockFS);
	pthread_mutex_unlock(&mutexListaAEnviarFS);

	t_pcb *pcbEnBlocked = obtenerSiguienteBlocked(propiedadesES->colaBlocked);

	if (pidYInterfaz->pid == pcbEnBlocked->contextoEjecucion->PID) {
		agregarAColaReady(pcbEnBlocked);
	}
	free(pidYInterfaz->nombreInterfaz);
	free(pidYInterfaz);
}

void esperarPidPcbADesbloquearRead(t_manejoBlocked *pidYInterfaz) {
	t_propiedadesEntradaSalida *propiedadesES = buscarESPorNombre(interfacesActivas, pidYInterfaz->nombreInterfaz);
	pidYInterfaz->codigo = recibirOperacion(propiedadesES->socketConexion);
	t_paquete *paquetePID = crearPaquete(pidYInterfaz->codigo);
	paquetePID->buffer->stream = recibirBuffer(&(paquetePID->buffer->size), propiedadesES->socketConexion);
	int PID = 0;
	memcpy(&PID, paquetePID->buffer->stream, sizeof(int));

	pthread_mutex_lock(&mutexListaAEnviarRead);
	actualizarEnUso(aEnviarRead, false, pidYInterfaz->nombreInterfaz);
	actualizarListaInterfaces(false, pidYInterfaz->nombreInterfaz);
	sem_post(&semLivelockRead);
	pthread_mutex_unlock(&mutexListaAEnviarRead);

	t_pcb *pcbEnBlocked = obtenerSiguienteBlocked(propiedadesES->colaBlocked);

	if (PID == pcbEnBlocked->contextoEjecucion->PID || pidYInterfaz->codigo == SOLICITUD_STDIN
			|| pidYInterfaz->codigo == SOLICITUD_STDOUT) {
		agregarAColaReady(pcbEnBlocked);
	} else {
		sacarDeExecute(pcbEnBlocked);
		terminarProceso(pcbEnBlocked, INVALID_INTERFACE); //TODO en realidad no finaliza por interfaz invalida, VER
	}
	eliminarPaquete(paquetePID);
	free(pidYInterfaz->nombreInterfaz);
	free(pidYInterfaz);
}

void esperarPidPcbADesbloquearWrite(t_manejoBlocked *pidYInterfaz) {
	t_propiedadesEntradaSalida *propiedadesES = buscarESPorNombre(interfacesActivas, pidYInterfaz->nombreInterfaz);
	pidYInterfaz->codigo = recibirOperacion(propiedadesES->socketConexion);
	t_paquete *paquetePID = crearPaquete(pidYInterfaz->codigo);
	paquetePID->buffer->stream = recibirBuffer(&(paquetePID->buffer->size), propiedadesES->socketConexion);
	int PID = 0;
	memcpy(&PID, paquetePID->buffer->stream, sizeof(int));

	pthread_mutex_lock(&mutexListaAEnviarWrite);
	actualizarEnUso(aEnviarWrite, false, pidYInterfaz->nombreInterfaz);
	actualizarListaInterfaces(false, pidYInterfaz->nombreInterfaz);
	sem_post(&semLivelockWrite);
	pthread_mutex_unlock(&mutexListaAEnviarWrite);

	t_pcb *pcbEnBlocked = obtenerSiguienteBlocked(propiedadesES->colaBlocked);

	if (PID == pcbEnBlocked->contextoEjecucion->PID || pidYInterfaz->codigo == SOLICITUD_STDIN
			|| pidYInterfaz->codigo == SOLICITUD_STDOUT) {
		agregarAColaReady(pcbEnBlocked);
	} else {
		sacarDeExecute(pcbEnBlocked);
		terminarProceso(pcbEnBlocked, INVALID_INTERFACE); //TODO en realidad no finaliza por interfaz invalida, VER
	}
	eliminarPaquete(paquetePID);
	free(pidYInterfaz->nombreInterfaz);
	free(pidYInterfaz);
}

void actualizarQuantumRestante(t_pcb *pcb, t_algoritmoPlanificacion algoritmo) {
	if (algoritmo == RR || tiempoTranscurrido >= pcb->quantum) {
		pthread_mutex_lock(&mutexCambioQuantum);
		pcb->quantum = cfgKernel->QUANTUM;
		pthread_mutex_unlock(&mutexCambioQuantum);
	} else {
		pthread_mutex_lock(&mutexCambioQuantum);
		pcb->quantum = cfgKernel->QUANTUM - tiempoTranscurrido;
		pthread_mutex_unlock(&mutexCambioQuantum);
	}
}

int encontrarRecurso(char *cadena, char *array[]) {
	for (int i = 0; array[i] != NULL; i++) {
		if (strcmp(cadena, array[i]) == 0) {
			return i;
		}
	}
	return -1; // No se encontró la cadena en el array
}

int valorSemaforo(sem_t *sem) {
	int sval;
	sem_getvalue(sem, &sval);
	return sval;
}

void enviarPidMemoria(int pid) {
	t_paquete *paqueteFinProceso = crearPaquete(FIN_PROCESO);
	agregarAPaquete(paqueteFinProceso, &pid, sizeof(int));
	enviarPaquete(paqueteFinProceso, socketMemoria);
	eliminarPaquete(paqueteFinProceso);
}

void desasignarRecursos(t_pcb *pcb) {
	int cantidadRecursos = contarRecursos(cfgKernel->INSTANCIAS_RECURSOS);
	for (int i = 0; i < cantidadRecursos ; i++) {
		buscarYEliminarPcbEnCola(colasRecursos[i], pcb);
		if (pcb->recursosAsignados[i] > 0) {
			for (int u = 0; u < pcb->recursosAsignados[i]; u++) {
				sem_post(semaforosRecursos[i]);
				pcb->recursosAsignados[i]--;
				finalizarProcesoConsola = true;
				if (!queue_is_empty(colasRecursos[i])) {
					t_pcb *pcbAReady1 = obtenerSiguienteBlocked(colasRecursos[i]);
					pcbAReady1->recursosAsignados[i]++;
					sem_wait(semaforosRecursos[i]);
					agregarAColaReady(pcbAReady1);
					cambiarEstadoPcb(pcbAReady1, READY);
				}
				finalizarProcesoConsola = false;
			}
		}
	}
}

void terminarProceso(t_pcb *pcb, t_opCode motivo) {
	desasignarRecursos(pcb);
	agregarAExit(pcb, motivo);
	cambiarEstadoPcb(pcb, EXIT);
	pthread_mutex_lock(&mutexListaPcbsEnSistema);
	if (!list_remove_element(pcbsEnSistema, pcb)) {
		log_error(auxLogger, "No se puedo sacar el pcb de la lista pcbsEnSistema");
	}
	pthread_mutex_unlock(&mutexListaPcbsEnSistema);
	enviarPidMemoria(pcb->contextoEjecucion->PID);

}
