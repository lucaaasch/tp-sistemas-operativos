#include <stdlib.h>
#include <stdio.h>
#include <utils/include/comunicacion.h>
#include "../include/conexionEntradaSalida.h"
#include "../include/entradasalida.h"

bool generarConexiones(int *socketConexionMemoria, int *socketConexionKernel) {
	*socketConexionKernel = crearConexion(cfgEntradaSalida->IP_KERNEL, cfgEntradaSalida->PUERTO_KERNEL, auxLogger);
	if (strcmp(cfgEntradaSalida->TIPO_INTERFAZ, "GENERICA") != 0) {
		*socketConexionMemoria = crearConexion(cfgEntradaSalida->IP_MEMORIA, cfgEntradaSalida->PUERTO_MEMORIA, auxLogger);
	}
	return *socketConexionMemoria != -1 && *socketConexionKernel != -1;
}

void enviarEntrada(t_parametrosInterfaz *parametros, int PID, t_opCode operacion) {
	t_paquete *paqueteEntrada = crearPaquete(operacion);

	int cantidadDirecciones = parametros->direccionesFisicas->cantidadDirecciones;
	agregarAPaquete(paqueteEntrada, &PID, sizeof(int));
	agregarAPaquete(paqueteEntrada, &cantidadDirecciones, sizeof(int));
	agregarAPaquete(paqueteEntrada, parametros->direccionesFisicas->listaDirecciones, cantidadDirecciones * sizeof(int));
	agregarAPaquete(paqueteEntrada, &(parametros->limiteEntrada), sizeof(int));

	enviarPaquete(paqueteEntrada, socketConexionMemoria);
	eliminarPaquete(paqueteEntrada);
}

void enviarEntradaConBuffer(t_parametrosInterfaz *parametros, char *buffer, int PID, t_opCode operacion) {
	t_paquete *paqueteEntrada = crearPaquete(operacion);

	int cantidadDirecciones = parametros->direccionesFisicas->cantidadDirecciones;
	agregarAPaquete(paqueteEntrada, &PID, sizeof(int));
	agregarAPaquete(paqueteEntrada, &cantidadDirecciones, sizeof(int));
	agregarAPaquete(paqueteEntrada, parametros->direccionesFisicas->listaDirecciones, cantidadDirecciones * sizeof(int));
	agregarAPaquete(paqueteEntrada, &(parametros->limiteEntrada), sizeof(int));
	agregarAPaquete(paqueteEntrada, buffer, parametros->limiteEntrada);

	enviarPaquete(paqueteEntrada, socketConexionMemoria);
	eliminarPaquete(paqueteEntrada);
}

void enviarPIDAKernel(int PID, t_opCode operacion) {
	t_paquete *paqueteKernel = crearPaquete(operacion);
	agregarAPaquete(paqueteKernel, &PID, sizeof(int));
	enviarPaquete(paqueteKernel, socketConexionKernel);
	eliminarPaquete(paqueteKernel);
}

void fsWrite(t_parametrosFS *parametros, int PID) {

	int *direccionesFisicas = parametros->parametrosInterfaz->direccionesFisicas->listaDirecciones;
	int cantidadDireccionesFisicas = parametros->parametrosInterfaz->direccionesFisicas->cantidadDirecciones;
	int bytesLectura = parametros->parametrosInterfaz->limiteEntrada;

	solicitarLectura(direccionesFisicas, cantidadDireccionesFisicas, bytesLectura, PID, socketConexionMemoria);

	void *lectura = recibirLectura(socketConexionMemoria);
	escribirEnFileSystem(parametros->nombreArchivo, bytesLectura, lectura, parametros->puntero);
	free(lectura);
}

void fsRead(t_parametrosFS *parametros, int PID) {

	int *direccionesFisicas = parametros->parametrosInterfaz->direccionesFisicas->listaDirecciones;
	int cantidadDireccionesFisicas = parametros->parametrosInterfaz->direccionesFisicas->cantidadDirecciones;
	int bytesLectura = parametros->parametrosInterfaz->limiteEntrada;

	void *lectura = malloc(bytesLectura);

	leerDeFileSystem(parametros->nombreArchivo, bytesLectura, lectura, parametros->puntero);

	solicitarEscritura(direccionesFisicas, cantidadDireccionesFisicas, lectura, bytesLectura, PID, socketConexionMemoria);

	free(lectura);
	// Para que se bloquee hasta que la lectura este hecha.
	recibirOperacion(socketConexionMemoria);

}

bool esOperacionFS(t_opCode operacion) {
	return operacion == SOLICITUD_IO_FS_CREATE || operacion == SOLICITUD_IO_FS_DELETE || operacion == SOLICITUD_IO_FS_TRUNCATE
			|| operacion == SOLICITUD_IO_FS_WRITE || operacion == SOLICITUD_IO_FS_READ;
}

