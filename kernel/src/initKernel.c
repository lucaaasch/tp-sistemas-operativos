#include "../include/initKernel.h"
#include "../include/kernel.h"

int* instanciasRecursos;

void imprimirConfigCargada() {
	printf("PUERTO_ESCUCHA: %s \n", cfgKernel->PUERTO_ESCUCHA);
	printf("IP_MEMORIA: %s \n", cfgKernel->IP_MEMORIA);
	printf("PUERTO_MEMORIA: %s \n", cfgKernel->PUERTO_MEMORIA);
	printf("IP_CPU: %s \n", cfgKernel->IP_CPU);
	printf("PUERTO_CPU_DISPATCH: %s \n", cfgKernel->PUERTO_CPU_DISPATCH);
	printf("PUERTO_CPU_INTERRUPT: %s \n", cfgKernel->PUERTO_CPU_INTERRUPT);
	printf("ALGORITMO_PLANIFICACION: %s \n", cfgKernel->ALGORITMO_PLANIFICACION);
	printf("QUANTUM: %i \n", cfgKernel->QUANTUM);
	printf("GRADO_MULTIPROGRAMACION: %i \n", cfgKernel->GRADO_MULTIPROGRAMACION);

	int cantidadRecursos = contarRecursos(cfgKernel->RECURSOS);

	char *recursosAImprimir = malloc(cantidadRecursos * 3 + 3);
	strcpy(recursosAImprimir, "[");
	for (int i = 0; i < cantidadRecursos; i++) {
		if (i + 1 != cantidadRecursos) {
			strcat(recursosAImprimir, (cfgKernel->RECURSOS)[i]);
			strcat(recursosAImprimir, ", ");
		} else {
			strcat(recursosAImprimir, (cfgKernel->RECURSOS)[i]);
		}
	}
	strcat(recursosAImprimir, "]");
	printf("RECURSOS: %s \n", recursosAImprimir);

	char *instanciasAImprimir = malloc(cantidadRecursos * 3 + 3);
	strcpy(instanciasAImprimir, "[");
	for (int i = 0; i < cantidadRecursos; i++) {
		if (i + 1 != cantidadRecursos) {
			strcat(instanciasAImprimir, (cfgKernel->INSTANCIAS_RECURSOS)[i]);
			strcat(instanciasAImprimir, ", ");
		} else {
			strcat(instanciasAImprimir, (cfgKernel->INSTANCIAS_RECURSOS)[i]);
		}
	}
	strcat(instanciasAImprimir, "]");
	printf("INSTANCIAS_RECURSOS: %s \n", instanciasAImprimir);

	free(recursosAImprimir);
	free(instanciasAImprimir);
}

void inicializarConfig(char* nombreConfig) {

	cfgKernel = malloc(sizeof(t_config_kernel));

	char* nombreCompleto = malloc(strlen("configs/") + strlen(nombreConfig) + strlen(".config") + 1);
	strcpy(nombreCompleto, "configs/");
	strcat(nombreCompleto, nombreConfig);
	strcat(nombreCompleto, ".config");

	t_config *cfgFile = config_create(nombreCompleto);

	if (cfgFile == NULL) {
		log_error(auxLogger, "No se pudo inicializar la configuracion");
		abort();
	}

	cfgKernel->PUERTO_ESCUCHA = strdup(config_get_string_value(cfgFile, "PUERTO_ESCUCHA"));
	cfgKernel->IP_MEMORIA = obtenerIPPorConsola("IP_MEMORIA");
	cfgKernel->PUERTO_MEMORIA = strdup(config_get_string_value(cfgFile, "PUERTO_MEMORIA"));
	cfgKernel->IP_CPU = obtenerIPPorConsola("IP_CPU");
	cfgKernel->PUERTO_CPU_DISPATCH = strdup(config_get_string_value(cfgFile, "PUERTO_CPU_DISPATCH"));
	cfgKernel->PUERTO_CPU_INTERRUPT = strdup(config_get_string_value(cfgFile, "PUERTO_CPU_INTERRUPT"));
	cfgKernel->ALGORITMO_PLANIFICACION = strdup(config_get_string_value(cfgFile, "ALGORITMO_PLANIFICACION"));
	cfgKernel->QUANTUM = config_get_int_value(cfgFile, "QUANTUM");
	cfgKernel->RECURSOS = config_get_array_value(cfgFile, "RECURSOS");
	cfgKernel->INSTANCIAS_RECURSOS = config_get_array_value(cfgFile, "INSTANCIAS_RECURSOS");
	cfgKernel->GRADO_MULTIPROGRAMACION = config_get_int_value(cfgFile, "GRADO_MULTIPROGRAMACION");

	log_info(auxLogger, "Archivo de configuracion cargado correctamente: ");
	imprimirConfigCargada();

	convertirInstancias();

	free(nombreCompleto);

	config_destroy(cfgFile);
}

int contarRecursos(char **recursos) {
    int count = 0;
    while (recursos[count] != NULL) {
        count++;
    }
    return count;
}

void convertirInstancias() {
	instanciasRecursos = malloc(contarRecursos(cfgKernel->INSTANCIAS_RECURSOS) * sizeof(int));
    if (instanciasRecursos == NULL) {
        perror("Error al asignar memoria para instancias");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < contarRecursos(cfgKernel->INSTANCIAS_RECURSOS); i++) {
    	instanciasRecursos[i] = atoi(cfgKernel->INSTANCIAS_RECURSOS[i]);
    }
}

void liberarConfig() {
	free(cfgKernel->PUERTO_ESCUCHA);
	free(cfgKernel->IP_MEMORIA);
	free(cfgKernel->PUERTO_MEMORIA);
	free(cfgKernel->IP_CPU);
	free(cfgKernel->PUERTO_CPU_DISPATCH);
	free(cfgKernel->PUERTO_CPU_INTERRUPT);
	free(cfgKernel->ALGORITMO_PLANIFICACION);
	free(cfgKernel->RECURSOS);
	free(cfgKernel->INSTANCIAS_RECURSOS);
	free(cfgKernel);
	if(instanciasRecursos!=NULL){
	free(instanciasRecursos);
	}
}
