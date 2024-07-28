#include "../include/cpu.h"

// Imita el IF (Interrupt Flag) del PSW.
bool hayInterrupciones;

// Semaforos
sem_t atenderInterrupciones;
sem_t mutexContexto;

t_contextoEjecucion *contexto;

void checkInterrupt(pthread_t interrupciones) {
	// Despues de ejecutar una instruccion, verificar si hay interrupciones
	if (hayInterrupciones) {
		// Si hay interrupciones, las atendemos
		sem_post(&atenderInterrupciones);
		// Esperamos que el hilo termine antes de seguir.
		pthread_join(interrupciones, NULL);
	} else {
		// Si no hay interrupciones, el hilo de interrupciones no debe ejecutarse
		pthread_cancel(interrupciones);
	}
}

void liberarInstruccion(const t_instruccion* instruccion){
	for(int i=0; i<instruccion->cantidadParametros;i++){
		free(instruccion->parametros[i]);
	}
	free(instruccion->parametros);
	free((void*)instruccion); // Se castea a void para hacer free de un const
}

void cicloInstruccion(t_contextoEjecucion *contextoEjecucion) {

	contexto = contextoEjecucion;
	hayInterrupciones = false;

	inicializarSemaforos();

	// Creamos un hilo que reciba las interrupciones
	pthread_t interrupciones;
	pthread_create(&interrupciones, NULL, (void*) recibirInterrupciones, NULL);

	// --- EJECUCION DEL CICLO --- //

	while (hayContexto() && !hayInterrupciones) {

		char *instruccionADecodificar = fetch();
		const t_instruccion *instruccion = decode(instruccionADecodificar);
		execute(instruccion);
		liberarInstruccion(instruccion);
	}

	// Despues de ejecutar una instruccion, verificar si hay interrupciones

	checkInterrupt(interrupciones);

}

// Funciones inicializacion

void inicializarSemaforos() {
	sem_init(&atenderInterrupciones, 0, 0);

	sem_init(&mutexContexto, 0, 1);

}

// Funciones de FETCH

void solicitarInstruccionAMemoria() {
	t_paquete *paqueteSolicitudInstruccion = crearPaquete(SOLICITUD_INSTRUCCION);
	agregarAPaquete(paqueteSolicitudInstruccion, &(contexto->PID), sizeof(int));
	agregarAPaquete(paqueteSolicitudInstruccion, &(contexto->PC), sizeof(uint32_t));
	enviarPaquete(paqueteSolicitudInstruccion, socketClienteMemoria);
	eliminarPaquete(paqueteSolicitudInstruccion);
}

char* obtenerInstruccion(int socketCliente) {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketCliente);
	char *instruccion = malloc(buffer->size * sizeof(char) + 1);
	memcpy(instruccion, buffer->stream, sizeof(char) * buffer->size);
	instruccion[buffer->size] = '\0';
	liberarBuffer(buffer);
	return instruccion;
}

char* recibirInstruccion(int socketCliente) {
	t_opCode opCode;
	if (recv(socketCliente, &opCode, sizeof(t_opCode), MSG_WAITALL) != -1 && opCode == INTERCAMBIO_INSTRUCCION) {
		char *instruccion = obtenerInstruccion(socketCliente);
		return instruccion;
	} else {
		return NULL;
	}
}

// Funciones de DECODE

