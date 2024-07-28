
#ifndef INCLUDE_CONEXIONKERNELENTRADASALIDA_H_
#define INCLUDE_CONEXIONKERNELENTRADASALIDA_H_
#include "../include/kernel.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/include/comunicacion.h>

extern int socketServerKernel;

void levantarServidor();

#endif /* INCLUDE_CONEXIONKERNELENTRADASALIDA_H_ */
