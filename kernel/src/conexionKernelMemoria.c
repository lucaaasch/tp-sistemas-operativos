#include "../include/conexionKernelMemoria.h"

void conectarAMemoria(){
	socketMemoria = crearConexion(cfgKernel->IP_MEMORIA,
			cfgKernel->PUERTO_MEMORIA, auxLogger);
	if (socketMemoria == -1) {
		log_error(auxLogger,"La conexion con la MEMORIA fallo");
		abort();
	}
}


