#ifndef INCLUDE_FILESYSTEM_H_
#define INCLUDE_FILESYSTEM_H_

typedef struct {
	int idBloque;
	bool ocupado;
} t_bloque;

typedef struct {
	char *nombreArchivo;
	int primerBloque;
	int cantidadBytes;
	int cantidadBloques;
} t_metadata;

void inicializarFileSystem();
void crearArchivo(char *nombreArchivo);
void eliminarArchivo(char *nombreArchivo);
void leerDeFileSystem(char *nombreArchivo, int tamanioBuffer, void *buffer, int punteroArchivo);
void escribirEnFileSystem(char *nombreArchivo, int tamanioBuffer, void *buffer, int punteroArchivo);
void truncarArchivo(char *nombreArchivo, int bytesTruncado,int PID);

#endif /* INCLUDE_FILESYSTEM_H_ */
