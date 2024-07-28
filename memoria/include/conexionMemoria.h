
#ifndef INCLUDE_CONEXIONMEMORIA_H_
#define INCLUDE_CONEXIONMEMORIA_H_
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <utils/include/comunicacion.h>

void iniciarServidorMemoria();
bool serverEscuchar(int socketServer);
char* recibirPathInstrucciones(int socketClienteKernel);

#endif /* INCLUDE_CONEXIONMEMORIA_H_ */
