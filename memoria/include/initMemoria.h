
#ifndef INCLUDE_INITMEMORIA_H_
#define INCLUDE_INITMEMORIA_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/config.h>

typedef struct {
	char *PUERTO_ESCUCHA;
	uint16_t TAM_MEMORIA;
	uint16_t TAM_PAGINA;
	char *PATH_INSTRUCCIONES;
	uint16_t RETARDO_RESPUESTA;
} t_config_memoria;

void inicializarConfigMemoria();
void liberarConfigMemoria();

#endif /* INCLUDE_INITMEMORIA_H_ */
