#ifndef MEMORIA_H_
#define MEMORIA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <math.h>
#include "conexionMemoria.h"
#include "initMemoria.h"
#include "memoriaInstrucciones.h"
#include "memoriaUsuario.h"
#include "funcionesAuxiliaresMemoria.h"

typedef struct {
	int numeroMarco;
	bool bitValidez;
} t_entradaTablaPaginas;

typedef struct {
	int PID;
	t_list *entradasTablaPaginas;
} t_tablaPaginas;

void liberarRecursos();
void inicializarLoggers();

extern t_config_memoria *cfgMemoria;
extern t_log *logger;
extern t_log *auxLogger;
extern t_list *espacioMemoriaInstruciones;
extern int socketServidorMemoria;
extern void *espacioUsuario;
extern bool *bitMap;
extern t_list *tablasPaginas;

#endif /* MEMORIA_H_ */
