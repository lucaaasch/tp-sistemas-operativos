#ifndef INCLUDE_MEMORIAUSUARIO_H_
#define INCLUDE_MEMORIAUSUARIO_H_

void crearTablaPaginas(int PID);
void liberarEspacioUsuario(int PID);
int obtenerNumeroMarcoAccediendoATablaPaginas(int PID, int numeroPagina);
bool redimensionar(int PID, int tamanio);
bool leerDeMemoria(int *direccionesFisicas, int cantidadDirecciones, int tamanioLectura, void *buffer);
bool escribirEnMemoria(int *direccionesFisicas, int cantidadDirecciones, int tamanioEscritura, void *buffer);

extern pthread_mutex_t mutexEspacioUsuario;
extern pthread_mutex_t mutexBitMap;
extern pthread_mutex_t mutexTablasPaginas;

#endif /* INCLUDE_MEMORIAUSUARIO_H_ */
