#include "../include/entradasalida.h"

t_list *mapeoBloques; // (t_bloque*)
t_list *mapeoArchivos; // (t_metadata*)
char *pathArchivoBloques; // (void*)
char *pathArchivoBitMap; // (t_bitarray)

void freeBitarray(t_bitarray* bitarr){
	if(bitarr!=NULL){
		if(bitarr->bitarray != NULL){
			free(bitarr->bitarray);
		}
		free(bitarr);
	}
}

char* obtenerPathCompleto(char *path) {
	size_t longitudPath = strlen(path);
	size_t longitudPathBase = strlen(cfgEntradaSalida->PATH_BASE_DIALFS);
	char *pathCompleto = malloc(longitudPath + longitudPathBase + 1);
	strcpy(pathCompleto, cfgEntradaSalida->PATH_BASE_DIALFS);
	strcat(pathCompleto, path);
	return pathCompleto;
}

bool existeArchivo(char *path) {
	return access(path, F_OK) != -1;
}

FILE* descubrirArchivo(char *path, int cantidadBytes) {
	FILE *archivo;
	if (existeArchivo(path)) {
		archivo = fopen(path, "r+");
	} else {
		archivo = fopen(path, "w+");
		fseek(archivo, 0, SEEK_SET);
		for (int i = 0; i < cantidadBytes; i++) {
			char byteCeros = 0;
			fwrite(&byteCeros, sizeof(char), 1, archivo);
		}
	}
	return archivo;
}

t_bloque* newBloque(int idBloque, bool ocupado) {
	t_bloque *bloque = malloc(sizeof(t_bloque));
	bloque->idBloque = idBloque;
	bloque->ocupado = ocupado;
	return bloque;
}

bool esArchivoMetadata(char *dirName) {
	return strcmp(dirName, ".") != 0 && strcmp(dirName, "..") != 0 && strcmp(dirName, "bloques.dat") != 0
			&& strcmp(dirName, "bitmap.dat") != 0;
}

t_metadata* newMetadata(char *nombreArchivo, int primerBloque, int cantidadBytes) {
	t_metadata *metadata = malloc(sizeof(t_metadata));
	metadata->nombreArchivo = malloc(strlen(nombreArchivo) + 1);
	strcpy(metadata->nombreArchivo, nombreArchivo);
	metadata->primerBloque = primerBloque;
	metadata->cantidadBytes = cantidadBytes;
	if (cantidadBytes == 0) {
		metadata->cantidadBloques = 1;
	} else {
		metadata->cantidadBloques = ceil((double) cantidadBytes / cfgEntradaSalida->BLOCK_SIZE);
	}
	return metadata;
}

void levantarDatos(FILE *file, int *primerBloque, int *cantidadBytes, char *nombreArchivo) {
	fseek(file, 0, SEEK_SET);
	char *lineaMetadata;
	size_t size;

	while (!feof(file)) {
		lineaMetadata = NULL;
		size = 0;
		getline(&lineaMetadata, &size, file);
		char *campo = strtok(lineaMetadata, "=");
		if (strcmp(campo, "BLOQUE_INICIAL") == 0) {
			char *dato = strtok(NULL, "\n");
			*primerBloque = strtol(dato, NULL, 10);
		} else if (strcmp(campo, "TAMANIO_ARCHIVO") == 0) {
			char *dato = strtok(NULL, "\n");
			*cantidadBytes = strtol(dato, NULL, 10);
		} else {
			log_error(auxLogger, "Archivo de metadata inválido: %s", nombreArchivo);
			abort();
		}
	}
}

void levantarMetadata() {
	struct dirent *entry;
	DIR *dir = opendir(cfgEntradaSalida->PATH_BASE_DIALFS);

	while ((entry = readdir(dir)) != NULL) {
		char *dirName = malloc(strlen(entry->d_name) + 1);
		strcpy(dirName, entry->d_name);

		if (esArchivoMetadata(dirName)) {
			char *path = obtenerPathCompleto(dirName);
			FILE *file = fopen(path, "r");
			int primerBloque = 0, cantidadBytes = 0;
			levantarDatos(file, &primerBloque, &cantidadBytes, dirName);
			t_metadata *metadata = newMetadata(dirName, primerBloque, cantidadBytes);
			list_add(mapeoArchivos, metadata);
			fclose(file);
			free(path);
		}
		free(dirName);
	}
	closedir(dir);
}

