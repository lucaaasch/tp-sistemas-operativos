#include <utils/include/comunicacion.h>
#include <utils/include/estructuras.h>

// Funciones para loggers

#include <ctype.h>

char* toUpperCase(char *str) {
	int i = 0;
	while (str[i]) {
		str[i] = toupper(str[i]);
		i++;
	}
	return str;
}

/*void inicializarLoggers(char *modulo, t_log **logger, t_log **loggerAuxiliar) {

 *logger = iniciarLogger(strcat(modulo, ".log"), toUpperCase(modulo), 0, LOG_LEVEL_INFO);
 *loggerAuxiliar = iniciarLogger(strcat(modulo, "Aux.log"), strcat(toUpperCase(modulo), "_AUX"), 1, LOG_LEVEL_TRACE);
 }*/

//FUNCIONES CLIENTE
void* serializarPaquete(t_paquete *paquete, int bytes) {
	void *serializado = malloc(bytes);
	int desplazamiento = 0;

	memcpy(serializado + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(serializado + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(serializado + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return serializado;
}

int crearConexion(char *ip, char *puerto, t_log *log) {
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int addrinfoRet = getaddrinfo(ip, puerto, &hints, &server_info);
	if (addrinfoRet != 0) {
		log_error(log, "Error en la funcion getaddrinfo");
	}

	int socketCliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	if (socketCliente == -1) {
		log_error(log, "Error en la funcion socket");
	}

	int connectRet = connect(socketCliente, server_info->ai_addr, server_info->ai_addrlen);
	if (connectRet == -1) {
		log_error(log, "Error en la funcion connect");
	}

	freeaddrinfo(server_info);

	return socketCliente;
}

void enviarMensaje(char *mensaje, int socket_cliente) {
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializarPaquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminarPaquete(paquete);
}

void enviarString(char *string, int socket_cliente, t_opCode codigoOperacion) {
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = codigoOperacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(string) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, string, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializarPaquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminarPaquete(paquete);
}

t_buffer* crearBuffer() {
	t_buffer *buffer = malloc(sizeof(t_buffer));

	assert(buffer != NULL);

	buffer->size = 0;

	buffer->stream = NULL;

	return buffer;
}
t_paquete* crearPaqueteParametros() {
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->buffer = crearBuffer();
	return paquete;
}

t_paquete* crearPaquete(t_opCode codigo) {
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	paquete->buffer = crearBuffer();
	return paquete;
}

void agregarAPaquete(t_paquete *paquete, void *valor, int bytes) {
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + bytes);
	memcpy(paquete->buffer->stream + paquete->buffer->size, valor, bytes);
	paquete->buffer->size += bytes;
}

void enviarPaquete(t_paquete *paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializarPaquete(paquete, bytes);

	int ret = send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminarPaquete(t_paquete *paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberarConexion(int socket_cliente) {
	close(socket_cliente);
}

t_log* iniciarLogger(char *nombreArchivo, char *nombreProceso,
bool consolaActiva, t_log_level nivelLog) {
	t_log *nuevo_logger;
	nuevo_logger = log_create(nombreArchivo, nombreProceso, consolaActiva, nivelLog);
	if (nuevo_logger == NULL) {
		printf("El logger de %s no se creo correctamente\n", nombreProceso);
		exit(1);
	}

	return nuevo_logger;
}

t_config* iniciarConfig(char *direccion_config) {
	t_config *nuevo_config;
	nuevo_config = config_create(direccion_config);
	if (nuevo_config == NULL) {
		printf("no se creo la config");
		exit(2);
	}

	return nuevo_config;
}

//FUNCIONES SERVIDOR

void* recibirBuffer(int *size, int socket_cliente) {
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

t_parametrosInterfaz* inicializarParametros(char *interfaz, char *buffer, int tiempoSleep, int *direccionesFisicas,
		int cantidadDireccionesFisicas, int limiteEntrada) {
	t_parametrosInterfaz *parametros = malloc(sizeof(t_parametrosInterfaz));
	parametros->direccionesFisicas = malloc(sizeof(t_direccionesFisicas));

	if (direccionesFisicas == NULL) {
		int aux = 0;
		direccionesFisicas = &aux;
	}
	parametros->interfaz = interfaz;
	parametros->largoInterfaz = strlen(interfaz);
	parametros->buffer = buffer;
	parametros->largoTexto = strlen(buffer);
	parametros->tiempoSleep = tiempoSleep;
	parametros->direccionesFisicas->cantidadDirecciones = cantidadDireccionesFisicas;
	parametros->direccionesFisicas->listaDirecciones = direccionesFisicas;
	parametros->limiteEntrada = limiteEntrada;

	return parametros;
}

#include <errno.h>

void obtenerErrorBind(int socket) {
	switch (errno) {
	case EACCES:
		fprintf(stderr, "bind error: Permission denied\n");
		break;
	case EADDRINUSE:
		fprintf(stderr, "bind error: Address already in use\n");
		break;
	case EADDRNOTAVAIL:
		fprintf(stderr, "bind error: Address not available\n");
		break;
	case EBADF:
		fprintf(stderr, "bind error: Invalid file descriptor\n");
		break;
	case EFAULT:
		fprintf(stderr, "bind error: Bad address\n");
		break;
	case EINVAL:
		fprintf(stderr, "bind error: Invalid argument\n");
		break;
	case ENOTSOCK:
		fprintf(stderr, "bind error: Not a socket\n");
		break;
	default:
		fprintf(stderr, "bind error: %s\n");
		break;
	}
	close(socket);
	exit(EXIT_FAILURE);
}

int iniciarServidor(char *puerto, t_log *logger, char *msj_server) {

	int socketServidor;
	bool huboError = false;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int yes = 1;

	int addrinfoRet = getaddrinfo(NULL, puerto, &hints, &servinfo);
	if (addrinfoRet != 0) {
		log_error(logger, "Error en la funcion getaddrinfo");
		huboError = true;
	}

	socketServidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if (socketServidor == -1) {
		log_error(logger, "Error en la funcion socket");
		huboError = true;
	}

	int bindRet = bind(socketServidor, servinfo->ai_addr, servinfo->ai_addrlen);
	if (bindRet == -1) {
		log_error(logger, "Error en la funcion bind");
		huboError = true;
		obtenerErrorBind(socketServidor);
	}

	int listenRet = listen(socketServidor, SOMAXCONN);
	if (listenRet == -1) {
		log_error(logger, "Error en la funcion listen");
		huboError = true;

	}

	freeaddrinfo(servinfo);

	if (huboError) {
		exit(EXIT_FAILURE);
	}

	log_trace(logger, "SERVER: %s ", msj_server);
	return socketServidor;
}

t_list* recibirPaquete(int socket_cliente) {
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibirBuffer(&size, socket_cliente);
	while (desplazamiento < size) {
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

void enviarOperacion(t_opCode codigo, int socket) {
	//send(socket_cliente, a_enviar, bytes, 0);
	send(socket, &codigo, sizeof(codigo), 0);
}

t_opCode recibirOperacion(int socketCliente) {
	t_opCode codOp;
	if (recv(socketCliente, &codOp, sizeof(t_opCode), MSG_WAITALL) != -1)
		return codOp;
	else {
		close(socketCliente);
		return -1;
	}
}

void liberarBuffer(t_buffer *buffer) {
	free(buffer->stream);
	free(buffer);
}

void enviarPID(int PID, int socket) {
	send(socket, &PID, sizeof(int), NULL);
}

void serializarContexto(t_paquete *paquete, t_contextoEjecucion *contexto) {
	agregarAPaquete(paquete, &(contexto->PID), sizeof(int));

	agregarAPaquete(paquete, &(contexto->PC), sizeof(uint32_t));

	// Agregamos los registros generales
	agregarAPaquete(paquete, &(contexto->registrosGenerales->AX), sizeof(uint8_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->BX), sizeof(uint8_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->CX), sizeof(uint8_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->DX), sizeof(uint8_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->EAX), sizeof(uint32_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->EBX), sizeof(uint32_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->ECX), sizeof(uint32_t));
	agregarAPaquete(paquete, &(contexto->registrosGenerales->EDX), sizeof(uint32_t));
	agregarAPaquete(paquete, &(contexto->registrosMemoria->SI), sizeof(uint32_t));
	agregarAPaquete(paquete, &(contexto->registrosMemoria->DI), sizeof(uint32_t));
}

void serializarParametros(t_paquete *paquete, t_parametrosInterfaz *interfaz) {
	int largoTexto = strlen(interfaz->buffer);
	int largoInterfaz = strlen(interfaz->interfaz);

	agregarAPaquete(paquete, &(interfaz->largoInterfaz), sizeof(int));
	agregarAPaquete(paquete, interfaz->interfaz, largoInterfaz * sizeof(char));

	agregarAPaquete(paquete, &(interfaz->largoTexto), sizeof(int));
	agregarAPaquete(paquete, interfaz->buffer, largoTexto * sizeof(char));

	int cantidadDirecciones = interfaz->direccionesFisicas->cantidadDirecciones;

	agregarAPaquete(paquete, &(interfaz->tiempoSleep), sizeof(int));

	agregarAPaquete(paquete, &(cantidadDirecciones), sizeof(int));

	agregarAPaquete(paquete, interfaz->direccionesFisicas->listaDirecciones, cantidadDirecciones * sizeof(int));

	agregarAPaquete(paquete, &(interfaz->limiteEntrada), sizeof(int));

}

t_parametrosFS* inicializarParametrosFS(char *nombreArchivo, int puntero, t_parametrosInterfaz *parametrosInterfaz) {
	t_parametrosFS *parametrosFS = malloc(sizeof(t_parametrosFS));

	parametrosFS->nombreArchivo = nombreArchivo;
	parametrosFS->largoNombreArchivo = strlen(nombreArchivo);
	parametrosFS->puntero = puntero;
	parametrosFS->parametrosInterfaz = parametrosInterfaz;

	return parametrosFS;
}

void serializarParametrosFS(t_paquete *paquete, t_parametrosFS *parametrosFS) {
	agregarAPaquete(paquete, &(parametrosFS->largoNombreArchivo), sizeof(int));
	agregarAPaquete(paquete, parametrosFS->nombreArchivo, sizeof(char) * parametrosFS->largoNombreArchivo);
	agregarAPaquete(paquete, &(parametrosFS->puntero), sizeof(int));
	serializarParametros(paquete, parametrosFS->parametrosInterfaz);
}

void liberarParametrosInterfaz(t_parametrosInterfaz *parametrosInterfaz) {
	if (parametrosInterfaz != NULL) {
		if (parametrosInterfaz->interfaz != NULL) {
			free(parametrosInterfaz->interfaz);
		}
		if (parametrosInterfaz->buffer != NULL) {
			free(parametrosInterfaz->buffer);
		}
		if (parametrosInterfaz->direccionesFisicas != NULL) {
			if (parametrosInterfaz->direccionesFisicas->listaDirecciones != NULL) {
				free(parametrosInterfaz->direccionesFisicas->listaDirecciones);
			}
			free(parametrosInterfaz->direccionesFisicas);
		}
		free(parametrosInterfaz);
	}
}

void liberarParametrosFS(t_parametrosFS *parametrosFS) {
	if (parametrosFS != NULL) {
		if (parametrosFS->nombreArchivo != NULL) {
			free(parametrosFS->nombreArchivo);
		}
		liberarParametrosInterfaz(parametrosFS->parametrosInterfaz);
		free(parametrosFS);
	}
}

// Funciones de DESERIALIZACION

t_registrosGenerales* deserializarRegistrosGenerales(t_buffer *buffer, int *desplazamiento) {
	t_registrosGenerales *registros = malloc(sizeof(t_registrosGenerales));

	memcpy(&(registros->AX), buffer->stream + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	memcpy(&(registros->BX), buffer->stream + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	memcpy(&(registros->CX), buffer->stream + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	memcpy(&(registros->DX), buffer->stream + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	memcpy(&(registros->EAX), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(registros->EBX), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(registros->ECX), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(registros->EDX), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	// ver que pasa con los registros de memoria
	return registros;
}

t_registrosMemoria* deserializarRegistrosMemoria(t_buffer *buffer, int *desplazamiento) {
	t_registrosMemoria *regMemoria = malloc(sizeof(t_registrosMemoria));

	memcpy(&(regMemoria->SI), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);

	memcpy(&(regMemoria->DI), buffer->stream + *desplazamiento, sizeof(uint32_t));
	*desplazamiento += sizeof(uint32_t);
	return regMemoria;
}

t_parametrosInterfaz* deserializarParametrosInterfaces(t_paquete *paquete, int *desplazamientoAux) {
	int *desplazamiento;
	if (desplazamientoAux == NULL) {
		*desplazamiento = 0;
	}
	desplazamiento = desplazamientoAux;

	t_parametrosInterfaz *parametros = malloc(sizeof(t_parametrosInterfaz));
	parametros->direccionesFisicas = malloc(sizeof(t_direccionesFisicas));
	t_buffer *buffer = paquete->buffer;
	//Largointerfaz
	memcpy(&(parametros->largoInterfaz), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametros->interfaz = malloc(parametros->largoInterfaz * sizeof(char) + 1);
	memcpy(parametros->interfaz, buffer->stream + *desplazamiento, parametros->largoInterfaz);
	parametros->interfaz[parametros->largoInterfaz] = '\0';
	*desplazamiento += parametros->largoInterfaz * sizeof(char);

	//txt (de ser necesario)
	memcpy(&(parametros->largoTexto), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	parametros->buffer = malloc(parametros->largoTexto * sizeof(char) + 1);
	memcpy(parametros->buffer, buffer->stream + *desplazamiento, parametros->largoTexto);
	parametros->buffer[parametros->largoTexto] = '\0';
	*desplazamiento += parametros->largoTexto * sizeof(char);

	//tiempoSleep
	memcpy(&(parametros->tiempoSleep), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	memcpy(&(parametros->direccionesFisicas->cantidadDirecciones), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	int cantidadDirecciones = parametros->direccionesFisicas->cantidadDirecciones;
	parametros->direccionesFisicas->listaDirecciones = malloc(cantidadDirecciones * sizeof(int));
	memcpy((parametros->direccionesFisicas->listaDirecciones), buffer->stream + *desplazamiento, cantidadDirecciones * sizeof(int));
	*desplazamiento += cantidadDirecciones * sizeof(int);

	memcpy(&(parametros->limiteEntrada), buffer->stream + *desplazamiento, sizeof(int));
	*desplazamiento += sizeof(int);

	return parametros;
}

t_parametrosFS* deserializarParametrosFS(t_paquete *paquete, int desplazamiento) {
	t_parametrosFS *parametrosFS = malloc(sizeof(t_parametrosFS));
	t_buffer *buffer = paquete->buffer;

	memcpy(&(parametrosFS->largoNombreArchivo), buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	parametrosFS->nombreArchivo = malloc(parametrosFS->largoNombreArchivo + 1);
	memcpy(parametrosFS->nombreArchivo, buffer->stream + desplazamiento, parametrosFS->largoNombreArchivo * sizeof(char));
	parametrosFS->nombreArchivo[parametrosFS->largoNombreArchivo] = '\0';
	desplazamiento += parametrosFS->largoNombreArchivo * sizeof(char);

	memcpy(&(parametrosFS->puntero), buffer->stream + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	parametrosFS->parametrosInterfaz = deserializarParametrosInterfaces(paquete, &desplazamiento);

	return parametrosFS;
}

void enviarInstruccion(char *mensaje, int socket_cliente) {
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = INTERCAMBIO_INSTRUCCION;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializarPaquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminarPaquete(paquete);
}

t_contextoEjecucion* deserializarContexto(t_buffer *buffer, int *desplazamientoAux) {
	int desplazamientoLocal = 0;

	int *desplazamiento = desplazamientoAux != NULL ? desplazamientoAux : &desplazamientoLocal;

	//Asigno memoria dinamica para el contexto
	t_contextoEjecucion *contexto = malloc(sizeof(t_contextoEjecucion)); //Se libera en liberar_contexto(*contexto)

	//Copio en el contexto los campos y actualizo el desplazamiento
	memcpy(&(contexto->PID), buffer->stream + *desplazamiento, sizeof(contexto->PID));
	*desplazamiento += sizeof(contexto->PID);

	memcpy(&(contexto->PC), buffer->stream + *desplazamiento, sizeof(contexto->PC));
	*desplazamiento += sizeof(contexto->PC);

	//Deserializo registros
	contexto->registrosGenerales = deserializarRegistrosGenerales(buffer, desplazamiento);
	contexto->registrosMemoria = deserializarRegistrosMemoria(buffer, desplazamiento);

	return contexto;
}

char* recibirMensaje(int socket_cliente, t_log *logger) {
	int size;
	char *buffer = recibirBuffer(&size, socket_cliente);
	log_info(logger, "Interfaz recibida: %s", buffer);
	return buffer;
}

int recibirPID(int socketCliente) {
	int PID;
	recv(socketCliente, &PID, sizeof(int), MSG_WAITALL);
	return PID;
}

uint32_t recibirPC(int socketCliente) {
	uint32_t PC;
	recv(socketCliente, &PC, sizeof(int), MSG_WAITALL);
	return PC;
}

// Funciones de contexto de ejecucion

void enviarContexto(t_contextoEjecucion *contexto, int socket, t_opCode cod_op) {
	t_paquete *paquete = crearPaquete(cod_op);
	serializarContexto(paquete, contexto);
	enviarPaquete(paquete, socket);
	eliminarPaquete(paquete);
}

void enviarParametrosInterfaces(t_parametrosInterfaz *parametros, int socket, t_opCode codigo) {
	t_paquete *paquete = crearPaquete(codigo);
	serializarParametros(paquete, parametros);
	enviarPaquete(paquete, socket);
	eliminarPaquete(paquete);
}

t_contextoEjecucion* recibirContexto(int socket) {
	t_buffer *buffer = crearBuffer();

	buffer->stream = recibirBuffer(&(buffer->size), socket);

	t_contextoEjecucion *contexto = deserializarContexto(buffer, NULL);

	liberarBuffer(buffer);

	return contexto;
}

//LIBERACION//

void liberarContexto(t_contextoEjecucion *contexto) {
	if (contexto != NULL) {
		if (contexto->registrosGenerales != NULL) {
			free(contexto->registrosGenerales);
		}
		if (contexto->registrosMemoria != NULL) {
			free(contexto->registrosMemoria);
		}
		free(contexto);
	}
}

// Funciones para la comunicacion memoria (de CPU y E/S)

void solicitarLectura(int *direccionesFisicas, int sizeDireccionesFisicas, int sizeLectura, int PID, int socket) {
	t_paquete *paqueteLectura = crearPaquete(SOLICITUD_LECTURA);

	agregarAPaquete(paqueteLectura, &PID, sizeof(int));
	agregarAPaquete(paqueteLectura, &sizeDireccionesFisicas, sizeof(int));
	agregarAPaquete(paqueteLectura, direccionesFisicas, sizeDireccionesFisicas * sizeof(int));
	agregarAPaquete(paqueteLectura, &sizeLectura, sizeof(int));

	enviarPaquete(paqueteLectura, socket);

	eliminarPaquete(paqueteLectura);
}

void* recibirLectura(int socket) {
	recibirOperacion(socket);
	int sizeBuffer = 0;
	void *buffer = recibirBuffer(&sizeBuffer, socket);
	buffer = realloc(buffer, sizeBuffer + 1);
	((char*) buffer)[sizeBuffer] = '\0';
	return buffer;
}

void solicitarEscritura(int *direccionesFisicas, int sizeDireccionesFisicas, void *valor, int sizeValor, int PID, int socket) {
	t_paquete *paqueteLectura = crearPaquete(SOLICITUD_ESCRITURA);

	agregarAPaquete(paqueteLectura, &PID, sizeof(int));
	agregarAPaquete(paqueteLectura, &sizeDireccionesFisicas, sizeof(int));
	agregarAPaquete(paqueteLectura, direccionesFisicas, sizeDireccionesFisicas * sizeof(int));
	agregarAPaquete(paqueteLectura, &sizeValor, sizeof(int));
	agregarAPaquete(paqueteLectura, valor, sizeValor);

	enviarPaquete(paqueteLectura, socket);

	eliminarPaquete(paqueteLectura);
}

char* obtenerStringMotivoFinProceso(t_opCode codOp) { //funcion para hacer los logs
	char *string;
	switch (codOp) {
	case SYSCALL_EXIT:
		string = "SUCCESS";
		break;
	case ERROR_OUT_OF_MEMORY:
		string = "OUT_OF_MEMORY";
		break;
	case INTERRUPTED_BY_USER:
		string = "INTERRUPTED_BY_USER";
		break;
	case INVALID_RESOURCE:
		string = "INVALID_RESOURCE";
		break;
	case INVALID_INTERFACE:
		string = "INVALID_INTERFACE";
		break;
	default:
		break;
	}
	return string;

}

char* obtenerStringEstado(t_estado estado) { //funcion para hacer los logs
	char *string;
	switch (estado) {
	case NEW:
		string = "NEW";
		break;
	case READY:
		string = "READY";
		break;
	case EXEC:
		string = "EXEC";
		break;
	case BLOCKED:
		string = "BLOCKED";
		break;
	case EXIT:
		string = "EXIT";
		break;
	}
	return string;
}

char* obtenerIPPorConsola(char *ipBuscada) {
	char *prompt = malloc(strlen(ipBuscada) + 15);
	sprintf(prompt, "Ingrese %s:\n> ", ipBuscada);
	char *entrada = readline(prompt);
	free(prompt);
	return entrada;
}