t_operacion stringToOperacion(char *operacion) {

	if (strcmp(operacion, "SET") == 0)
		return SET;
	else if (strcmp(operacion, "SUM") == 0)
		return SUM;
	else if (strcmp(operacion, "SUB") == 0)
		return SUB;
	else if (strcmp(operacion, "JNZ") == 0)
		return JNZ;
	else if (strcmp(operacion, "IO_GEN_SLEEP") == 0)
		return IO_GEN_SLEEP;
	else if (strcmp(operacion, "MOV_IN") == 0)
		return MOV_IN;
	else if (strcmp(operacion, "MOV_OUT") == 0)
		return MOV_OUT;
	else if (strcmp(operacion, "RESIZE") == 0)
		return RESIZE;
	else if (strcmp(operacion, "COPY_STRING") == 0)
		return COPY_STRING;
	else if (strcmp(operacion, "IO_STDIN_READ") == 0)
		return IO_STDIN_READ;
	else if (strcmp(operacion, "IO_STDOUT_WRITE") == 0)
		return IO_STDOUT_WRITE;
	else if (strcmp(operacion, "EXIT") == 0)
		return INSTRUCCION_EXIT;
	else if (strcmp(operacion, "WAIT") == 0)
		return WAIT;
	else if (strcmp(operacion, "SIGNAL") == 0)
		return SIGNAL;
	else if (strcmp(operacion, "IO_FS_CREATE") == 0)
		return IO_FS_CREATE;
	else if (strcmp(operacion, "IO_FS_DELETE") == 0)
		return IO_FS_DELETE;
	else if (strcmp(operacion, "IO_FS_TRUNCATE") == 0)
		return IO_FS_TRUNCATE;
	else if (strcmp(operacion, "IO_FS_WRITE") == 0)
		return IO_FS_WRITE;
	else if (strcmp(operacion, "IO_FS_READ") == 0)
		return IO_FS_READ;
	else
		return -1;
}

// Funciones para el chequeo de interrupciones

bool hayContexto() {
	return contexto != NULL;
}

// CICLO DE INSTRUCCION:

char* fetch() {
	log_info(auxLogger, "PID: %d - FETCH - Program Counter: %d", contexto->PID, contexto->PC);

	solicitarInstruccionAMemoria();
	char *instruccion = recibirInstruccion(socketClienteMemoria);
	return instruccion;
}

t_operacion obtenerOperacion(char *instruccionADecodificar, char *delimitador) {
	char *operacion = strtok(instruccionADecodificar, delimitador);
	return stringToOperacion(operacion);
}

void obtenerParametros(char ***parametros, int *cantidadParametros, size_t capacidad, char *delimitador) {
	char *parametro = strtok(NULL, delimitador);
	while (parametro != NULL) {
		if (*cantidadParametros >= capacidad) {
			// Redimensionar el array de punteros si es necesario
			capacidad += 5;
			*parametros = realloc(*parametros, capacidad * sizeof(char*));
		}
		(*parametros)[*cantidadParametros] = strdup(parametro); // Duplicar la cadena
		(*cantidadParametros)++;
		parametro = strtok(NULL, delimitador);
	}
}

const t_instruccion* decode(char *instruccionADecodificar) {
	t_instruccion *instruccionDecodificada = malloc(sizeof(t_instruccion));

	char *delimitador = " \n";

	instruccionDecodificada->operacion = obtenerOperacion(instruccionADecodificar, delimitador);

	size_t capacidad = 5;

	char **parametros = malloc(capacidad * sizeof(char*));

	int cantidadParametros = 0;

	obtenerParametros(&parametros, &cantidadParametros, capacidad, delimitador);

	instruccionDecodificada->parametros = parametros;
	instruccionDecodificada->cantidadParametros = cantidadParametros;

	free(instruccionADecodificar);
	return instruccionDecodificada;
}

