#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <stdbool.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "conexionKernelCpu.h"
#include "conexionKernelEntradaSalida.h"
#include "conexionKernelMemoria.h"
#include "initKernel.h"
#include "consola.h"
#include "planificacion.h"

extern t_config_kernel* cfgKernel;
extern t_log* logger;
extern t_log* auxLogger;
extern int socketMemoria,socketDispatch,socketInterrupt,socketServerKernel,socketEntradaSalida;
extern int* instanciasRecursos;


extern t_list* interfacesActivas;


void liberarRecursos();
void inicializarLoggers();
void inicializarConsola();
void levantarPlanificador ();

#endif /* KERNEL_H_ */
