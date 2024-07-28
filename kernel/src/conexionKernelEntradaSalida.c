#include <commons/collections/list.h>
#include <commons/log.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/socket.h>
#include <utils/include/comunicacion.h>
#include <utils/include/estructuras.h>

#include "../include/kernel.h"

typedef struct {
	int socketCliente;
	char *serverName;
} t_argumentosConexion;

void iterator(char *value) {
	log_info(auxLogger, "%s", value);
}

void procesarConexion(void *argumentos) {
	t_argumentosConexion *args = (t_argumentosConexion*) argumentos;

	int socketCliente = args->socketCliente;
	char *serverName = args->serverName;
	free(args);
	t_opCode cop;
	t_list *lista;
	bool seguirEjecutando = 1;
	// TODO Esto deberia ser una funcion que solo recibe el nombre de entrada salida (ver con FS).
	while (seguirEjecutando) {
		if (recv(socketCliente, &cop, sizeof(t_opCode), 0) == -1) {
			log_error(auxLogger, "El cliente se desconecto de: %s ", serverName);
			return;
		} else {
			switch (cop) {
			case NOMBRE_ENTRADA_SALIDA:
				char *nombreInterfaz = recibirMensaje(socketCliente, auxLogger);
				//TODO liberar Buffer
				t_propiedadesEntradaSalida *idEntradaSalida = malloc(sizeof(t_propiedadesEntradaSalida));
				idEntradaSalida->nombre = malloc(strlen(nombreInterfaz)+1);
				strcpy (idEntradaSalida->nombre ,nombreInterfaz);
				idEntradaSalida->socketConexion = socketCliente;
				idEntradaSalida->colaBlocked = queue_create();
				idEntradaSalida->enUso = false;
				list_add(interfacesActivas, (void*) idEntradaSalida);
				seguirEjecutando = 0;
				break;

			case PAQUETE:
				lista = recibirPaquete(socketCliente);
				log_info(auxLogger, "Me llegaron los siguientes valores: ");
				list_iterate(lista, (void*) iterator);
				break;

			default:
				log_warning(auxLogger, "Operacion desconocida. No quieras meter la pata de: %s", serverName);
				return;

			}
		}
		usleep(10);
	}

	return;
}

void serverEscuchar() {
	bool huboErrores = 0;
	while (!huboErrores) {
		socketEntradaSalida = accept(socketServerKernel, NULL, NULL);
		if (socketEntradaSalida != -1) {
			pthread_t hilo;
			t_argumentosConexion *args = malloc(sizeof(t_argumentosConexion));
			args->socketCliente = socketEntradaSalida;
			pthread_create(&hilo, NULL, (void*) procesarConexion, (void*) args);
			pthread_detach(hilo);
			huboErrores = 0;
		} else {
			huboErrores = 1;
			log_error(auxLogger, "Hubo error al aceptar conexiones.");
		}
	}

}

void levantarServidor() {
	socketServerKernel = iniciarServidor(cfgKernel->PUERTO_ESCUCHA, auxLogger, "KERNEL");
	if (socketServerKernel == -1) {
		log_error(auxLogger, "No se pudo iniciar el servidor de KERNEL");
		abort();
	} else {
		log_info(auxLogger, "Servidor de Kernel levantado.");
		serverEscuchar();
	}

}

