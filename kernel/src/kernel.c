#include <stdlib.h>
#include <stdio.h>
#include "../include/kernel.h"

t_config_kernel *cfgKernel;
t_log *auxLogger;
int socketMemoria, socketDispatch, socketInterrupt, socketServerKernel, socketEntradaSalida;
pthread_t planificadorLargoPlazo;
t_list *interfacesActivas;

int main(int argc, char *argv[]) {

	interfacesActivas = list_create();
	// Iniciamos los loggers
	inicializarLoggers();

	// Iniciamos la config
	inicializarConfig(argv[1]);

	// Conectamos KERNEL a CPU
	conectarACpu();

	// Conectamos KERNEL a MEMORIA
	conectarAMemoria();

	//Inicializamos semaforos y estructuras Planificacion
	inicializarPlanificacion();

	//Levantamos hilos de planificacion
	levantarPlanificador();

	//Levantamos la Consola
	inicializarConsola();

	// Levantamos el servidor para ENTRADA/SALIDA
	levantarServidor();

	// Liberamos el programa
	liberarRecursos();

	return 0;
}

void inicializarLoggers() {
	auxLogger = iniciarLogger("kernel.log", "KERNEL", 1, LOG_LEVEL_TRACE);
}

void liberarEntradasYSalidas (){
	for(int i=0; i<list_size(interfacesActivas);i++){
	t_propiedadesEntradaSalida *entradaSalida = list_get(interfacesActivas,i);
	free(entradaSalida->nombre);
	queue_destroy_and_destroy_elements(entradaSalida->colaBlocked,(void*)liberarPcb);
	}
}

void liberarRecursos() {
	log_destroy(auxLogger);
	list_destroy_and_destroy_elements (interfacesActivas,(void*)liberarEntradasYSalidas);
	liberarConfig();
	liberarConexion(socketMemoria);
	liberarConexion(socketDispatch);
	liberarConexion(socketInterrupt);
	liberarConexion(socketServerKernel);
	liberarEstructurasPlanificacion();
}

void inicializarConsola() {
	pthread_t hiloConsola;
	pthread_create(&hiloConsola, NULL, (void*) consola, NULL);
	pthread_detach(hiloConsola);
}

void levantarPlanificador() {
	pthread_create(&planificadorLargoPlazo, NULL, planificarLargoPlazo, NULL);
	pthread_detach(planificadorLargoPlazo);

}


