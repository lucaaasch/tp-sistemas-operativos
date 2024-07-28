#include "../include/memoria.h"

void imprimirConfigCargada() {
	printf("PUERTO_ESCUCHA: %s \n", cfgMemoria->PUERTO_ESCUCHA);
	printf("PATH_INSTRUCCIONES: %s \n", cfgMemoria->PATH_INSTRUCCIONES);
	printf("TAM_MEMORIA: %i \n", cfgMemoria->TAM_MEMORIA);
	printf("TAM_PAGINA: %i \n", cfgMemoria->TAM_PAGINA);
	printf("RETARDO_RESPUESTA: %i \n", cfgMemoria->RETARDO_RESPUESTA);
}

void inicializarConfigMemoria(char* nombreConfig) {

	cfgMemoria = malloc(sizeof(t_config_memoria));

	char* nombreCompleto = malloc(strlen("configs/") + strlen(nombreConfig) + strlen(".config") + 1);
	strcpy(nombreCompleto, "configs/");
	strcat(nombreCompleto, nombreConfig);
	strcat(nombreCompleto, ".config");

	t_config *cfgFile = config_create(nombreCompleto);

	if (cfgFile == NULL) {
		log_error(auxLogger, "No se encontro el archivo de configuracion");
		abort();
	}

	cfgMemoria->PUERTO_ESCUCHA = strdup(config_get_string_value(cfgFile, "PUERTO_ESCUCHA"));
	cfgMemoria->PATH_INSTRUCCIONES = strdup(config_get_string_value(cfgFile, "PATH_INSTRUCCIONES"));
	cfgMemoria->TAM_MEMORIA = config_get_int_value(cfgFile, "TAM_MEMORIA");
	cfgMemoria->TAM_PAGINA = config_get_int_value(cfgFile, "TAM_PAGINA");
	cfgMemoria->RETARDO_RESPUESTA = config_get_int_value(cfgFile, "RETARDO_RESPUESTA");

	log_info(auxLogger, "Archivo de configuracion cargado correctamente:");
	imprimirConfigCargada();

	free(nombreCompleto);

	config_destroy(cfgFile);

}

void liberarConfigMemoria() {
	free(cfgMemoria->PUERTO_ESCUCHA);
	free(cfgMemoria->PATH_INSTRUCCIONES);
	free(cfgMemoria);
}

