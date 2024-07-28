#ifndef INCLUDE_CONEXIONCPUKERNEL_H_
#define INCLUDE_CONEXIONCPUKERNEL_H_

#include <semaphore.h>

extern int socketServerDispatch;
extern int socketServerInterrupt;
extern int socketClienteDispatch;
extern int socketClienteInterrupt;
extern sem_t semaforoCPULibre;
extern sem_t semaforoCPUOcupada;

void iniciarServidores();
void conectarDispatch();
void conectarInterrupt();
void esperarContextos();



#endif /* INCLUDE_CONEXIONCPUKERNEL_H_ */
