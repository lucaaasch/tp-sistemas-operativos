#include "../include/cpu.h"
#include <errno.h>
#include <signal.h>

#define SHUTDOWN_ERROR -1
#define TIME_WAIT 1

void inicializarVariablesGlobales() {
	tlb = list_create();
}

void liberarEntradaTLB(void *elemento) {
	if (elemento != NULL) {
		entradaTLB *entrada = (entradaTLB*) elemento;
		free(entrada);
	}
}

void liberarTLB() {
	list_destroy_and_destroy_elements(tlb, liberarEntradaTLB);
}

void liberarSockets() {
	if (socketClienteDispatch != -1) {
		liberarConexion(socketServerDispatch);
	}
	if (socketClienteInterrupt != -1) {
		liberarConexion(socketClienteInterrupt);
	}
	if (socketClienteMemoria != -1) {
		liberarConexion(socketClienteMemoria);
	}
	if (socketServerDispatch != -1) {
		liberarConexion(socketServerDispatch);
	}
	if (socketServerInterrupt != -1) {
		liberarConexion(socketServerInterrupt);
	}
}

void liberarRecursosCpu() {
	if (auxLogger != NULL) {
		log_destroy(auxLogger);
	}
	if (cfgCpu != NULL) {
		liberarConfigCpu();
	}
	if (tlb != NULL) {
		liberarTLB();
	}
	liberarSockets();
}

void inicializarLoggers() {
	auxLogger = iniciarLogger("cpu.log", "CPU", 1, LOG_LEVEL_TRACE);
}

void cerrarSockets() {
	int contadorErrores = 0;

	int closeCM = close(socketClienteMemoria);
	socketClienteMemoria = -1;
	if (closeCM != 0) {
		log_error(auxLogger, "Error al cerrar el socket de <Cliente Memoria>");
		contadorErrores++;
	}
	int closeCD = close(socketClienteDispatch);
	socketClienteDispatch = -1;
	if (closeCD != 0) {
		log_error(auxLogger, "Error al cerrar el socket de <Cliente Dispatch>" );
		contadorErrores++;
	}
	int closeCI = close(socketClienteInterrupt);
	socketClienteInterrupt = -1;
	if (closeCI != 0) {
		log_error(auxLogger, "Error al cerrar el socket de <Cliente Interrupt>");
		contadorErrores++;
	}
	int closeSD = close(socketServerDispatch);
	socketServerDispatch = -1;
	if (closeSD != 0) {
		log_error(auxLogger, "Error al cerrar el socket de <Server Dispatch>");
		contadorErrores++;
	}
	int closeSI = close(socketServerInterrupt);
	socketServerInterrupt = -1;
	if (closeSI != 0) {
		log_error(auxLogger, "Error al cerrar el socket de <Server Interrupt>");
		contadorErrores++;
	}
	if(contadorErrores == 0){
		log_info(auxLogger,"Todos los sockets fueron cerrados correctamente.");
	}
}

void obtenerErrorShutdown(char* socket){
	printf("El socket de %s tuvo el siguiente error:\n",socket);
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

void shutdownSockets(){

	bool shutdownError = false;

	int shutCM = shutdown(socketClienteMemoria, SHUT_RDWR);
	if (shutCM == SHUTDOWN_ERROR) {
		obtenerErrorShutdown("<Cliente Memoria>");
		shutdownError = true;
	}
	int shutCD = shutdown(socketClienteDispatch, SHUT_RDWR);
	if (shutCD == SHUTDOWN_ERROR) {
		obtenerErrorShutdown("<Cliente Dispatch>");
		shutdownError = true;
	}
	int shutCI = shutdown(socketClienteInterrupt,SHUT_RDWR);
	if (shutCI == SHUTDOWN_ERROR) {
		obtenerErrorShutdown("<Cliente Interrupt>");
		shutdownError = true;
	}
	int shutSD = shutdown(socketServerDispatch,SHUT_RDWR);
	if (shutSD == SHUTDOWN_ERROR) {
		obtenerErrorShutdown("<Server Dispatch>");
		shutdownError = true;
	}
	int shutSI = shutdown(socketServerInterrupt,SHUT_RDWR);
	if (shutSI == SHUTDOWN_ERROR) {
		obtenerErrorShutdown("<Server Interrupt>");
		shutdownError = true;
	}
	if(shutdownError){
		liberarRecursosCpu();
		exit(EXIT_FAILURE);
	}
}

void abortarCPU(){
	shutdownSockets();
	sleep(TIME_WAIT);
	cerrarSockets();
	liberarRecursosCpu();
	exit(EXIT_FAILURE);
}

void manejarSIGINT(int signalNumber) {
	abortarCPU();
}

void handleSIGINT() {
	struct sigaction sig_action;
	sig_action.sa_handler = manejarSIGINT;
	sigaction(SIGINT, &sig_action, NULL);
}
