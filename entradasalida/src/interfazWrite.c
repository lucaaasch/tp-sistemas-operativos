#include "../include/entradasalida.h"

void mostrarEnConsola(t_parametrosInterfaz *parametros, int PID) {
	enviarEntrada(parametros, PID, SOLICITUD_LECTURA);

	char *valorLeido = recibirLectura(socketConexionMemoria);
	if (valorLeido != NULL) {
		log_info(auxLogger, "El valor leido es: %s", valorLeido);
		enviarPIDAKernel(PID, SOLICITUD_STDOUT);
	} else {
		log_error(auxLogger, "No se pudo leer el valor");
		enviarPIDAKernel(PID, FINALIZACION_PROCESO);
	}
	free(valorLeido);
}
