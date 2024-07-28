#ifndef CONEXIONENTRADASALIDA_H_
#define CONEXIONENTRADASALIDA_H_
#include <stdbool.h>
#include <commons/log.h>

bool generarConexiones(int *socketConexionMemoria, int *socketConexionKernel);
bool tipoInterfazConcuerda(t_opCode tipoInterfaz);

char* consultarNombre();

void enviarEntrada(t_parametrosInterfaz *parametros, int PID, t_opCode operacion);
void enviarEntradaConBuffer(t_parametrosInterfaz *parametros, char *buffer, int PID, t_opCode operacion);
void esperarEntradaSalida();
void enviarPIDAKernel(int PID, t_opCode operacion);

void fsWrite(t_parametrosFS* parametros, int PID);
void fsRead(t_parametrosFS* parametros, int PID);

#endif /* CONEXIONENTRADASALIDA_H_ */
