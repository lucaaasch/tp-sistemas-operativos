#include "../include/memoria.h"

typedef struct {
	int socketCliente;
	char *serverName;
} t_argumentosConexion;

void recibirSolicitudInstruccion(int *PID, uint32_t *PC, int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;

	memcpy(PID, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(PC, buffer->stream + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	liberarBuffer(buffer);
}

void recibirDosInt(int *a, int *b, int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;

	memcpy(a, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(b, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	liberarBuffer(buffer);
}

void recibirInt(int *a, int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;

	memcpy(a, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	liberarBuffer(buffer);
}

void enviarInt(int a, t_opCode codigo, int socketCliente) {
	t_paquete *paquete = crearPaquete(codigo);
	agregarAPaquete(paquete, &a, sizeof(int));
	enviarPaquete(paquete, socketCliente);
	eliminarPaquete(paquete);
}

void recibirCreacionProceso(int *PID, char **path, int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;
	int pathSize;

	memcpy(PID, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&pathSize, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	*path = malloc(pathSize + 1);

	memcpy(*path, buffer->stream + desplazamiento, sizeof(char) * pathSize);
	(*path)[pathSize] = '\0';
	desplazamiento += sizeof(char) * pathSize;

	liberarBuffer(buffer);
}

void iterator(char *value) {
	log_info(auxLogger, "%s", value);
}

void enviarTamPagina(int socketCliente) {
	t_paquete *paqueteTamPagina = crearPaquete(SOLICITUD_TAM_PAGINA);
	agregarAPaquete(paqueteTamPagina, &(cfgMemoria->TAM_PAGINA), sizeof(uint16_t));
	enviarPaquete(paqueteTamPagina, socketCliente);
	eliminarPaquete(paqueteTamPagina);
}

void recibirSolicitudEscritura(int *PID, int *cantidadDirecciones, int **listaDirecciones, int *tamanioEscritura, void **bufferEscritura,
		int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;

	memcpy(PID, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(cantidadDirecciones, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	*listaDirecciones = malloc(sizeof(int) * (*cantidadDirecciones));
	memcpy(*listaDirecciones, buffer->stream + desplazamiento, sizeof(int) * (*cantidadDirecciones));
	desplazamiento += (*cantidadDirecciones) * sizeof(int);

	memcpy(tamanioEscritura, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	*bufferEscritura = malloc(*tamanioEscritura + 1);
	((char*) *bufferEscritura)[*tamanioEscritura] = '\0';
	memcpy(*bufferEscritura, buffer->stream + desplazamiento, *tamanioEscritura);
	desplazamiento += sizeof(int);

	liberarBuffer(buffer);
}

void recibirSolicitudLectura(int *PID, int *cantidadDirecciones, int **listaDirecciones, int *tamanioEscritura, int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	int desplazamiento = 0;

	memcpy(PID, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(cantidadDirecciones, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	*listaDirecciones = malloc(sizeof(int) * (*cantidadDirecciones));
	memcpy(*listaDirecciones, buffer->stream + desplazamiento, sizeof(int) * (*cantidadDirecciones));
	desplazamiento += (*cantidadDirecciones) * sizeof(int);

	memcpy(tamanioEscritura, buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	liberarBuffer(buffer);
}

void enviarLecturaExitosa(int tamanioLectura, void *buffer, int socketCliente) {
	t_paquete *paquete = crearPaquete(ACCESO_OK);
	agregarAPaquete(paquete, buffer, tamanioLectura);
	enviarPaquete(paquete, socketCliente);
	eliminarPaquete(paquete);
}

void procesarConexion(void *argumentos) {
	t_argumentosConexion *args = (t_argumentosConexion*) argumentos;
	int socketCliente = args->socketCliente;
	char *serverName = args->serverName;
	free(args);
	t_opCode cop;
	while (socketCliente != -1) {
		if (recv(socketCliente, &cop, sizeof(t_opCode), 0) != sizeof(t_opCode)) {
			log_info(auxLogger, "DISCONNECT!");
			return;
		}
		int PID = 0;
		uint32_t PC = 0;
		char *path;
		int size = 0;
		int numeroPagina = 0;
		int *listaDirecciones;
		int cantidadDirecciones;
		int tamanioAcceso;
		void *buffer;
		switch (cop) {
		case SOLICITUD_INSTRUCCION:
			recibirSolicitudInstruccion(&PID, &PC, socketCliente);
			enviarInstruccionACPU(PID, PC, socketCliente);
			break;
		case SOLICITUD_NUMERO_MARCO:
			recibirDosInt(&PID, &numeroPagina, socketCliente);
			int numeroMarco = obtenerNumeroMarcoAccediendoATablaPaginas(PID, numeroPagina);
			enviarInt(numeroMarco, SOLICITUD_NUMERO_MARCO, socketCliente);
			break;
		case CREAR_PROCESO:
			recibirCreacionProceso(&PID, &path, socketCliente);
			cargarInstrucciones(path, PID);
			crearTablaPaginas(PID);
			enviarOperacion(PROCESO_OK, socketCliente);
			break;
		case FIN_PROCESO:
			recibirInt(&PID, socketCliente);
			liberarBloqueMemoriaInstruccion(PID);
			liberarEspacioUsuario(PID);
			break;
		case SOLICITUD_RESIZE:
			recibirDosInt(&PID, &size, socketCliente);
			if (redimensionar(PID, size)) {
				enviarOperacion(SOLICITUD_RESIZE, socketCliente);
			} else {
				enviarOperacion(ERROR_OUT_OF_MEMORY, socketCliente);
			}
			break;
		case SOLICITUD_ESCRITURA:
			recibirSolicitudEscritura(&PID, &cantidadDirecciones, &listaDirecciones, &tamanioAcceso, &buffer, socketCliente);
			log_info(auxLogger, "PID: %d - Accion: Escritura - Direccion fisica: %d - Tamaño: %d", PID, listaDirecciones[0], tamanioAcceso);
			if (escribirEnMemoria(listaDirecciones, cantidadDirecciones, tamanioAcceso, buffer)) {
				enviarOperacion(ACCESO_OK, socketCliente);
			} else {
				enviarOperacion(ACCESO_ERROR, socketCliente);
			}
			free(listaDirecciones);
			free(buffer);
			break;
		case SOLICITUD_LECTURA:
			recibirSolicitudLectura(&PID, &cantidadDirecciones, &listaDirecciones, &tamanioAcceso, socketCliente);
			log_info(auxLogger, "PID: %d - Accion: Lectura - Direccion fisica: %d - Tamaño: %d", PID, listaDirecciones[0], tamanioAcceso);
			buffer = malloc(tamanioAcceso + 1);
			((char*) buffer)[tamanioAcceso] = '\0';
			if (leerDeMemoria(listaDirecciones, cantidadDirecciones, tamanioAcceso, buffer)) {
				enviarLecturaExitosa(tamanioAcceso, buffer, socketCliente);
			} else {
				enviarOperacion(ACCESO_ERROR, socketCliente);
			}
			free(listaDirecciones);
			free(buffer);
			break;
		case SOLICITUD_TAM_PAGINA:
			enviarTamPagina(socketCliente);
			break;
		default:
			log_warning(auxLogger, "Operacion desconocida. No quieras meter la pata de: %s", serverName);
			return;
		}
	}

	free(serverName);
	log_warning(auxLogger, "El cliente se desconecto de %s server", serverName);
	return;
}

void iniciarServidorMemoria() {
	socketServidorMemoria = iniciarServidor(cfgMemoria->PUERTO_ESCUCHA, auxLogger, "MEMORIA");

	if (socketServidorMemoria == -1) {
		log_error(auxLogger, "No se pudo iniciar el server de DISPATCH");
		abort();
	}
}

bool serverEscuchar(int socketServer) {
	bool huboErrores = 0;
	while (!huboErrores) {
		int socketCliente = accept(socketServer, NULL, NULL);
		log_info(auxLogger, "CLIENTE ACEPTADO");
		if (socketCliente != -1) {
			pthread_t hilo;
			t_argumentosConexion *args = malloc(sizeof(t_argumentosConexion));
			args->socketCliente = socketCliente;
			pthread_create(&hilo, NULL, (void*) procesarConexion, (void*) args);
			pthread_detach(hilo);
			huboErrores = 0;
		} else {
			huboErrores = 1;
		}
	}
	return huboErrores;
}
