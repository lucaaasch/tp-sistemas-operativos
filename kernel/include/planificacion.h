#ifndef INCLUDE_PLANIFICACION_H_
#define INCLUDE_PLANIFICACION_H_

#include <pthread.h>
#include <utils/include/estructuras.h>
#include <utils/include/comunicacion.h>
#include "kernel.h"
#include <commons/temporal.h>
#include <time.h>
#include <sched.h>
typedef struct {
	int socketConexion;
	char *nombre;
	t_queue *colaBlocked;
	bool enUso;
} t_propiedadesEntradaSalida;

typedef struct {
	int pid;
	t_opCode codigo;
	char* nombreInterfaz;
} t_manejoBlocked;

typedef struct {
	t_opCode codigo;
	t_parametrosInterfaz* paramInterfaz;
	t_propiedadesEntradaSalida *propiedades;
	int PID;
}t_envioPcbInterfaz;

typedef struct {
	t_opCode codigo;
	t_parametrosFS* parametrosFS;
	t_propiedadesEntradaSalida *propiedades;
	int PID;
}t_envioPcbFS;
typedef struct {
		int largoNombre;
		char *nombre;
} t_nombreRecurso;

extern pthread_mutex_t mutexColaEstadoNew;
extern pthread_mutex_t mutexListaEstadoReady;
extern pthread_mutex_t mutexPcbEnExec;
extern pthread_mutex_t mutexColaEstadoBlocked;
extern pthread_mutex_t mutexListaEstadoExit;
extern pthread_mutex_t mutexListaPcbsEnSistema;

extern sem_t semHayProcesosEnNew;
extern sem_t semMultiprogramacion;
extern sem_t semProcesoEnExec;
extern sem_t semHayProcesoEnReady;
extern sem_t semProcesoEnBlocked;
extern sem_t semParaComunicacionFS;
extern sem_t semParaComunicacionSleep;
extern sem_t semParaComunicacionWrite;
extern sem_t semParaComunicacionRead;
extern sem_t **semaforosRecursos;

extern sem_t semPausaPlanificacionLargo;
extern sem_t semPausaPlanificacionCorto;
extern sem_t semPausaMotivoDesalojo;
extern sem_t semPausaBlockedAReady;
extern sem_t semPausarReadyAExec;
extern sem_t semPausarNewAReady;
extern sem_t semPausarExit;

extern t_queue *colaEstadoNew;
extern t_list *listaEstadoReady;
extern t_pcb *pcbEnExec;
extern t_queue *colaEstadoBlocked;
extern t_list *listaEstadoExit;
extern t_queue **colasRecursos;
extern t_queue *colaPrioritariaReady;


extern pthread_t algoritmoPlanificacion;

void inicializarSemaforosRecursos();

void inicializarPlanificacion();

void inicializarColasRecuros();

void envioParametrosAInterfaz(t_list* listaPcbAEnviar);

bool algunaInterfazLibre(t_list* lista);
bool algunaInterfazLibreRead(t_list* lista);
bool algunaInterfazLibreWrite(t_list* lista);
bool algunaInterfazLibreFS(t_list *lista);

//Planificador de largo plazo

void cambiarEstadoPcb(t_pcb *pcb, t_estado estadoNuevo);

void* planificarLargoPlazo();

t_paquete* obtenerYActualizarContexto(t_pcb *pcb, int *desplazamiento, t_contextoEjecucion *contexto);

void* planificarCortoPlazo();

bool esSyscall(t_opCode codigo);

t_algoritmoPlanificacion obtenerAlgoritmoPlanificacion();

void ingresarANew(t_pcb *pcb);

t_pcb* ingresarNewAReady();

t_pcb* obtenerSiguienteNew();

void agregarAColaReady(t_pcb *pcb);

void agregarAExit(t_pcb *pcb, t_opCode motivo);

//Planificador de corto plazo

t_pcb* obtenerSiguienteReady(t_algoritmoPlanificacion algoritmo);

void agregarAExecute(t_pcb *pcb, t_algoritmoPlanificacion algoritmo);

t_pcb* obtenerSiguenteExecute();

t_pcb* agregarABlocked(t_pcb *pcb, char *nombreInterfaz, t_parametrosInterfaz *parametros, t_propiedadesEntradaSalida **propiedadesES);

t_pcb* obtenerSiguienteBlocked(t_queue *cola);

void manejarInterrupcionesYFinQuantum(t_pcb *pcb);

void planificarPorFifo(void *pcb);

void planificarPorRR(void *pcb);

void planificarPorVRR(void *pcb);

void enviarInterrupcion(t_opCode motivoInterrupcion);

void esperarPIDADesbloquear(t_manejoBlocked *pidYInterfaz);

void esperarPidPcbADesbloquearStd(t_manejoBlocked *pidYInterfaz);

void actualizarQuantumRestante(t_pcb *pcb, t_algoritmoPlanificacion algoritmo);

t_nombreRecurso* recibirNombreRecurso (t_paquete *paquete, int *desplazamientoAux);

void actualizarContexto(t_pcb *pcb, t_contextoEjecucion *contexto);

void gestionarDesalojo(t_pcb *pcb, t_paquete *paquete, int desplazamiento);

void recibirParametrosInterfaz(t_parametrosInterfaz *param, t_buffer *buffer, t_opCode codigo);

void liberarEstructurasPlanificacion();
int encontrarRecurso(char *cadena, char *array[]);
int valorSemaforo (sem_t *sem);
void enviarPidMemoria (int pid);
void esperarPIDInterfaz(t_manejoBlocked *pidEInterfaz);

void terminarProceso(t_pcb *pcb, t_opCode motivo);

void sacarDeExecute(t_pcb *pcb);

void desasignarRecursos (t_pcb *pcb);

int valorSemaforo(sem_t *sem);
void actualizarSemMultiprogramacion (t_pcb * pcb);
void esperarPidPcbADesbloquearRead(t_manejoBlocked *pidYInterfaz);
void esperarPidPcbADesbloquearWrite(t_manejoBlocked *pidYInterfaz);
void esperarPIDADesbloquearFS(t_manejoBlocked *pidYInterfaz);
t_envioPcbFS* obtenerYRemoverPrimeroLibreFS(t_list* lista);

void liberarPcb (t_pcb * pcb);

#endif /* INCLUDE_PLANIFICACION_H_ */