bool tipoInterfazConcuerda(t_opCode tipoInterfaz) {
	return (tipoInterfaz == SYSCALL_IO_GEN_SLEEP && !strcmp(cfgEntradaSalida->TIPO_INTERFAZ, "GENERICA"))
			|| (tipoInterfaz == SYSCALL_IO_STDIN_READ && !strcmp(cfgEntradaSalida->TIPO_INTERFAZ, "STDIN"))
			|| (tipoInterfaz == SYSCALL_IO_STDOUT_WRITE && !strcmp(cfgEntradaSalida->TIPO_INTERFAZ, "STDOUT"))
			|| (esOperacionFS(tipoInterfaz) && !strcmp(cfgEntradaSalida->TIPO_INTERFAZ, "DIALFS"));
}

char* consultarNombre() {
	char *nombre = readline("Ingrese Nombre interfaz:\n> ");
	return nombre;
}

void consumirUnidadTiempo() {
	useconds_t unidadTiempo = 1000 * (cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO);
	usleep(unidadTiempo);
}

void esperarEntradaSalida() {

	while (1) {

		t_opCode codigo = recibirOperacion(socketConexionKernel);

		t_paquete *paqueteParametrosInterfaz = crearPaquete(codigo);

		paqueteParametrosInterfaz->buffer->stream = recibirBuffer(&(paqueteParametrosInterfaz->buffer->size), socketConexionKernel);

		int PID = 0;

		memcpy(&PID, paqueteParametrosInterfaz->buffer->stream, sizeof(int));

		t_parametrosFS *parametrosFS;
		t_parametrosInterfaz *parametrosInterfaz;

		if (esOperacionFS(codigo)) {
			parametrosFS = deserializarParametrosFS(paqueteParametrosInterfaz, sizeof(int));

		} else {
			int desplazamiento = sizeof(int);
			parametrosInterfaz = deserializarParametrosInterfaces(paqueteParametrosInterfaz, &desplazamiento);
		}

		eliminarPaquete(paqueteParametrosInterfaz);

		if (tipoInterfazConcuerda(codigo)) {

			switch (codigo) {
			case SYSCALL_IO_GEN_SLEEP:
				log_info(auxLogger, "PID: %i - OPERACION: IO_GEN_SLEEP", PID);
				dormirInterfaz(parametrosInterfaz, PID);
				break;
			case SYSCALL_IO_STDIN_READ:
				log_info(auxLogger, "PID: %i - OPERACION: IO_STDIN_READ", PID);
				esperarEntradaPorTeclado(parametrosInterfaz, PID);
				break;
			case SYSCALL_IO_STDOUT_WRITE:
				log_info(auxLogger, "PID: %i - OPERACION: IO_STDOUT_WRITE", PID);
				mostrarEnConsola(parametrosInterfaz, PID);
				break;
			case SOLICITUD_IO_FS_CREATE:
				log_info(auxLogger, "PID: %i - Crear Archivo: %s",PID,parametrosFS->nombreArchivo);
				crearArchivo(parametrosFS->nombreArchivo);
				consumirUnidadTiempo();
				enviarPID(PID, socketConexionKernel);
				break;
			case SOLICITUD_IO_FS_DELETE:
				log_info(auxLogger, "PID: %i - Eliminar Archivo: %s",PID,parametrosFS->nombreArchivo);
				eliminarArchivo(parametrosFS->nombreArchivo);
				consumirUnidadTiempo();
				enviarPID(PID, socketConexionKernel);
				break;
			case SOLICITUD_IO_FS_TRUNCATE:
				log_info(auxLogger, "PID: %i - Truncar Archivo: %s - Tamaño: %i",PID,parametrosFS->nombreArchivo,parametrosFS->parametrosInterfaz->limiteEntrada);
				truncarArchivo(parametrosFS->nombreArchivo, parametrosFS->parametrosInterfaz->limiteEntrada,PID);
				consumirUnidadTiempo();
				enviarPID(PID, socketConexionKernel);
				break;
			case SOLICITUD_IO_FS_WRITE:
				log_info(auxLogger, "PID: %i - Escribir Archivo: %s - Tamaño a Escribir: %i - Puntero Archivo: %i",PID,parametrosFS->nombreArchivo,parametrosFS->parametrosInterfaz->limiteEntrada,parametrosFS->puntero);
				fsWrite(parametrosFS, PID);
				consumirUnidadTiempo();
				enviarPID(PID, socketConexionKernel);
				break;
			case SOLICITUD_IO_FS_READ:
				log_info(auxLogger, "PID: %i - Leer Archivo: %s - Tamaño a Leer: %i - Puntero Archivo: %i",PID,parametrosFS->nombreArchivo,parametrosFS->parametrosInterfaz->limiteEntrada,parametrosFS->puntero);
				fsRead(parametrosFS, PID);
				consumirUnidadTiempo();
				enviarPID(PID, socketConexionKernel);
				break;
			default:
				log_error(auxLogger, "El tipo de interfaz que buscas no existe.");
				break;
			}
			if(esOperacionFS(codigo)){
				liberarParametrosFS(parametrosFS);
			}
			else{
				liberarParametrosInterfaz(parametrosInterfaz);
			}
		} else {
			log_info(auxLogger, "ERROR La interfaz no admite la operacion solicitada");
		}
	}
}

