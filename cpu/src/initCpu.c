#include "../include/initCpu.h"

#include "../include/cpu.h"

void imprimirConfigCargada() {
	printf("IP_MEMORIA: %s \n", cfgCpu->IP_MEMORIA);
	printf("PUERTO_MEMORIA: %s \n", cfgCpu->PUERTO_MEMORIA);
	printf("PUERTO_ESCUCHA_DISPATCH: %s \n", cfgCpu->PUERTO_ESCUCHA_DISPATCH);
	printf("PUERTO_ESCUCHA_INTERRUPT: %s \n", cfgCpu->PUERTO_ESCUCHA_INTERRUPT);
	printf("ALGORITMO_TLB: %s \n", cfgCpu->ALGORITMO_TLB);
	printf("CANTIDAD_ENTRADAS_TLB: %i \n", cfgCpu->CANTIDAD_ENTRADAS_TLB);
}

void inicializarConfigCpu(char* nombreConfig) {

	cfgCpu = malloc(sizeof(t_config_cpu));

	char* nombreCompleto = malloc(strlen("configs/") + strlen(nombreConfig) + strlen(".config") + 1);
	strcpy(nombreCompleto, "configs/");
	strcat(nombreCompleto, nombreConfig);
	strcat(nombreCompleto, ".config");

	t_config *cfgFile = config_create(nombreCompleto);

	if (cfgFile == NULL) {
		log_error(auxLogger, "No se encontro cpu.config");
		abort();
	}

	cfgCpu->IP_MEMORIA = obtenerIPPorConsola("IP_MEMORIA");
	cfgCpu->PUERTO_MEMORIA = strdup(config_get_string_value(cfgFile, "PUERTO_MEMORIA"));
	cfgCpu->PUERTO_ESCUCHA_DISPATCH = strdup(config_get_string_value(cfgFile, "PUERTO_ESCUCHA_DISPATCH"));
	cfgCpu->PUERTO_ESCUCHA_INTERRUPT = strdup(config_get_string_value(cfgFile, "PUERTO_ESCUCHA_INTERRUPT"));
	cfgCpu->CANTIDAD_ENTRADAS_TLB = config_get_int_value(cfgFile, "CANTIDAD_ENTRADAS_TLB");
	cfgCpu->ALGORITMO_TLB = strdup(config_get_string_value(cfgFile, "ALGORITMO_TLB"));

	log_info(auxLogger, "Archivo de configuracion cargado correctamente :");
	imprimirConfigCargada();

	free(nombreCompleto);

	config_destroy(cfgFile);
}

void liberarConfigCpu(){
	free(cfgCpu->IP_MEMORIA);
	free(cfgCpu->PUERTO_MEMORIA);
	free(cfgCpu->PUERTO_ESCUCHA_DISPATCH);
	free(cfgCpu->PUERTO_ESCUCHA_INTERRUPT);
	free(cfgCpu->ALGORITMO_TLB);
	free(cfgCpu);
}
