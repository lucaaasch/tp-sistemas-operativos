#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <stdbool.h>
#include <math.h>
#include "../include/conexionCpuMemoria.h"
#include "../include/conexionCpuKernel.h"
#include "../include/cicloInstruccionCpu.h"
#include "../include/instrucciones.h"
#include "../include/traduccionDirecciones.h"
#include "../include/funcionesAuxiliaresCpu.h"
#include <utils/include/comunicacion.h>
#include <utils/include/estructuras.h>
#include <pthread.h>

#include "initCpu.h"

extern t_config_cpu *cfgCpu;
extern t_log *logger;
extern t_log *auxLogger;
extern bool mantenerEnExec;
extern sem_t manejoRecursos;
extern int tamPagina;

#endif /* CPU_H_ */
