#include "../include/memoria.h"
#include <errno.h>
#include <signal.h>
#define SHUTDOWN_ERROR -1
#define TIME_WAIT 1

void inicializarSemaforos() {
	pthread_mutex_init(&mutexTablasPaginas, (void*) NULL);
	pthread_mutex_init(&mutexBitMap, (void*) NULL);
	pthread_mutex_init(&mutexEspacioMemoriaInstrucciones, (void*) NULL);
	pthread_mutex_init(&mutexEspacioUsuario, (void*) NULL);
}

void inicializarVariablesGlobales() {

	espacioMemoriaInstruciones = list_create();

	espacioUsuario = malloc(cfgMemoria->TAM_MEMORIA);

	int cantidadMarcos = floor((double) cfgMemoria->TAM_MEMORIA / cfgMemoria->TAM_PAGINA);
	bitMap = malloc(sizeof(bool) * cantidadMarcos);
	for (int i = 0; i < cantidadMarcos; i++) {
		bitMap[i] = 0;
	}

	tablasPaginas = list_create();

	inicializarSemaforos();
}

void liberarRecursos() {
	if (auxLogger != NULL) {
		log_destroy(auxLogger);
	}
	if (cfgMemoria != NULL) {
		liberarConfigMemoria();
	}
	if (socketServidorMemoria != -1) {
		close(socketServidorMemoria);
	}
}

void inicializarLoggers() {
	auxLogger = iniciarLogger("memoria.log", "MEMORIA", 1, LOG_LEVEL_TRACE);
}

void obtenerErrorShutdown() {
	switch (errno) {
	case EBADF:
		log_error(auxLogger, "Descriptor de socket no válido");
		break;
	case EACCES:
		log_error(auxLogger, "Permisos insuficientes para cerrar el socket");
		break;
	case ENOTSOCK:
		log_error(auxLogger, "El descriptor no es un socket");
		break;
	case EIO:
		log_error(auxLogger, "Error de entrada/salida");
		break;
	default:
		log_error(auxLogger, "Error de shutdown desconocido");
	}
}

void sigintHandler(int signalNumber) {

	int shutdownRet = shutdown(socketServidorMemoria, SHUT_RDWR);
	if (shutdownRet == SHUTDOWN_ERROR) {
		obtenerErrorShutdown();
		liberarRecursos();
		exit(EXIT_FAILURE);
	}

	sleep(TIME_WAIT);

	int closeRet = close(socketServidorMemoria);
	socketServidorMemoria = -1;

	if (closeRet != 0) {
		log_error(auxLogger, "Error al cerrar el socket");
		liberarRecursos();
		exit(EXIT_FAILURE);

	} else {
		log_info(auxLogger, "Socket cerrado exitosamente\n");
		liberarRecursos();
		exit(EXIT_SUCCESS);
	}
}

void handleSIGINT() {
	struct sigaction sig_action;
	sig_action.sa_handler = sigintHandler;
	sigaction(SIGINT, &sig_action, NULL);
}