void execute(const t_instruccion *instruccion) {

	t_operacion operacion = instruccion->operacion;

	int PID = contexto->PID;

	switch (operacion) {
	case SET:
		log_info(auxLogger, "PID: %d - Ejecutando: SET - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		set(instruccion);
		break;
	case SUM:
		log_info(auxLogger, "PID: %d - Ejecutando: SUM - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		sum(instruccion);
		break;
	case SUB:
		log_info(auxLogger, "PID: %d - Ejecutando: SUB - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		sub(instruccion);
		break;
	case JNZ:
		log_info(auxLogger, "PID: %d - Ejecutando: JNZ - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		jnz(instruccion);
		break;
	case IO_GEN_SLEEP:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_GEN_SLEEP - %s ", PID, instruccion->parametros[0]);
		ioGenSleep(instruccion);
		break;
	case MOV_IN:
		log_info(auxLogger, "PID: %d - Ejecutando: MOV_IN - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		movIn(instruccion);
		break;
	case MOV_OUT:
		log_info(auxLogger, "PID: %d - Ejecutando: MOV_OUT - %s %s", PID, instruccion->parametros[0], instruccion->parametros[1]);
		movOut(instruccion);
		break;
	case RESIZE:
		log_info(auxLogger, "PID: %d - Ejecutando: RESIZE - %s", PID, instruccion->parametros[0]);
		resize(instruccion);
		break;
	case COPY_STRING:
		log_info(auxLogger, "PID: %d - Ejecutando: COPY_STRING - %s", PID, instruccion->parametros[0]);
		copyString(instruccion);
		break;
	case IO_STDIN_READ:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_STDIN_READ - %s %s %s", PID, instruccion->parametros[0], instruccion->parametros[1],
				instruccion->parametros[2]);
		ioStdinRead(instruccion);
		break;
	case IO_STDOUT_WRITE:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_STDOUT_WRITE - %s %s %s", PID, instruccion->parametros[0], instruccion->parametros[1],
				instruccion->parametros[2]);
		ioStdoutWrite(instruccion);
		break;
	case IO_FS_CREATE:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_FS_CREATE - %s %s ", PID, instruccion->parametros[0], instruccion->parametros[1]);
		ioFSCreateOrDelete(instruccion,SOLICITUD_IO_FS_CREATE);
		break;
	case IO_FS_DELETE:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_FS_DELETE - %s %s ", PID, instruccion->parametros[0], instruccion->parametros[1]);
		ioFSCreateOrDelete(instruccion,SOLICITUD_IO_FS_DELETE);
		break;
	case IO_FS_TRUNCATE:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_FS_TRUNCATE - %s %s %s", PID, instruccion->parametros[0], instruccion->parametros[1],
				instruccion->parametros[2]);
		ioFSTruncate(instruccion);
		break;
	case IO_FS_WRITE:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_FS_WRITE - %s %s %s %s %s", PID, instruccion->parametros[0], instruccion->parametros[1],
				instruccion->parametros[2],instruccion->parametros[3],instruccion->parametros[4]);
		ioFSWriteOrRead(instruccion,SOLICITUD_IO_FS_WRITE);
		break;
	case IO_FS_READ:
		log_info(auxLogger, "PID: %d - Ejecutando: IO_FS_READ - %s %s %s %s %s", PID, instruccion->parametros[0], instruccion->parametros[1],
				instruccion->parametros[2],instruccion->parametros[3],instruccion->parametros[4]);
		ioFSWriteOrRead(instruccion,SOLICITUD_IO_FS_READ);
		break;
	case INSTRUCCION_EXIT:
		log_info(auxLogger, "PID: %d - Ejecutando: EXIT ", PID);
		instruccionExit();
		break;
	case WAIT:
		log_info(auxLogger, "PID: %d - Ejecutando: WAIT - %s", PID, instruccion->parametros[0]);
		instruccionWait(instruccion);
		break;
	case SIGNAL:
		log_info(auxLogger, "PID: %d - Ejecutando: SIGNAL - %s", PID, instruccion->parametros[0]);
		instruccionSignal(instruccion);
		break;
	default:
		break;
	}
}

void recibirInterrupciones() {

	t_opCode motivoInterrupcion = recibirOperacion(socketClienteInterrupt);
//	int PID = 0;
//	recv(socketClienteInterrupt, &PID, sizeof(int), MSG_WAITALL);

	hayInterrupciones = true;

	sem_wait(&atenderInterrupciones);

	if (hayContexto() /* && contexto->PID == PID */) {

		sem_wait(&mutexContexto);

		enviarContexto(contexto, socketClienteDispatch, motivoInterrupcion);
		liberarContexto(contexto);

		sem_post(&mutexContexto);
	}
}
