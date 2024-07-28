#ifndef INCLUDE_CICLOINSTRUCCIONCPU_H_
#define INCLUDE_CICLOINSTRUCCIONCPU_H_
#include <utils/include/estructuras.h>

extern sem_t atenderInterrupciones;
extern sem_t mutexContexto;
extern t_contextoEjecucion *contexto;

// Funciones para fetch
void solicitarInstruccionAMemoria();
char* recibirInstruccion(int socket_cliente);

// Funciones para decode

t_operacion stringToOperacion(char *operacion);

// Ciclo de instruccion
void cicloInstruccion();

char* fetch();
const t_instruccion* decode(char *instruccionADecodificar);
void execute(const t_instruccion *instruccion);

// Otras funciones para el ciclo de instruccion

void inicializarSemaforos();
void recibirInterrupciones();
bool hayContexto();

#endif /* INCLUDE_CICLOINSTRUCCIONCPU_H_ */
