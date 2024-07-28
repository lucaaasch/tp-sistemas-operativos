#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include "interfazGenerica.h"
#include "interfazRead.h"
#include "interfazWrite.h"
#include "fileSystem.h"
#include "initEntradaSalida.h"
#include <utils/include/comunicacion.h>
#include <utils/include/estructuras.h>
#include "../include/conexionEntradaSalida.h"

extern t_list *mapeoBloques; // (t_bloque*)
extern t_list *mapeoArchivos; // (t_metadata*)
extern char *pathArchivoBloques; // (void*)
extern char *pathArchivoBitMap; // (t_bitarray)

extern t_config_entradasalida *cfgEntradaSalida;
extern t_log *logger;
extern t_log *auxLogger;
extern int socketConexionMemoria, socketConexionKernel;

void liberarPrograma();
void inicializarLoggers();

#endif /* ENTRADASALIDA_H_ */
