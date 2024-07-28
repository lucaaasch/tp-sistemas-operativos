#include "../include/cpu.h"

void devolverSolamenteContexto(t_opCode syscall) {
	if (hayContexto()) {
		sem_wait(&mutexContexto);

		enviarContexto(contexto, socketClienteDispatch, syscall);
		liberarContexto(contexto);
		contexto = NULL;

		sem_post(&mutexContexto);
	}
}

void* obtenerRegistro(char *nombreRegistro, int *sizeRegistro) {

	t_registrosGenerales *registros = contexto->registrosGenerales;
	t_registrosMemoria *registrosMemoria = contexto->registrosMemoria;

	if (strcmp(nombreRegistro, "PC") == 0) {
		*sizeRegistro = 4;
		return (void*) &(contexto->PC);
	} else if (strcmp(nombreRegistro, "AX") == 0) {
		*sizeRegistro = 1;
		return (void*) &(registros->AX);
	} else if (strcmp(nombreRegistro, "BX") == 0) {
		*sizeRegistro = 1;
		return (void*) &(registros->BX);
	} else if (strcmp(nombreRegistro, "CX") == 0) {
		*sizeRegistro = 1;
		return (void*) &(registros->CX);
	} else if (strcmp(nombreRegistro, "DX") == 0) {
		*sizeRegistro = 1;
		return (void*) &(registros->DX);
	} else if (strcmp(nombreRegistro, "EAX") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registros->EAX);
	} else if (strcmp(nombreRegistro, "EBX") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registros->EBX);
	} else if (strcmp(nombreRegistro, "ECX") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registros->ECX);
	} else if (strcmp(nombreRegistro, "EDX") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registros->EDX);
	} else if (strcmp(nombreRegistro, "SI") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registrosMemoria->SI);
	} else if (strcmp(nombreRegistro, "DI") == 0) {
		*sizeRegistro = 4;
		return (void*) &(registrosMemoria->DI);
	} else {
		sizeRegistro = NULL;
		return NULL;
	}
}

int obtenerValorNumerico(char *strNumber) {
	return atoi(strNumber);
}

void reasignarValor(int size, int valor, void *registro) {
	sem_wait(&mutexContexto);
	memcpy(registro, &valor, size);
	sem_post(&mutexContexto);
}

int obtenerValorRegistro(int sizeValor, void *registro) {
	int valor = 0;
	memcpy(&valor, registro, sizeValor);
	if (sizeValor == 1) {
		valor = (int) (uint8_t) valor;
	} else {
		valor = (int) (uint32_t) valor;
	}
	return valor;
}

void set(const t_instruccion *instruccion) {
	contexto->PC++;
	int size = 0;
	void *registro = obtenerRegistro(instruccion->parametros[0], &size);
	int valor = obtenerValorNumerico(instruccion->parametros[1]);

	reasignarValor(size, valor, registro);
}

void sum(const t_instruccion *instruccion) {
	int sizeValorDestino = 0;
	void *registroDestino = obtenerRegistro(instruccion->parametros[0], &sizeValorDestino);
	int sizeValorOrigen = 0;
	void *registroOrigen = obtenerRegistro(instruccion->parametros[1], &sizeValorOrigen);

	int valorDestino = obtenerValorRegistro(sizeValorDestino, registroDestino);
	int valorOrigen = obtenerValorRegistro(sizeValorOrigen, registroOrigen);

	reasignarValor(sizeValorDestino, valorDestino + valorOrigen, registroDestino);
	contexto->PC++;
}

void sub(const t_instruccion *instruccion) {
	int sizeValorDestino = 0;
	void *registroDestino = obtenerRegistro(instruccion->parametros[0], &sizeValorDestino);
	int sizeValorOrigen = 0;
	void *registroOrigen = obtenerRegistro(instruccion->parametros[1], &sizeValorOrigen);

	int valorDestino = obtenerValorRegistro(sizeValorDestino, registroDestino);
	int valorOrigen = obtenerValorRegistro(sizeValorOrigen, registroOrigen);

	reasignarValor(sizeValorDestino, valorDestino - valorOrigen, registroDestino);
	contexto->PC++;
}

void jnz(const t_instruccion *instruccion) {
	int size = 0;
	int valor = 0;
	void *registro = obtenerRegistro(instruccion->parametros[0], &size);
	memcpy(&valor, registro, size);

	uint32_t direccionInstruccion = (uint32_t) obtenerValorNumerico(instruccion->parametros[1]);
	sem_wait(&mutexContexto);
	if (valor != 0) {
		contexto->PC = direccionInstruccion;
	}
	sem_post(&mutexContexto);
}

