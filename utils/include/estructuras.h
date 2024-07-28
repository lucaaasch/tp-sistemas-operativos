#ifndef SRC_UTILS_INCLUDE_ESTRUCTURAS_H_
#define SRC_UTILS_INCLUDE_ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <stdint.h>
#include <semaphore.h>

// TODO ACTUALIZAR LOS OPCODES
typedef enum {
	MENSAJE,
	PAQUETE,
	CREAR_PROCESO,
	CONTEXTO_EJECUCION,
	FIN_QUANTUM,
	FIN_PROCESO,
	SOLICITUD_INSTRUCCION,
	INTERCAMBIO_INSTRUCCION,
	CONTINUAR_EJECUCION,
	SYSCALL_IO_GEN_SLEEP,
	SYSCALL_EXIT,
	SYSCALL_WAIT,
	SYSCALL_SIGNAL,
	EXCEPCION_CANTIDAD_PARAMETROS,
	SYSCALL_IO_STDIN_READ,
	SYSCALL_IO_STDOUT_WRITE,
	DIAL_FS,
	NOMBRE_ENTRADA_SALIDA,
	SOLICITUD_NUMERO_MARCO,
	SOLICITUD_TAM_PAGINA,
	FINALIZACION_PROCESO,
	SOLICITUD_LECTURA,
	SOLICITUD_ESCRITURA,
	SOLICITUD_RESIZE,
	ERROR_OUT_OF_MEMORY,
	SOLICITUD_STDIN,
	SOLICITUD_STDOUT,
	ESCRITURA_OK,
	ESCRITURA_ERROR,
	ACCESO_OK,
	ACCESO_ERROR,
	PROCESO_OK,
	SOLICITUD_IO_FS_CREATE,
	SOLICITUD_IO_FS_DELETE,
	SOLICITUD_IO_FS_TRUNCATE,
	SOLICITUD_IO_FS_WRITE,
	SOLICITUD_IO_FS_READ,
	DETENER_EJECUCION,
	INTERRUPTED_BY_USER,
	INVALID_RESOURCE,
	INVALID_INTERFACE
} t_opCode;

typedef enum {
	SET,
	SUM,
	SUB,
	JNZ,
	IO_GEN_SLEEP,
	INSTRUCCION_EXIT,
	MOV_IN,
	MOV_OUT,
	RESIZE,
	COPY_STRING,
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	WAIT,
	SIGNAL,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ
} t_operacion;

typedef struct {
	int *listaDirecciones;
	int cantidadDirecciones;
} t_direccionesFisicas;

typedef struct {
	char *interfaz;
	int largoInterfaz;
	char *buffer;
	int largoTexto; // TODO cambiar esto
	int tiempoSleep;
	t_direccionesFisicas *direccionesFisicas;
	int limiteEntrada;
} t_parametrosInterfaz;

typedef struct {
	char* nombreArchivo;
	int largoNombreArchivo;
	int puntero;
	t_parametrosInterfaz* parametrosInterfaz;
} t_parametrosFS;

typedef enum {
	NEW, READY, EXIT, EXEC, BLOCKED
} t_estado;

typedef struct {
	t_operacion operacion; //EJ:SET,SUM,SUB
	int cantidadParametros;
	char **parametros;
} t_instruccion;

typedef struct {
	int size;
	void *stream;
} t_buffer;

typedef struct {
	t_opCode codigo_operacion;
	t_buffer *buffer;
} t_paquete;

typedef struct {
	uint32_t SI;
	uint32_t DI;
} t_registrosMemoria;

typedef struct {
	uint8_t AX;
	uint8_t BX;
	uint8_t CX;
	uint8_t DX;
	uint32_t EAX;
	uint32_t EBX;
	uint32_t ECX;
	uint32_t EDX;
} t_registrosGenerales;

// Declaración de t_contextoEjecucion
typedef struct {
	int PID;
	uint32_t PC;
	t_registrosGenerales *registrosGenerales;  // Uso del tipo declarado anteriormente
	t_registrosMemoria *registrosMemoria;
} t_contextoEjecucion;

typedef struct {
	//Quiza hay que agregar t_list * archivosAbiertos
	uint16_t quantum;
	t_contextoEjecucion *contextoEjecucion;
	//VER DE AGREGAR ARCHIVOS ABIERTOS
	t_estado estado;
	int *recursosAsignados;
} t_pcb;

//TODO algoritmos es solo de kernel
typedef enum {
	FIFO, RR, VRR
} t_algoritmoPlanificacion;

#endif /* SRC_UTILS_INCLUDE_ESTRUCTURAS_H_ */
