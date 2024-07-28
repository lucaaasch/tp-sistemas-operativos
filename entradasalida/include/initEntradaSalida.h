#ifndef INCLUDE_INITENTRADASALIDA_H_
#define INCLUDE_INITENTRADASALIDA_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <commons/config.h>

typedef struct {
	char *TIPO_INTERFAZ; //GENERICA-STDIN-STDOUT-DIALFS
	uint16_t TIEMPO_UNIDAD_TRABAJO; //En ms
	char *IP_KERNEL;
	char *PUERTO_KERNEL;
	char *IP_MEMORIA;
	char *PUERTO_MEMORIA;
	char *PATH_BASE_DIALFS; //Path donde se van a encontrar los archivos de DialFS
	uint16_t BLOCK_SIZE; //Tamanio de los bloques del FS
	uint16_t BLOCK_COUNT; //Cantidad de bloques del FS
	uint16_t RETRASO_COMPACTACION;
} t_config_entradasalida;

bool inicializarConfig();
void liberarConfig();
#endif /* INCLUDE_INITENTRADASALIDA_H_ */
