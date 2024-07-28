#include "../include/cpu.h"

sem_t semaforoCPUOcupada;

void iniciarServidores() {

	int huboErrores = 0;

	socketServerDispatch = iniciarServidor(cfgCpu->PUERTO_ESCUCHA_DISPATCH, auxLogger, "CPU");

	if (socketServerDispatch == -1) {
		log_error(auxLogger, "No se pudo iniciar el server de DISPATCH");
		huboErrores = 1;
	}

	socketServerInterrupt = iniciarServidor(cfgCpu->PUERTO_ESCUCHA_INTERRUPT, auxLogger, "CPU");

	if (socketServerInterrupt == -1) {
		log_error(auxLogger, "No se pudo iniciar el server de INTERRUPT");
		huboErrores = 1;
	}

	if (huboErrores) {
		abort();
	}
}

void conectarDispatch() {

	socketClienteDispatch = accept(socketServerDispatch, NULL, NULL);

	if (socketClienteDispatch == -1) {
		log_error(auxLogger, "KERNEL no se pudo conectar con DISPATCH");
		abort();
	}

	pthread_t conexionDispatch;
	pthread_create(&conexionDispatch, NULL, (void*) esperarContextos, NULL);
	pthread_join(conexionDispatch, NULL);
}

void conectarInterrupt() {

	socketClienteInterrupt = accept(socketServerInterrupt, NULL, NULL);

	if (socketClienteInterrupt == -1) {
		log_error(auxLogger, "KERNEL no se pudo conectar con DISPATCH");
		abort();
	}

	// El hilo que recibe las interrupciones se crea en el ciclo de instruccion
}

void manejarSolicitudesManejoRecursos(t_opCode operacion) {
	if (operacion == CONTINUAR_EJECUCION) {
		mantenerEnExec = true;
		sem_post(&manejoRecursos);
	} else if (operacion == DETENER_EJECUCION) {
		mantenerEnExec = false;
		sem_post(&manejoRecursos);
	}
}

void recibirSolicitudesManejoRecursos() {
	while (true) {
		sem_wait(&semaforoCPUOcupada);
		t_opCode operacion = recibirOperacion(socketClienteDispatch);
		manejarSolicitudesManejoRecursos(operacion);
	}
}

void esperarContextos() {
	sem_init(&semaforoCPUOcupada, 0, 0);
	int contador = 0;
	pthread_t manejoRecursos;
	pthread_create(&manejoRecursos, NULL, (void*) recibirSolicitudesManejoRecursos, NULL);
	pthread_detach(manejoRecursos);
	while (true) {
		t_contextoEjecucion *contexto;
		t_opCode operacion = recibirOperacion(socketClienteDispatch);
		if (operacion == CONTEXTO_EJECUCION) {
			contexto = recibirContexto(socketClienteDispatch);
			cicloInstruccion(contexto);
		} else {
			log_error(auxLogger, "Operacion recibida desconocida");
			contador++;
			if(contador > 2){
				exit(EXIT_FAILURE);
			}
		}
	}
}

