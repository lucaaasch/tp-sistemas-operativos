#ifndef INCLUDE_INITCPU_H_
#define INCLUDE_INITCPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>

typedef struct {
	char* IP_MEMORIA;
	char* PUERTO_MEMORIA;
	char* PUERTO_ESCUCHA_DISPATCH;
	char* PUERTO_ESCUCHA_INTERRUPT;
	uint16_t CANTIDAD_ENTRADAS_TLB;
	char* ALGORITMO_TLB;
}t_config_cpu;

void inicializarConfigCpu();
void liberarConfigCpu();

#endif /* INCLUDE_INITCPU_H_ */