void liberarParametrosInterfazCPU(t_parametrosInterfaz *parametrosInterfaz) {
	if (parametrosInterfaz != NULL) {
		if (parametrosInterfaz->direccionesFisicas != NULL) {
			free(parametrosInterfaz->direccionesFisicas);
		}
		free(parametrosInterfaz);
	}
}

void liberarParametrosFSCPU(t_parametrosFS *parametrosFS) {
	if (parametrosFS != NULL) {
		liberarParametrosInterfazCPU(parametrosFS->parametrosInterfaz);
		free(parametrosFS);
	}
}

void devolverContextoConParametrosInterfaces(const t_instruccion *instruccion, t_opCode syscall) {
	if (hayContexto()) {
		t_paquete *paquete = crearPaquete(syscall);
		sem_wait(&mutexContexto);
		serializarContexto(paquete, contexto);
		liberarContexto(contexto);
		contexto = NULL;
		sem_post(&mutexContexto);
		char *nombreInterfaz = instruccion->parametros[0];
		int tiempoSleep = strtoul(instruccion->parametros[1], NULL, 10);
		t_parametrosInterfaz *parametros = inicializarParametros(nombreInterfaz, "", tiempoSleep, NULL, 0, 0);
		serializarParametros(paquete, parametros);
		enviarPaquete(paquete, socketClienteDispatch);
		eliminarPaquete(paquete);

		liberarParametrosInterfazCPU(parametros);
	}
}

void ioGenSleep(const t_instruccion *instruccion) {
	contexto->PC++;
	devolverContextoConParametrosInterfaces(instruccion, SYSCALL_IO_GEN_SLEEP);
}

void instruccionExit() {
	contexto->PC++;
	devolverSolamenteContexto(SYSCALL_EXIT);

	/* Para testear como se liberan los recursos
	 sleep(5);
	 liberarRecursosCpu();
	 exit(EXIT_SUCCESS);
	 */
}

