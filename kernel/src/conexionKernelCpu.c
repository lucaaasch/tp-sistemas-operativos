#include "../include/conexionKernelCpu.h"

void conectarACpu() {
	int huboErrores = 0;
	socketDispatch = crearConexion(cfgKernel->IP_CPU, cfgKernel->PUERTO_CPU_DISPATCH, auxLogger);
	if (socketDispatch == -1) {
		log_error(auxLogger, "La conexion con el DISPATCH fallo");
		huboErrores = 1;
	}
	socketInterrupt = crearConexion(cfgKernel->IP_CPU, cfgKernel->PUERTO_CPU_INTERRUPT, auxLogger);
	if (socketInterrupt == -1) {
		log_error(auxLogger, "La conexion con el INTERRUPT fallo");
		huboErrores = 1;
	}

	if (huboErrores) {
		abort();
	}
}

void enviarContextoEjecucion (t_contextoEjecucion* contexto , int socket,t_opCode codigo){
	t_paquete* paquete = crearPaquete(codigo);
	serializarContexto(paquete,contexto);
	enviarPaquete(paquete,socket);
	eliminarPaquete(paquete);
}