t_bitarray* levantarBitMap(FILE *file) {
	char *bitsBuffer = malloc(ceil((double) cfgEntradaSalida->BLOCK_COUNT / 8));
	fseek(file, 0, SEEK_SET);
	fread(bitsBuffer, sizeof(char), cfgEntradaSalida->BLOCK_COUNT, file);
	t_bitarray *bitarray = bitarray_create_with_mode(bitsBuffer, cfgEntradaSalida->BLOCK_COUNT, MSB_FIRST);
	return bitarray;
}

void mapearBloques(FILE *archivoBitMap) {
	t_bitarray *bitMap = levantarBitMap(archivoBitMap);
	for (int i = 0; i < cfgEntradaSalida->BLOCK_COUNT; i++) {
		list_add(mapeoBloques, newBloque(i, bitarray_test_bit(bitMap, i)));
	}
	freeBitarray(bitMap);
}

void inicializarFileSystem() {
	pathArchivoBloques = obtenerPathCompleto("bloques.dat");
	pathArchivoBitMap = obtenerPathCompleto("bitmap.dat");
	mapeoBloques = list_create();
	mapeoArchivos = list_create();

	FILE *archivoBloques = descubrirArchivo(pathArchivoBloques, cfgEntradaSalida->BLOCK_COUNT * cfgEntradaSalida->BLOCK_SIZE);
	FILE *archivoBitMap = descubrirArchivo(pathArchivoBitMap, ceil((double) cfgEntradaSalida->BLOCK_COUNT / 8));

	mapearBloques(archivoBitMap);

	levantarMetadata();

	fclose(archivoBloques);
	fclose(archivoBitMap);
}

bool estaLibre(t_bloque *bloque) {
	return !(bloque->ocupado);
}

bool estaOcupadoYLiberarLibres(t_bloque *bloque) {
	if (estaLibre(bloque)) {
		free(bloque);
		return false;
	} else {
		return true;
	}
}

