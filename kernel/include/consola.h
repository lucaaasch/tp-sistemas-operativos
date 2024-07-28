#ifndef INCLUDE_CONSOLA_H_
#define INCLUDE_CONSOLA_H_
#include "../include/kernel.h"

extern bool isPlanificacionPausada;
extern t_list *pcbsEnSistema;
extern int  cantAEsperar;
;

bool buscarYEliminarPcbEnCola(t_queue *cola, t_pcb *pcbBuscado);

char* imprimirPidsLista(t_list *lista);

char* imprimirPidsCola(t_queue *cola);

typedef enum {
	INICIAR_PROCESO,
	FINALIZAR_PROCESO,
	DETENER_PLANIFICACION,
	INICIAR_PLANIFICACION,
	PROCESO_ESTADO,
	EJECUTAR_SCRIPT,
	MULTIPROGRAMACION,
	ERROR
}t_mensajeDeConsola;

void consola ();

#endif /* INCLUDE_CONSOLA_H_ */