void movIn(const t_instruccion *instruccion) {
	int sizeDatos = 0;
	void *registroDatos = obtenerRegistro(instruccion->parametros[0], &sizeDatos);

	int sizeDireccion = 0;
	void *registroDireccion = obtenerRegistro(instruccion->parametros[1], &sizeDireccion);

	int direccionLogica = obtenerValorRegistro(sizeDireccion, registroDireccion);

	int sizeDireccionesFisicas = 0;
	int *direccionesFisicas = traducirDirecciones(direccionLogica, sizeDatos, &sizeDireccionesFisicas);

	solicitarLectura(direccionesFisicas, sizeDireccionesFisicas, sizeDatos, contexto->PID, socketClienteMemoria);

	void *lectura = recibirLectura(socketClienteMemoria);

	memcpy(registroDatos, lectura, sizeDatos);

	int valorLeido = obtenerValorRegistro(sizeDatos, registroDatos);
	log_info(auxLogger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %d", contexto->PID, direccionesFisicas[0], valorLeido);

	free(lectura);
	free(direccionesFisicas);
	contexto->PC++;
}

void* obtenerValorRegistroComoBuffer(void *registro, int sizeValorRegistro) {
	void *buffer = malloc(sizeValorRegistro);
	memcpy(buffer, registro, sizeValorRegistro);
	return buffer;
}

void movOut(const t_instruccion *instruccion) {
	int sizeDireccion = 0;
	void *registroDireccion = obtenerRegistro(instruccion->parametros[0], &sizeDireccion);

	int sizeDatos = 0;
	void *registroDatos = obtenerRegistro(instruccion->parametros[1], &sizeDatos);

	int direccionLogica = obtenerValorRegistro(sizeDireccion, registroDireccion);
	void *buffer = obtenerValorRegistroComoBuffer(registroDatos, sizeDatos);

	int sizeDireccionesFisicas = 0;
	int *direccionesFisicas = traducirDirecciones(direccionLogica, sizeDatos, &sizeDireccionesFisicas);

	solicitarEscritura(direccionesFisicas, sizeDireccionesFisicas, buffer, sizeDatos, contexto->PID, socketClienteMemoria);

	int valorEscrito = obtenerValorRegistro(sizeDatos, registroDatos);
	log_info(auxLogger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %d", contexto->PID, direccionesFisicas[0],
				valorEscrito);

	recibirOperacion(socketClienteMemoria);

	free(buffer);
	free(direccionesFisicas);
	contexto->PC++;

}

void solicitarResize(int size) {
	t_paquete *paqueteLectura = crearPaquete(SOLICITUD_RESIZE);

	agregarAPaquete(paqueteLectura, &(contexto->PID), sizeof(int));

	agregarAPaquete(paqueteLectura, &size, sizeof(int));

	enviarPaquete(paqueteLectura, socketClienteMemoria);

	eliminarPaquete(paqueteLectura);
}

void resize(const t_instruccion *instruccion) {
	int size = obtenerValorNumerico(instruccion->parametros[0]);

	solicitarResize(size);

	t_opCode respuestaMemoria = recibirOperacion(socketClienteMemoria);

	contexto->PC++;

	if (respuestaMemoria == ERROR_OUT_OF_MEMORY) {
		devolverSolamenteContexto(respuestaMemoria);
	}
}

void copyString(const t_instruccion *instruccion) {
	int size = obtenerValorNumerico(instruccion->parametros[0]);

	uint32_t registroSI = contexto->registrosMemoria->SI;
	uint32_t registroDI = contexto->registrosMemoria->DI;

	int cantidadDireccionesLectura = 0;
	int *direccionesLectura = traducirDirecciones(registroSI, size, &cantidadDireccionesLectura);

	solicitarLectura(direccionesLectura, cantidadDireccionesLectura, size, contexto->PID, socketClienteMemoria);

	char *lectura = recibirLectura(socketClienteMemoria);

	log_info(auxLogger, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %s", contexto->PID, direccionesLectura[0], lectura);

	int cantidadDireccionesEscritura = 0;
	int *direccionesEscritura = traducirDirecciones(registroDI, size, &cantidadDireccionesEscritura);

	solicitarEscritura(direccionesEscritura, cantidadDireccionesEscritura, lectura, size, contexto->PID, socketClienteMemoria);
	recibirOperacion(socketClienteMemoria);

	log_info(auxLogger, "PID: %d - Acción: ESCRIBIR - Dirección Física: %d - Valor: %s", contexto->PID, direccionesEscritura[0], lectura);

	free(lectura);
	free(direccionesLectura);
	free(direccionesEscritura);

	contexto->PC++;
}

void enviarContextoConParametrosInterfaces(t_parametrosInterfaz *parametros, t_opCode syscallInterfaz) {
	t_paquete *paquete = crearPaquete(syscallInterfaz);

	serializarContexto(paquete, contexto);
	serializarParametros(paquete, parametros);
	enviarPaquete(paquete, socketClienteDispatch);
	eliminarPaquete(paquete);
}

void devolverContextoParaEntradaSalida(t_parametrosInterfaz *parametros, t_opCode syscallInterfaz) {
	if (hayContexto()) {
		sem_wait(&mutexContexto);
		enviarContextoConParametrosInterfaces(parametros, syscallInterfaz);
		liberarContexto(contexto);
		contexto = NULL;
		sem_post(&mutexContexto);
	}
}

void gestionarSTD(const t_instruccion *instruccion, t_opCode operacion) {
	char *interfaz = instruccion->parametros[0];
	int sizeDireccion = 0;
	void *registroDireccion = obtenerRegistro(instruccion->parametros[1], &sizeDireccion);
	int sizeTamanio = 0;
	void *registroTamanio = obtenerRegistro(instruccion->parametros[2], &sizeTamanio);
	int direccionLogica = obtenerValorRegistro(sizeDireccion, registroDireccion);
	int size = obtenerValorRegistro(sizeTamanio, registroTamanio);
	int cantidadDireccionesFisicas = 0;
	int *direccionesFisicas = traducirDirecciones(direccionLogica, size, &cantidadDireccionesFisicas);
	t_parametrosInterfaz *parametros = inicializarParametros(interfaz, "", 0, direccionesFisicas, cantidadDireccionesFisicas, size);

	devolverContextoParaEntradaSalida(parametros, operacion);

	free(direccionesFisicas);
	liberarParametrosInterfazCPU(parametros);

}

void ioStdinRead(const t_instruccion *instruccion) {
	contexto->PC++;
	gestionarSTD(instruccion, SYSCALL_IO_STDIN_READ);
}

void ioStdoutWrite(const t_instruccion *instruccion) {
	contexto->PC++;
	gestionarSTD(instruccion, SYSCALL_IO_STDOUT_WRITE);
}

void enviarContextoConRecurso(const t_instruccion *instruccion, t_opCode syscallInterfaz) {
	t_paquete *paquete = crearPaquete(syscallInterfaz);

	serializarContexto(paquete, contexto);

	int largoRecurso = strlen(instruccion->parametros[0]);
	agregarAPaquete(paquete, &largoRecurso, sizeof(int));
	agregarAPaquete(paquete, instruccion->parametros[0], largoRecurso * sizeof(char));

	enviarPaquete(paquete, socketClienteDispatch);
	eliminarPaquete(paquete);
}

void devolverContextoConRecurso(const t_instruccion *instruccion, t_opCode syscallInterfaz) {
	if (hayContexto()) {
		sem_wait(&mutexContexto);
		sem_post(&semaforoCPUOcupada);
		enviarContextoConRecurso(instruccion, syscallInterfaz);
		sem_wait(&manejoRecursos);
		if (!mantenerEnExec) {
			liberarContexto(contexto);
			contexto = NULL;
		}
		sem_post(&mutexContexto);
	}
}

void instruccionWait(const t_instruccion *instruccion) {
	contexto->PC++;
	devolverContextoConRecurso(instruccion, SYSCALL_WAIT);
}

void instruccionSignal(const t_instruccion *instruccion) {
	contexto->PC++;
	devolverContextoConRecurso(instruccion, SYSCALL_SIGNAL);
}

void devolverContextoConParametrosFS(const t_instruccion *instruccion, t_opCode syscall, t_parametrosFS *parametrosFS) {
	if (hayContexto()) {
		t_paquete *paquete = crearPaquete(syscall);
		sem_wait(&mutexContexto);
		serializarContexto(paquete, contexto);
		liberarContexto(contexto);
		contexto = NULL;
		sem_post(&mutexContexto);
		serializarParametrosFS(paquete, parametrosFS);
		enviarPaquete(paquete, socketClienteDispatch);
		eliminarPaquete(paquete);
	}
}

void ioFSCreateOrDelete(const t_instruccion *instruccion, t_opCode operacion) {
	contexto->PC++;

	char *nombreInterfaz = instruccion->parametros[0];
	char *nombreArchivo = instruccion->parametros[1];

	t_parametrosInterfaz *parametrosInterfaz = inicializarParametros(nombreInterfaz, "", 0, NULL, 0, 0);
	t_parametrosFS *parametrosFS = inicializarParametrosFS(nombreArchivo, 0, parametrosInterfaz);

	devolverContextoConParametrosFS(instruccion, operacion, parametrosFS);
	liberarParametrosFSCPU(parametrosFS);

}

void ioFSTruncate(const t_instruccion *instruccion) {
	contexto->PC++;

	char *nombreInterfaz = instruccion->parametros[0];
	char *nombreArchivo = instruccion->parametros[1];

	int sizeRegistroTamanio = 0;
	void *registroTamanio = obtenerRegistro(instruccion->parametros[2], &sizeRegistroTamanio);
	int tamanioTruncado = obtenerValorRegistro(sizeRegistroTamanio, registroTamanio);

	// Estaba pactado utilizar largoTexto para enviar el tamanio del truncado pero no es posible por la funcion. En su defecto, usare limiteEntrada
	t_parametrosInterfaz *parametrosInterfaz = inicializarParametros(nombreInterfaz, "", 0, NULL, 0, tamanioTruncado);
	t_parametrosFS *parametrosFS = inicializarParametrosFS(nombreArchivo, 0, parametrosInterfaz);

	devolverContextoConParametrosFS(instruccion, SOLICITUD_IO_FS_TRUNCATE, parametrosFS);
	liberarParametrosFSCPU(parametrosFS);

}

void ioFSWriteOrRead(const t_instruccion *instruccion, t_opCode operacion) {
	contexto->PC++;

	char *nombreInterfaz = instruccion->parametros[0];
	char *nombreArchivo = instruccion->parametros[1];

	int sizeRegistroTamanio = 0;
	void *registroTamanio = obtenerRegistro(instruccion->parametros[3], &sizeRegistroTamanio);
	int bytesLectura = obtenerValorRegistro(sizeRegistroTamanio, registroTamanio);

	int sizeDireccion = 0;
	void *registroDireccion = obtenerRegistro(instruccion->parametros[2], &sizeDireccion);
	int direccionLogica = obtenerValorRegistro(sizeDireccion, registroDireccion);
	int cantidadDireccionesFisicas = 0;
	int *direccionesFisicas = traducirDirecciones(direccionLogica, bytesLectura, &cantidadDireccionesFisicas);

	int sizeRegistroPunteroArchivo = 0;
	void *registroPunteroArchivo = obtenerRegistro(instruccion->parametros[4], &sizeRegistroPunteroArchivo);
	int punteroArchivo = obtenerValorRegistro(sizeRegistroPunteroArchivo, registroPunteroArchivo);

	t_parametrosInterfaz *parametrosInterfaz = inicializarParametros(nombreInterfaz, "", 0, direccionesFisicas, cantidadDireccionesFisicas,
			bytesLectura);
	t_parametrosFS *parametrosFS = inicializarParametrosFS(nombreArchivo, punteroArchivo, parametrosInterfaz);

	devolverContextoConParametrosFS(instruccion, operacion, parametrosFS);

	free(direccionesFisicas);
	liberarParametrosFSCPU(parametrosFS);
}
