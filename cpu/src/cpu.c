#include <stdlib.h>
#include <stdio.h>
#include "../include/cpu.h"

t_config_cpu *cfgCpu;
t_log *auxLogger;
bool mantenerEnExec;
sem_t manejoRecursos;
int socketClienteMemoria;
int socketServerDispatch, socketServerInterrupt;
int socketClienteDispatch, socketClienteInterrupt;

int tamPagina;

int main(int argc, char *argv[]) {

	handleSIGINT();

	sem_init(&manejoRecursos, 0, 0);

	inicializarLoggers();

	inicializarConfigCpu(argv[1]);

	conectarAMemoria();

	// Levanto los servidores para DISPATCH e INTERRUPT

	iniciarServidores();

	inicializarVariablesGlobales();

	// Si ambos servidores se inician, realizamos las conexiones con KERNEL
	conectarInterrupt();
	conectarDispatch();

	// Libero los recursos asociados al programa

	liberarRecursosCpu();

	return 0;
}
