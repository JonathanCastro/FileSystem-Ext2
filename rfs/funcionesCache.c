#include "libRFS.h"
#include <libmemcached/memcached.h>
#include <stdio.h>
#include <string.h>

void inicializarMemcached() {
	if (cache != 0) {
		extern memcached_st *memcached;
		memcached_server_st *servers = NULL;
		memcached_return resultado;

		memcached = memcached_create(NULL);
		servers = memcached_server_list_append(servers, ipCache, puertoCache,
				&resultado);
		resultado = memcached_server_push(memcached, servers);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Ahora esta conectado a Memcached\n");
		else
			printf("No se pudo conectar");
	}
}

char * armarKey(char operacion, const char *path) {
	if ((strlen(path) + 1) > 40) {
		printf("El path no puede tener mas de 40 caracteres.\n");
		exit(0);
	}
	char *key = malloc(sizeof(char) + strlen(path) + 1);
	uint32_t tamPath = strlen(path) + 1;

	memcpy(key, &operacion, sizeof(char));
	memcpy(key + sizeof(char), path, tamPath);
	return key;

}

void setearValor(char operacion, const char *path, char* buffer) {

	if (cache != 0) {
		extern memcached_st *memcached;
		extern pthread_mutex_t bloqueoCache;
		char *key = malloc(sizeof(char) + strlen(path) + 1);

		size_t tamkey = 0;
		size_t tamBuff = 0;
		memcached_return resultado;

		key = armarKey(operacion, path);

		tamkey = strlen(key);
		tamBuff = strlen(buffer);
		pthread_mutex_lock(&bloqueoCache);
		resultado = memcached_set(memcached, key, tamkey, buffer, tamBuff,
				(time_t) 0, (uint32_t) 0);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Se seteo %s en la Memcached\n", key);

		pthread_mutex_unlock(&bloqueoCache);

		free(key);
	}

}

char* getValor(char operacion, const char *path) {
	if (cache != 0) {
		extern memcached_st *memcached;
		extern pthread_mutex_t bloqueoCache;

		char *key = malloc(sizeof(char) + strlen(path) + 1);
		size_t tamKey = 0;
		size_t tamBuffer;
		uint32_t flags;
		char* buffer;
		memcached_return_t resultado;

		key = armarKey(operacion, path);

		tamKey = strlen(key);
		pthread_mutex_lock(&bloqueoCache);

		buffer = memcached_get(memcached, key, tamKey, &tamBuffer, &flags,
				&resultado);

		if (resultado == MEMCACHED_SUCCESS) {
			free(key);
			pthread_mutex_unlock(&bloqueoCache);

			return buffer;

		} else {
			free(key);
			pthread_mutex_unlock(&bloqueoCache);

			return NULL;
		}
	} else
		return NULL;

}

void eliminarValor(char operacion, const char *path) {

	if (cache != 0) {
		char *key = malloc(sizeof(char) + strlen(path) + 1);
		extern pthread_mutex_t bloqueoCache;

		extern memcached_st *memcached;
		size_t tamkey = 0;
		memcached_return resultado;

		key = armarKey(operacion, path);

		tamkey = strlen(key);
		pthread_mutex_lock(&bloqueoCache);

		resultado = memcached_delete(memcached, key, tamkey, (time_t) 0);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Se elimino %s en la Memcached\n", key);

		free(key);
		pthread_mutex_unlock(&bloqueoCache);


	}
}

char * armarKeyParaRW(int32_t nroBolque, const char *path) {

	if ((strlen(path) + 1) > 34) {
		printf(
				"El path no puede tener mas de 34 caracteres para una operacion de read o write.\n");
		exit(0);
	}

	char nroBloqueS[7];

	sprintf(nroBloqueS, "%d", nroBolque);

	char *key = malloc(sizeof(nroBloqueS) + strlen(path) + 1);

	strcpy(key, nroBloqueS);
	strcat(key, path);

	return key;

}

void setearValorRW(int32_t nroBolque, const char *path, char* buffer) {
	if (cache != 0) {
		extern int32_t TAM_BLOCK;
		extern pthread_mutex_t bloqueoCache;

		extern memcached_st *memcached;
		char *key = malloc(sizeof(int32_t) + strlen(path) + 1);

		size_t tamkey = 0;
		memcached_return resultado;

		key = armarKeyParaRW(nroBolque, path);

		tamkey = strlen(key);
		pthread_mutex_lock(&bloqueoCache);

		resultado = memcached_set(memcached, key, tamkey, buffer, TAM_BLOCK,
				(time_t) 0, (uint32_t) 0);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Se seteo %s en la Memcached\n", key);
		else
			printf("Error al setear %s en la Memcached\n", key);

		free(key);
		pthread_mutex_unlock(&bloqueoCache);

	}
}

char* getValorRW(int32_t nroBolque, const char *path) {
	if (cache != 0) {
		extern int32_t TAM_BLOCK;
		extern pthread_mutex_t bloqueoCache;

		extern memcached_st *memcached;
		char *key = malloc(sizeof(int32_t) + strlen(path) + 1);
		size_t tamKey = 0;
		size_t tamBuffer;
		uint32_t flags;
		char* buffer = (char*) calloc(TAM_BLOCK, sizeof(char));
		memcached_return_t resultado;

		key = armarKeyParaRW(nroBolque, path);

		tamKey = strlen(key);
		pthread_mutex_lock(&bloqueoCache);

		buffer = memcached_get(memcached, key, tamKey, &tamBuffer, &flags,
				&resultado);

		if (resultado == MEMCACHED_SUCCESS) {
			free(key);
			pthread_mutex_unlock(&bloqueoCache);
			return buffer;

		} else {
			free(key);
			free(buffer);
			pthread_mutex_unlock(&bloqueoCache);

			return NULL;
		}
	} else
		return NULL;
}

void liberarCache() {
	extern memcached_st *memcached;
	memcached_free(memcached);
}

char* buscarPathSinNombre(char* path, int32_t cantToke) {
	char *raiz = "/";
	int32_t tamPath = (strlen(path) + 1);
	char* pathQueloContiene = (char*) calloc(200, sizeof(char));
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra;
	memcpy(ruta, path, tamPath);

	int32_t offSet = 0;

	palabra = strtok(ruta, "/");

	while (palabra != NULL && cantToke > 0) {
		memcpy(pathQueloContiene + offSet, raiz, 1);
		offSet += 1;
		memcpy(pathQueloContiene + offSet, palabra, strlen(palabra));
		offSet += strlen(palabra);
		cantToke--;
		palabra = strtok(NULL, raiz);
	}

	pathQueloContiene = realloc(pathQueloContiene,
			strlen(pathQueloContiene) + 1);

	return pathQueloContiene;

}

char* obtenerpathQueLoContiene(char* path) { // le paso la var nombre para que me la complete
	char *raiz = "/";
	int32_t tamPath = (strlen(path) + 1);

	int32_t cantToken = 0;
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra;
	memcpy(ruta, path, tamPath);
	palabra = strtok(ruta, "/");

	while (palabra != NULL) {
		cantToken++;
		palabra = strtok(NULL, raiz);
	}
	cantToken--;

	if (cantToken != 0) {
		return buscarPathSinNombre(path, cantToken);
	} else {
		return "/";
	}
}