char* crearNuevaMetadata(int primerBloque, int cantidadBytes) {
	const int metadataSize = strlen("BLOQUE_INICIAL=") + strlen("\nTAMANIO_ARCHIVO=") + 21;
	char *buffer = malloc(metadataSize);
	snprintf(buffer, metadataSize, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d", primerBloque, cantidadBytes);
	return buffer;
}

void subirBitMap(FILE *archivoBitMap, t_bitarray *bitMap) {
	fseek(archivoBitMap, 0, SEEK_SET);
	fwrite(bitMap->bitarray, sizeof(char), ceil((double) cfgEntradaSalida->BLOCK_COUNT / 8), archivoBitMap);
	freeBitarray(bitMap);
}

void actualizarBitMapYMapeo(t_bloque *bloqueLibre) {
	FILE *archivoBitMap = fopen(pathArchivoBitMap, "r+");
	t_bitarray *bitMap = levantarBitMap(archivoBitMap);
	bitarray_set_bit(bitMap, bloqueLibre->idBloque);
	bloqueLibre->ocupado = 1;
	subirBitMap(archivoBitMap, bitMap);
	fclose(archivoBitMap);
}

void crearYMapearNuevaMetadata(char *nombreArchivo, t_bloque *bloqueLibre) {
	t_metadata *nuevaMetadata = newMetadata(nombreArchivo, bloqueLibre->idBloque, 0);
	char *pathNuevaMetadata = obtenerPathCompleto(nombreArchivo);
	FILE *archivoNuevaMetadata = fopen(pathNuevaMetadata, "w+");
	char *metadataInfo = crearNuevaMetadata(nuevaMetadata->primerBloque, nuevaMetadata->cantidadBytes);
	fseek(archivoNuevaMetadata, 0, SEEK_SET);
	fwrite(metadataInfo, sizeof(char), strlen(metadataInfo), archivoNuevaMetadata);
	list_add(mapeoArchivos, nuevaMetadata);
	fclose(archivoNuevaMetadata);
	free(metadataInfo);
	free(pathNuevaMetadata);
}

void crearArchivo(char *nombreArchivo) {
	t_bloque *bloqueLibre = list_find(mapeoBloques, (void*) estaLibre);
	actualizarBitMapYMapeo(bloqueLibre);
	crearYMapearNuevaMetadata(nombreArchivo, bloqueLibre);
}

t_metadata* obtenerArchivoSegunNombre(char *nombreArchivo) {
	t_list_iterator *iterator = list_iterator_create(mapeoArchivos);
	while (list_iterator_has_next(iterator)) {
		t_metadata *metadata = list_iterator_next(iterator);
		if (strcmp(metadata->nombreArchivo, nombreArchivo) == 0) {
			list_iterator_destroy(iterator);
			return metadata;
		}
	}
	list_iterator_destroy(iterator);
	return NULL;
}

t_metadata* obtenerArchivoSegunPrimerBloque(int primerBloque) {
	t_list_iterator *iterator = list_iterator_create(mapeoArchivos);
	while (list_iterator_has_next(iterator)) {
		t_metadata *metadata = list_iterator_next(iterator);
		if (metadata->primerBloque == primerBloque) {
			list_iterator_destroy(iterator);
			return metadata;
		}
	}
	list_iterator_destroy(iterator);
	return NULL;
}

void desocuparBloques(int primerBloque, int cantidadBloques) {
	FILE *archivoBitMap = fopen(pathArchivoBitMap, "r+");
	t_bitarray *bitMap = levantarBitMap(archivoBitMap);

	t_list *bloquesAEliminar = list_slice(mapeoBloques, primerBloque, cantidadBloques);
	t_list_iterator *iterator = list_iterator_create(bloquesAEliminar);
	while (list_iterator_has_next(iterator)) {
		t_bloque *bloque = list_iterator_next(iterator);
		bloque->ocupado = 0;
		bitarray_clean_bit(bitMap, bloque->idBloque);
	}
	list_destroy(bloquesAEliminar);
	list_iterator_destroy(iterator);
	subirBitMap(archivoBitMap, bitMap);
	fclose(archivoBitMap);
}

void eliminarMapeoYMetadata(t_metadata *archivo) {
	char *pathMetadata = obtenerPathCompleto(archivo->nombreArchivo);
	remove(pathMetadata);
	list_remove_element(mapeoArchivos, archivo);
	free(pathMetadata);
}

void eliminarArchivo(char *nombreArchivo) {
	t_metadata *archivo = obtenerArchivoSegunNombre(nombreArchivo);
	desocuparBloques(archivo->primerBloque, archivo->cantidadBloques);
	eliminarMapeoYMetadata(archivo);
	free(archivo->nombreArchivo);
	free(archivo);
}

void accederFileSystem(char *nombreArchivo, int tamanioBuffer, void *buffer, int punteroArchivo, bool modoAcceso) {
	if (modoAcceso) {
		buffer = realloc(buffer, tamanioBuffer + 1);
		memset(buffer, 0, tamanioBuffer + 1);
	}
	t_metadata *archivo = obtenerArchivoSegunNombre(nombreArchivo);
	int cantidadAAcceder =
			tamanioBuffer > (archivo->cantidadBytes - punteroArchivo) ? archivo->cantidadBytes - punteroArchivo : tamanioBuffer;
	int punteroReal = punteroArchivo + archivo->primerBloque * cfgEntradaSalida->BLOCK_SIZE;
	FILE *archivoBloques = fopen(pathArchivoBloques, "r+");
	fseek(archivoBloques, punteroReal, SEEK_SET);
	if (modoAcceso) {
		void *bufferAux = malloc(cantidadAAcceder);
		fread(bufferAux, 1, cantidadAAcceder, archivoBloques);
		memcpy(buffer, bufferAux, cantidadAAcceder);
		free(bufferAux);
	} else {
		fwrite(buffer, 1, cantidadAAcceder, archivoBloques);
	}
	fclose(archivoBloques);
}

void leerDeFileSystem(char *nombreArchivo, int tamanioBuffer, void *buffer, int punteroArchivo) {
	const bool MODO_LECTURA = 1;
	return accederFileSystem(nombreArchivo, tamanioBuffer, buffer, punteroArchivo, MODO_LECTURA);
}

void escribirEnFileSystem(char *nombreArchivo, int tamanioBuffer, void *buffer, int punteroArchivo) {
	const bool MODO_ESCRITURA = 0;
	return accederFileSystem(nombreArchivo, tamanioBuffer, buffer, punteroArchivo, MODO_ESCRITURA);
}

void actualizarMetadata(t_metadata *archivo) {
	char *pathMetadata = obtenerPathCompleto(archivo->nombreArchivo);
	FILE *archivoMetadata = fopen(pathMetadata, "w+");
	char *nuevaMetadata = crearNuevaMetadata(archivo->primerBloque, archivo->cantidadBytes);
	fseek(archivoMetadata, 0, SEEK_SET);
	fwrite(nuevaMetadata, sizeof(char), strlen(nuevaMetadata), archivoMetadata);
	fclose(archivoMetadata);
	free(pathMetadata);
	free(nuevaMetadata);
}

void reduccion(t_metadata *archivo, int bloquesTruncado, int bytesTruncado) {
	int cantidadBloquesAEliminar = archivo->cantidadBloques - bloquesTruncado;
	int primerBloqueAEliminar = archivo->primerBloque + archivo->cantidadBloques - cantidadBloquesAEliminar;
	desocuparBloques(primerBloqueAEliminar, cantidadBloquesAEliminar);
	archivo->cantidadBytes = bytesTruncado;
	archivo->cantidadBloques = bloquesTruncado;
	actualizarMetadata(archivo);
}

void ocuparBloques(t_list *bloquesContiguosLibres) {
	FILE *archivoBitMap = fopen(pathArchivoBitMap, "r+");
	t_bitarray *bitMap = levantarBitMap(archivoBitMap);
	t_list_iterator *iterator = list_iterator_create(bloquesContiguosLibres);
	while (list_iterator_has_next(iterator)) {
		t_bloque *bloque = list_iterator_next(iterator);
		bloque->ocupado = 1;
		bitarray_set_bit(bitMap, bloque->idBloque);
	}
	list_destroy(bloquesContiguosLibres);
	list_iterator_destroy(iterator);
	subirBitMap(archivoBitMap, bitMap);
	fclose(archivoBitMap);
}

void rellenarArchivoBloquesActualizandoBitMap(int cantidadBloquesCompactados, t_list *bloquesCompactados, t_bitarray *bitMap) {
	for (int i = 0; i < cfgEntradaSalida->BLOCK_COUNT - cantidadBloquesCompactados; i++) {
		int posicion = i + cantidadBloquesCompactados;
		t_bloque *bloqueVacio = newBloque(posicion, false);
		list_add(bloquesCompactados, bloqueVacio);
		bitarray_clean_bit(bitMap, posicion);
	}
}

void agregarBloquesTruncadoActualizandoBitMap(int bloquesTruncado, int cantidadBloquesCompactados, t_list *bloquesCompactados,
		t_bitarray *bitMap) {
	for (int i = 0; i < bloquesTruncado; i++) {
		int posicion = i + cantidadBloquesCompactados;
		t_bloque *bloqueAAgregar = newBloque(posicion, true);
		list_add(bloquesCompactados, bloqueAAgregar);
		bitarray_set_bit(bitMap, posicion);
	}
}

void actualizarMapeoBloques(t_list *bloquesCompactados) {
	list_destroy(mapeoBloques); // No destruyo ademas los elementos porque bloquesCompactados tiene bloques de mapeoBloques?
	mapeoBloques = bloquesCompactados;
}

void reubicarArchivoTruncado(int cantidadBloquesCompactados, FILE *archivoBloques, void *dataArchivo, t_metadata *archivoATruncar) {
	fseek(archivoBloques, cantidadBloquesCompactados * cfgEntradaSalida->BLOCK_SIZE, SEEK_SET);
	fwrite(dataArchivo, 1, archivoATruncar->cantidadBytes, archivoBloques);
	archivoATruncar->primerBloque = cantidadBloquesCompactados;
}

void reubicarBloquesCompactados(t_list *bloquesCompactados, FILE *archivoBloques, t_bitarray *bitMap) {
	t_list_iterator *iteratorBloquesCompactados = list_iterator_create(bloquesCompactados);
	void *buffer = malloc(cfgEntradaSalida->BLOCK_SIZE);
	while (list_iterator_has_next(iteratorBloquesCompactados)) {
		t_bloque *bloque = list_iterator_next(iteratorBloquesCompactados);
		int nuevaPosicion = list_iterator_index(iteratorBloquesCompactados);
		int anteriorPosicion = bloque->idBloque;
		fseek(archivoBloques, anteriorPosicion * cfgEntradaSalida->BLOCK_SIZE, SEEK_SET);
		fread(buffer, 1, cfgEntradaSalida->BLOCK_SIZE, archivoBloques);
		fseek(archivoBloques, nuevaPosicion * cfgEntradaSalida->BLOCK_SIZE, SEEK_SET);
		fwrite(buffer, 1, cfgEntradaSalida->BLOCK_SIZE, archivoBloques);
		t_metadata *archivo = obtenerArchivoSegunPrimerBloque(anteriorPosicion);
		if (archivo != NULL) {
			archivo->primerBloque = nuevaPosicion;
			actualizarMetadata(archivo);
		}
		bloque->idBloque = nuevaPosicion;
		bitarray_set_bit(bitMap, nuevaPosicion);
	}
	free(buffer);
	list_iterator_destroy(iteratorBloquesCompactados);
}

void compactar(t_metadata *archivo, int bloquesTruncado, int PID) {
	log_info(auxLogger, "PID: %i - Inicio Compactación.", PID);
	t_metadata *archivoATruncar = obtenerArchivoSegunNombre(archivo->nombreArchivo);

	void *dataArchivo = malloc(archivo->cantidadBytes);
	leerDeFileSystem(archivo->nombreArchivo, archivo->cantidadBytes, dataArchivo, 0);

	desocuparBloques(archivo->primerBloque, archivo->cantidadBloques);

	t_list *bloquesCompactados = list_filter(mapeoBloques, (void*) estaOcupadoYLiberarLibres);

	FILE *archivoBloques = fopen(pathArchivoBloques, "r+");
	FILE *archivoBitMap = fopen(pathArchivoBitMap, "r+");
	t_bitarray *bitMap = levantarBitMap(archivoBitMap);

	reubicarBloquesCompactados(bloquesCompactados, archivoBloques, bitMap);

	int cantidadBloquesCompactados = list_size(bloquesCompactados);
	reubicarArchivoTruncado(cantidadBloquesCompactados, archivoBloques, dataArchivo, archivoATruncar);

	// Esta funcion tambien actualiza el bitmap
	agregarBloquesTruncadoActualizandoBitMap(bloquesTruncado, cantidadBloquesCompactados, bloquesCompactados, bitMap);

	cantidadBloquesCompactados += bloquesTruncado;

	// Esta funcion tambien actualiza el bitmap
	rellenarArchivoBloquesActualizandoBitMap(cantidadBloquesCompactados, bloquesCompactados, bitMap);

	subirBitMap(archivoBitMap, bitMap);

	fclose(archivoBitMap);
	fclose(archivoBloques);

	actualizarMapeoBloques(bloquesCompactados);

}

void esperarRetrasoCompactacion() {
	useconds_t retrasoCompactacion = 1000 * (cfgEntradaSalida->RETRASO_COMPACTACION);
	usleep(retrasoCompactacion);
}

void ampliacion(t_metadata *archivo, int bloquesTruncado, int bytesTruncado, int PID) {
	int cantidadBloquesAAgregar = bloquesTruncado - archivo->cantidadBloques;
	t_list *bloquesContiguosLibres = list_slice(mapeoBloques, archivo->primerBloque + archivo->cantidadBloques, cantidadBloquesAAgregar);

	if (list_all_satisfy(bloquesContiguosLibres, (void*) estaLibre)) {
		ocuparBloques(bloquesContiguosLibres);
	} else {
		compactar(archivo, bloquesTruncado, PID);
		list_destroy(bloquesContiguosLibres);
		esperarRetrasoCompactacion();
		log_info(auxLogger, "PID: %i - Fin Compactación.", PID);
	}
	archivo->cantidadBytes = bytesTruncado;
	archivo->cantidadBloques = bloquesTruncado;
	actualizarMetadata(archivo);

}

void truncarArchivo(char *nombreArchivo, int bytesTruncado, int PID) {

	t_metadata *archivo = obtenerArchivoSegunNombre(nombreArchivo);
	int bloquesTruncado = ceil((double) bytesTruncado / cfgEntradaSalida->BLOCK_SIZE);
	if (bloquesTruncado > archivo->cantidadBloques) {
		ampliacion(archivo, bloquesTruncado, bytesTruncado, PID);
	} else if (bloquesTruncado < archivo->cantidadBloques) {
		reduccion(archivo, bloquesTruncado, bytesTruncado);
	} else {
		// no hacer nada
	}
}

void liberarFS() {
	free(pathArchivoBitMap);
	free(pathArchivoBloques);
}
