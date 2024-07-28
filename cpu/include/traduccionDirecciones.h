#ifndef INCLUDE_TRADUCCIONDIRECCIONES_H_
#define INCLUDE_TRADUCCIONDIRECCIONES_H_

#define ERROR_RECIBIR_MARCO -1
#define ERROR_NUMERO_MARCO -1
#define TLB_MISS -1

#include <time.h>

typedef struct {
	int PID;
	int pagina;
	int marco;
	time_t ultimoAcceso;  // Para LRU
} entradaTLB;

extern t_list* tlb;

int* traducirDirecciones(int direccionLogica, int sizeLectura,int* cantidadDireccionesFisicas);

#endif /* INCLUDE_TRADUCCIONDIRECCIONES_H_ */
