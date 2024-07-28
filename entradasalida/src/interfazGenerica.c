#include "../include/entradasalida.h"


void dormirInterfaz (t_parametrosInterfaz* param, int PID){
	useconds_t tiempo = 1000*(cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO)*(param->tiempoSleep);
	usleep (tiempo);
	enviarPID(PID,socketConexionKernel);
}

