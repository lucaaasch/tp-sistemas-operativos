#ifndef INCLUDE_INTERFAZGENERICA_H_
#define INCLUDE_INTERFAZGENERICA_H_
#include <pthread.h>
#include <semaphore.h>
#include "entradasalida.h"
#include <time.h>
#include <utils/include/estructuras.h>
#include <utils/include/comunicacion.h>
#include <unistd.h>

void dormirInterfaz (t_parametrosInterfaz* param, int pid);
#endif /* INCLUDE_INTERFAZGENERICA_H_ */
