#ifndef HELLO_H_
#define HELLO_H_

#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>
#include "estructuras.h"

//void inicializarLoggers(char *modulo, t_log **logger, t_log **loggerAuxiliar);
int crearConexion(char *ip, char *puerto, t_log *log);
void enviarMensaje(char *mensaje, int socket_cliente);
void crearBufferParaPaquete(t_paquete *paquete);
t_buffer* crearBuffer();
void liberarBuffer(t_buffer *buffer);
t_paquete* crearPaquete(t_opCode codigo);
void agregarAPaquete(t_paquete *paquete, void *valor, int tamanio);
void enviarPaquete(t_paquete *paquete, int socket_cliente);
void eliminarPaquete(t_paquete *paquete);
void liberarConexion(int socket_cliente);
void* serializarPaquete(t_paquete *paquete, int bytes);

void enviarString(char *string, int socket_cliente, t_opCode codigoOperacion);

t_log* iniciarLogger(char *nombreArchivo, char *nombreProceso, bool consolaActiva, t_log_level nivelLog);
t_config* iniciarConfig(char *direccion_config);

t_parametrosInterfaz* inicializarParametros(char *interfaz, char *buffer, int tiempoSleep, int *direccionesFisicas,
		int cantidadDireccionesFisicas, int limiteEntrada);

//SERVIDOR

void* recibirBuffer(int *size, int socket_cliente);
int iniciarServidor(char *puerto, t_log *logger, char *msj_server);
t_opCode recibirOperacion(int socket_cliente);
char* recibirMensaje(int socket_cliente, t_log *logger);
t_list* recibirPaquete(int socket_cliente);

// Funciones de serializacion
void serializarContexto(t_paquete *paquete, t_contextoEjecucion *ctx);
void serializarParametros(t_paquete *paquete, t_parametrosInterfaz *interfaz);
t_parametrosFS* inicializarParametrosFS(char *nombreArchivo, int puntero, t_parametrosInterfaz *parametrosInterfaz);
void serializarParametrosFS(t_paquete *paquete, t_parametrosFS *parametrosFS);
void serializarMensajeYDireccion(t_paquete *paquete, t_parametrosInterfaz *parametros, char *mensaje);
void serializarParametrosYPid(t_paquete *paquete, t_parametrosInterfaz *interfaz, int pid);
void enviarParametrosInterfacesYPid(t_parametrosInterfaz *parametros, int socket, t_opCode codigo, int pid);

// Funciones de Deserializacion

t_registrosGenerales* deserializarRegistrosGenerales(t_buffer *buffer, int *desplazamiento);
t_parametrosInterfaz* deserializarParametrosInterfaces(t_paquete *paquete, int *desplazamiento);
t_parametrosFS* deserializarParametrosFS(t_paquete *paquete, int desplazamiento);
t_contextoEjecucion* deserializarContexto(t_buffer *buffer, int *desplazamiento);

// Nuevas

void enviarInstruccion(char *mensaje, int socket_cliente);
uint32_t* deserializarPC(t_buffer *buffer);
void serializarParametros(t_paquete *paquete, t_parametrosInterfaz *interfaz);
int recibirPID(int socketCliente);

// Funciones de contexto de ejecucion

void enviarContexto(t_contextoEjecucion *ctx, int socket, t_opCode cod_op);
void enviarParametrosInterfaces(t_parametrosInterfaz *parametros, int socket, t_opCode cod_op);
t_contextoEjecucion* recibirContexto(int socket);
void enviarOperacion(t_opCode operacion, int socket);
void enviarPID(int PID, int socket);
// Funciones de liberacion

void liberarUnaInstruccion(t_instruccion *instruccion);

void liberarContexto(t_contextoEjecucion *contexto_ejecucion);
void liberarParametrosInterfaz(t_parametrosInterfaz *parametrosInterfaz);
void liberarParametrosFS(t_parametrosFS *parametrosFS);

// Funciones para la comunicacion memoria (de CPU y E/S)
void solicitarLectura(int *direccionesFisicas, int sizeDireccionesFisicas, int sizeLectura, int PID, int socket);
void* recibirLectura(int socket);
void solicitarEscritura(int *direccionesFisicas, int sizeDireccionesFisicas, void *valor, int sizeValor, int PID, int socket);

//funciones para hacer logs
char* obtenerStringMotivoFinProceso(t_opCode codOp);
char* obtenerStringEstado(t_estado estado);

char* obtenerIPPorConsola(char *ipBuscada);
#endif /* HELLO_H_ */
