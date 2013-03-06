#include "libFSC.h"
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

		extern t_log *loguear;
		extern memcached_st *memcached;
		extern pthread_mutex_t bloquearCache;
		char *key = malloc(sizeof(char) + strlen(path) + 1);

		size_t tamkey = 0;
		size_t tamBuff = 0;
		memcached_return resultado;

		key = armarKey(operacion, path);

		tamkey = strlen(key);
		tamBuff = strlen(buffer);
		pthread_mutex_lock(&bloquearCache);

		resultado = memcached_set(memcached, key, tamkey, buffer, tamBuff,
				(time_t) 0, (uint32_t) 0);

		log_debug(loguear,
				"Solicitud de almacenamiento en cache para la key: %s", key);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Se seteo %s en la Memcached\n", key);

		free(key);
		pthread_mutex_unlock(&bloquearCache);
	}
}

char* getValor(char operacion, const char *path) {
	if (cache != 0) {
		extern t_log *loguear;
		extern memcached_st *memcached;
		extern pthread_mutex_t bloquearCache;

		char *key = malloc(sizeof(char) + strlen(path) + 1);
		size_t tamKey = 0;
		size_t tamBuffer;
		uint32_t flags;
		char* buffer;
		memcached_return_t resultado;

		key = armarKey(operacion, path);

		tamKey = strlen(key);
		pthread_mutex_lock(&bloquearCache);

		buffer = memcached_get(memcached, key, tamKey, &tamBuffer, &flags,
				&resultado);
		log_debug(loguear, "Consulta de un valor en cache con la key: %s", key);

		if (resultado == MEMCACHED_SUCCESS) {
			printf("Se encontro la key %s en la Memcached. Devolvio:  %s\n",
					key, buffer);
			free(key);
			pthread_mutex_unlock(&bloquearCache);

			return buffer;

		} else {
			free(key);
			pthread_mutex_unlock(&bloquearCache);

			return NULL;
		}
	} else {
		return NULL;
	}
}

void eliminarValor(char operacion, const char *path) {

	if (cache != 0) {
		extern pthread_mutex_t bloquearCache;

		char *key = malloc(sizeof(char) + strlen(path) + 1);
		extern memcached_st *memcached;
		size_t tamkey = 0;
		memcached_return resultado;

		key = armarKey(operacion, path);

		tamkey = strlen(key);
		pthread_mutex_lock(&bloquearCache);

		resultado = memcached_delete(memcached, key, tamkey, (time_t) 0);

		if (resultado == MEMCACHED_SUCCESS)
			printf("Se elimino %s en la Memcached\n", key);
		else
			printf("Error al eliminar %s en la Memcached\n", key);

		free(key);
		pthread_mutex_unlock(&bloquearCache);
	}
}

void liberarCache() {
	extern memcached_st *memcached;
	memcached_free(memcached);
}
