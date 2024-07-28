#ifndef INCLUDE_INTERFAZWRITE_H_
#define INCLUDE_INTERFAZWRITE_H_

#include <pthread.h>
#include <semaphore.h>
#include "entradasalida.h"
#include <utils/include/estructuras.h>
#include <unistd.h>

void mostrarEnConsola(t_parametrosInterfaz *parametros, int PID);

#endif /* INCLUDE_INTERFAZWRITE_H_ */
