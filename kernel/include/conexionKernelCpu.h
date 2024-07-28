
#ifndef INCLUDE_CONEXIONKERNELCPU_H_
#define INCLUDE_CONEXIONKERNELCPU_H_
#include "../include/kernel.h"
#include <utils/include/estructuras.h>

extern int socketDispatch;
extern int socketInterrupt;

void conectarACpu ();
void enviarContextoEjecucion (t_contextoEjecucion* contexto , int socket,t_opCode codigo);

#endif /* INCLUDE_CONEXIONKERNELCPU_H_ */
