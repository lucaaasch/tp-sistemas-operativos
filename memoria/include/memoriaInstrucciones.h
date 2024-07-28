#ifndef INCLUDE_MEMORIAINSTRUCCIONES_H_
#define INCLUDE_MEMORIAINSTRUCCIONES_H_

typedef struct{
	int PID;
	t_list* pseudocodigo;
}t_memoriaInstruccion;

void cargarInstrucciones(char *path, int PID);
void enviarInstruccionACPU(int PID, uint32_t PC, int socketClienteKernel);
void liberarBloqueMemoriaInstruccion(int PID);

extern pthread_mutex_t mutexEspacioMemoriaInstrucciones;

#endif
