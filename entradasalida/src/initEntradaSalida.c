#include "../include/initEntradaSalida.h"
#include "../include/entradasalida.h"

void imprimirConfigCargada() {
	printf("IP_KERNEL: %s \n", cfgEntradaSalida->IP_KERNEL);
	printf("PUERTO_KERNEL: %s \n", cfgEntradaSalida->PUERTO_KERNEL);
	printf("IP_MEMORIA: %s \n", cfgEntradaSalida->IP_MEMORIA);
	printf("PUERTO_MEMORIA: %s \n", cfgEntradaSalida->PUERTO_MEMORIA);
	printf("PATH_BASE_DIALFS: %s \n", cfgEntradaSalida->PATH_BASE_DIALFS);
	printf("TIEMPO_UNIDAD_TRABAJO: %i \n",
			cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO);
	printf("BLOCK_SIZE: %i \n", cfgEntradaSalida->BLOCK_SIZE);
	printf("BLOCK_COUNT: %i \n", cfgEntradaSalida->BLOCK_COUNT);
	printf("RETRASO_COMPACTACION: %i \n", cfgEntradaSalida->RETRASO_COMPACTACION);
}

bool inicializarConfig(char* nombreConfig) {
	cfgEntradaSalida = malloc(sizeof(t_config_entradasalida));

	char* nombreCompleto = malloc(strlen("configs/") + strlen(nombreConfig) + strlen(".config") + 1);
	strcpy(nombreCompleto, "configs/");
	strcat(nombreCompleto, nombreConfig);
	strcat(nombreCompleto, ".config");

	t_config *cfgFile = config_create(nombreCompleto);

	if (cfgFile == NULL) {
		log_error(auxLogger, "No se encontro entradasalida.config");
		return 0;
	}

	char *tipoInterfaz = strdup(config_get_string_value(cfgFile, "TIPO_INTERFAZ"));
	cfgEntradaSalida->TIPO_INTERFAZ = tipoInterfaz;

	if (strcmp(tipoInterfaz, "GENERICA") == 0) {
		cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(cfgFile, "TIEMPO_UNIDAD_TRABAJO");
		cfgEntradaSalida->IP_KERNEL = obtenerIPPorConsola("IP_KERNEL");
		cfgEntradaSalida->PUERTO_KERNEL = strdup(config_get_string_value(cfgFile, "PUERTO_KERNEL"));
	} else if (strcmp(tipoInterfaz, "STDIN") == 0) {
		cfgEntradaSalida->IP_KERNEL = obtenerIPPorConsola("IP_KERNEL");
		cfgEntradaSalida->PUERTO_KERNEL = strdup(config_get_string_value(cfgFile, "PUERTO_KERNEL"));
		cfgEntradaSalida->IP_MEMORIA = obtenerIPPorConsola("IP_MEMORIA");
		cfgEntradaSalida->PUERTO_MEMORIA = strdup(config_get_string_value(cfgFile, "PUERTO_MEMORIA"));
	} else if (strcmp(tipoInterfaz, "STDOUT") == 0) {
		cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(cfgFile, "TIEMPO_UNIDAD_TRABAJO");
		cfgEntradaSalida->IP_KERNEL = obtenerIPPorConsola("IP_KERNEL");
		cfgEntradaSalida->PUERTO_KERNEL = strdup(config_get_string_value(cfgFile, "PUERTO_KERNEL"));
		cfgEntradaSalida->IP_MEMORIA = obtenerIPPorConsola("IP_MEMORIA");
		cfgEntradaSalida->PUERTO_MEMORIA = strdup(config_get_string_value(cfgFile, "PUERTO_MEMORIA"));
	} else if (strcmp(tipoInterfaz, "DIALFS") == 0) {
		cfgEntradaSalida->TIEMPO_UNIDAD_TRABAJO = config_get_int_value(cfgFile, "TIEMPO_UNIDAD_TRABAJO");
		cfgEntradaSalida->IP_KERNEL = obtenerIPPorConsola("IP_KERNEL");
		cfgEntradaSalida->PUERTO_KERNEL = strdup(config_get_string_value(cfgFile, "PUERTO_KERNEL"));
		cfgEntradaSalida->IP_MEMORIA = obtenerIPPorConsola("IP_MEMORIA");
		cfgEntradaSalida->PUERTO_MEMORIA = strdup(config_get_string_value(cfgFile, "PUERTO_MEMORIA"));
		cfgEntradaSalida->PATH_BASE_DIALFS = strdup(config_get_string_value(cfgFile, "PATH_BASE_DIALFS"));
		cfgEntradaSalida->BLOCK_SIZE = config_get_int_value(cfgFile, "BLOCK_SIZE");
		cfgEntradaSalida->BLOCK_COUNT = config_get_int_value(cfgFile, "BLOCK_COUNT");
		cfgEntradaSalida->RETRASO_COMPACTACION = config_get_int_value(cfgFile, "RETRASO_COMPACTACION");
		inicializarFileSystem();
	} else {
		//no hace nada
	}

	log_info(auxLogger, "Archivo de configuracion cargado correctamente");
	imprimirConfigCargada();

	free(nombreCompleto);

	config_destroy(cfgFile);

	return 1;
}

void liberarConfig() {
	free(cfgEntradaSalida->TIPO_INTERFAZ);
	free(cfgEntradaSalida->IP_KERNEL);
	free(cfgEntradaSalida->PUERTO_KERNEL);
	free(cfgEntradaSalida->IP_MEMORIA);
	free(cfgEntradaSalida->PUERTO_MEMORIA);
	free(cfgEntradaSalida->PATH_BASE_DIALFS);
	free(cfgEntradaSalida);
}
