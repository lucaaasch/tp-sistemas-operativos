#ifndef INCLUDE_INTERFAZREAD_H_
#define INCLUDE_INTERFAZREAD_H_
#include <pthread.h>
#include <semaphore.h>
#include "entradasalida.h"
#include <utils/include/estructuras.h>
#include <unistd.h>

void esperarEntradaPorTeclado(t_parametrosInterfaz *parametros, int PID);

#endif /* INCLUDE_INTERFAZREAD_H_ */
