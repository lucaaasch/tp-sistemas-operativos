#include <stdlib.h>
#include <stdio.h>
#include "../include/memoria.h"
#include <utils/include/comunicacion.h>

t_config_memoria *cfgMemoria;
t_log *auxLogger;
int socketServidorMemoria;

void *espacioUsuario;
bool *bitMap;
t_list *tablasPaginas;
t_list *espacioMemoriaInstruciones;

int main(int argc, char *argv[]) {

	handleSIGINT();

	inicializarLoggers();

	inicializarConfigMemoria(argv[1]);

	iniciarServidorMemoria();

	inicializarVariablesGlobales();

	serverEscuchar(socketServidorMemoria);

	liberarRecursos();

	return 0;
}
