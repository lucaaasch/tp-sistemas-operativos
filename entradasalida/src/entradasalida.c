#include "../include/entradasalida.h"

t_config_entradasalida *cfgEntradaSalida;
t_log *auxLogger;
int socketConexionMemoria = 0, socketConexionKernel = 0;

int main(int argc, char *argv[]) {

	inicializarLoggers();

	if (!inicializarConfig(argv[1])) {
		log_info(auxLogger, "No se pudo inicializar la configuracion");
		return 1;
	}

	log_info(auxLogger, "Se inicializo una E/S de tipo: %s", cfgEntradaSalida->TIPO_INTERFAZ);

	char *nombreInterfaz = consultarNombre();

	if (!generarConexiones(&socketConexionMemoria, &socketConexionKernel)) {
		log_destroy(auxLogger);
		return EXIT_FAILURE;
	}

	enviarString(nombreInterfaz, socketConexionKernel, NOMBRE_ENTRADA_SALIDA);

	esperarEntradaSalida();

	liberarPrograma();
	return 0;
}

void liberarPrograma() {
	log_destroy(auxLogger);
	liberarConfig();
	liberarConexion(socketConexionMemoria);
	liberarConexion(socketConexionKernel);
}

void inicializarLoggers() {
	auxLogger = iniciarLogger("entradaSalida.log", "ENTRADA_SALIDA", 1, LOG_LEVEL_TRACE);
}
