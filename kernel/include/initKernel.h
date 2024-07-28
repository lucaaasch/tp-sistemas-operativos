#ifndef INIT_KERNEL_H_
#define INIT_KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>
#include <commons/collections/list.h>


typedef struct {
	char *PUERTO_ESCUCHA;
	char *IP_MEMORIA;
	char *PUERTO_MEMORIA;
	char *IP_CPU;
	char *PUERTO_CPU_DISPATCH;
	char *PUERTO_CPU_INTERRUPT;
	char *ALGORITMO_PLANIFICACION;
	uint16_t QUANTUM;
	char **RECURSOS;
	char ** INSTANCIAS_RECURSOS;
	uint16_t GRADO_MULTIPROGRAMACION;
} t_config_kernel;

void inicializarConfig();
void liberarConfig();
void convertirInstancias();
int contarRecursos(char **recursos);

#endif /* INIT_KERNEL_H_ */
