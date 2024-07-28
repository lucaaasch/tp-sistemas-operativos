#include "../include/entradasalida.h"

// Propuesta:
void esperarEntradaPorTeclado(t_parametrosInterfaz *parametros, int PID) {
	char *entrada = readline("> ");

	while (strlen(entrada) > parametros->limiteEntrada) {
		printf("Entrada por teclado demasiado larga. Por favor, ingresar otra:\n");
		free(entrada);
		entrada = readline("> ");
	}
	enviarEntradaConBuffer(parametros, entrada, PID, SOLICITUD_ESCRITURA);
	free(entrada);
	if (recibirOperacion(socketConexionMemoria) == ACCESO_OK) {
		enviarPIDAKernel(PID, SOLICITUD_STDIN);
	}
}
