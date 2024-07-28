#include "../include/memoria.h"

pthread_mutex_t mutexEspacioMemoriaInstrucciones;

void leerInstruccion(FILE *archivoPseudocodigo, t_memoriaInstruccion *memoriaInstruccion) {
	char *bufferInstruccion = NULL;
	size_t size = 0;
	getline(&bufferInstruccion, &size, archivoPseudocodigo);
	list_add(memoriaInstruccion->pseudocodigo, bufferInstruccion);
}

t_memoriaInstruccion* inicializarBloqueMemoriaInstruccion(int PID) {
	t_memoriaInstruccion *memoriaInstruccion = malloc(sizeof(t_memoriaInstruccion));
	memoriaInstruccion->PID = PID;
	memoriaInstruccion->pseudocodigo = list_create();
	return memoriaInstruccion;
}

char* obtenerPathCompleto(char *path) {
	char *pathCompleto = malloc(strlen(path) + strlen(cfgMemoria->PATH_INSTRUCCIONES) + 1);
	strcpy(pathCompleto, cfgMemoria->PATH_INSTRUCCIONES);
	strcat(pathCompleto, path);
	free(path);
	return pathCompleto;
}

void cargarInstrucciones(char *path, int PID) {
	t_memoriaInstruccion *memoriaInstruccion = inicializarBloqueMemoriaInstruccion(PID);
	char *pathCompleto = obtenerPathCompleto(path);
	FILE *archivoPseudocodigo = fopen(pathCompleto, "r+b");

	fseek(archivoPseudocodigo, 0, SEEK_SET);
	while (!feof(archivoPseudocodigo)) {
		leerInstruccion(archivoPseudocodigo, memoriaInstruccion);
	}

	pthread_mutex_lock(&mutexEspacioMemoriaInstrucciones);
	list_add(espacioMemoriaInstruciones, memoriaInstruccion);
	pthread_mutex_unlock(&mutexEspacioMemoriaInstrucciones);

	fclose(archivoPseudocodigo);
	free(pathCompleto);
}

t_memoriaInstruccion* obtenerBloqueMemoriaSegunPID(int PID) {
	t_memoriaInstruccion *bloqueMemoria;

	pthread_mutex_lock(&mutexEspacioMemoriaInstrucciones);
	int listSize = list_size(espacioMemoriaInstruciones);
	for (int i = 0; i < listSize; i++) {
		bloqueMemoria = list_get(espacioMemoriaInstruciones, i);
		if (bloqueMemoria->PID == PID) {
			pthread_mutex_unlock(&mutexEspacioMemoriaInstrucciones);
			return bloqueMemoria;
		}
	}
	pthread_mutex_unlock(&mutexEspacioMemoriaInstrucciones);
	return NULL;
}

void enviarInstruccionACPU(int PID, uint32_t PC, int socketClienteCpu) {
	t_memoriaInstruccion *memoriaInstruccion = obtenerBloqueMemoriaSegunPID(PID);
	char *instruccion = list_get(memoriaInstruccion->pseudocodigo, PC);
	t_paquete *paquete = crearPaquete(INTERCAMBIO_INSTRUCCION);
	agregarAPaquete(paquete, instruccion, strlen(instruccion) + 1);
	usleep(cfgMemoria->RETARDO_RESPUESTA * 1000);
	enviarPaquete(paquete, socketClienteCpu);
	eliminarPaquete(paquete);
}

void liberarBloqueMemoriaInstruccion(int PID) {
	t_memoriaInstruccion *memoriaInstruccion = obtenerBloqueMemoriaSegunPID(PID);

	pthread_mutex_lock(&mutexEspacioMemoriaInstrucciones);
	list_remove_element(espacioMemoriaInstruciones, memoriaInstruccion);
	pthread_mutex_unlock(&mutexEspacioMemoriaInstrucciones);

	list_destroy_and_destroy_elements(memoriaInstruccion->pseudocodigo, (void*) free);
	free(memoriaInstruccion);
}
