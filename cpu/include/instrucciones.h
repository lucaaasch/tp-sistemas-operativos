#ifndef INCLUDE_INSTRUCCIONES_H_
#define INCLUDE_INSTRUCCIONES_H_

void* obtenerRegistro(char *nombreRegistro, int *sizeRegistro);
int obtenerValorNumerico(char *strNumber);

void set(const t_instruccion *instruccion);
void sum(const t_instruccion *instruccion);
void sub(const t_instruccion *instruccion);
void jnz(const t_instruccion *instruccion);
void ioGenSleep(const t_instruccion *instruccion);
void instruccionExit();
void movIn(const t_instruccion *instruccion);
void movOut(const t_instruccion *instruccion);
void resize(const t_instruccion *instruccion);
void copyString(const t_instruccion *instruccion);
void ioStdinRead(const t_instruccion *instruccion);
void ioStdoutWrite(const t_instruccion *instruccion);
void ioFSCreateOrDelete(const t_instruccion *instruccion, t_opCode operacion);
void ioFSTruncate(const t_instruccion *instruccion);
void ioFSWriteOrRead(const t_instruccion *instruccion, t_opCode operacion);
void instruccionWait(const t_instruccion *instruccion);
void instruccionSignal(const t_instruccion *instruccion);

#endif /* INCLUDE_INSTRUCCIONES_H_ */
