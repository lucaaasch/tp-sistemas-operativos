#include "../include/cpu.h"

void recibirTamPagina() {
	t_buffer *buffer = crearBuffer();
	buffer->stream = recibirBuffer(&(buffer->size), socketClienteMemoria);
	memcpy(&tamPagina, buffer->stream, sizeof(uint16_t));
	liberarBuffer(buffer);
}

void conectarAMemoria() {

	socketClienteMemoria = crearConexion(cfgCpu->IP_MEMORIA, cfgCpu->PUERTO_MEMORIA,auxLogger);

	if (socketClienteMemoria == -1) {
		log_error(auxLogger, "La conexion con la MEMORIA fallo");
		abort();
	}

	enviarOperacion(SOLICITUD_TAM_PAGINA, socketClienteMemoria);

	tamPagina = 0;
	// Recibo el tamanio de la pagina antes de empezar el ciclo de instruccion
	if (recibirOperacion(socketClienteMemoria) == SOLICITUD_TAM_PAGINA) {
		recibirTamPagina();
	}
}
