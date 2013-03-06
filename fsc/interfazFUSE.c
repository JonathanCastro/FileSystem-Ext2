#include "libFSC.h"

int32_t fsc_getattr(const char *path, struct stat *stbuf) {
	extern t_log *loguear;
	memset(stbuf, 0, sizeof(struct stat));

	printf("Funcion getAttr \n");
	printf("path :%s \n", path);

	if ((strcmp(path, "/.Trash") == 0) || (strcmp(path, "/.Trash-1000") == 0)
			|| (strcmp(path, "/.xdg-volume-info") == 0)
			|| (strcmp(path, "/autorun.inf") == 0)
			|| (strcmp(path, "/autorun.inf") == 0)
			|| (strcmp(path, "/lost") == 0)) {

		return -ENOENT;
	}

	char *buff = getValor('1', path);

	// Consulto en la cache.

	getAttrCache *paqueteCache = (getAttrCache *) malloc(sizeof(getAttrCache));

	if (buff == NULL) {

		NIPC respuesta;
		respuesta = enviarOperacion(1, (char*) path, 0, 0, NULL, 0);

		if (respuesta.modo != 0) {

			stbuf->st_mode = respuesta.modo;
			stbuf->st_size = respuesta.size;
			stbuf->st_nlink = respuesta.offset;

			paqueteCache->mode = respuesta.modo;
			paqueteCache->size = respuesta.size;
			paqueteCache->links = respuesta.offset;
			buff = serializarGetAttrCache(paqueteCache);
			setearValor('1', path, buff);

			char* nombreDeArchivo = dameElNombre((char*) path);
			log_debug(loguear, " Nombre Be Archivo: %s- Operacion getattr",
					nombreDeArchivo);

			free(buff);
			return 0;

		} else {
			char* nombreDeArchivo = dameElNombre((char*) path);
			log_debug(loguear, " Nombre Be Archivo: %s- Operacion getattr",
					nombreDeArchivo);

			return -ENOENT;
		}
	} else {
		deserializarGetAttrCache(buff, paqueteCache);
		stbuf->st_mode = paqueteCache->mode;
		stbuf->st_size = paqueteCache->size;
		stbuf->st_nlink = paqueteCache->links;
		char* nombreDeArchivo = dameElNombre((char*) path);
		log_debug(loguear, " Nombre Be Archivo: %s- Operacion getattr",
				nombreDeArchivo);

		return 0;

	}

}

int32_t fsc_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		off_t offset, struct fuse_file_info *fi) {
	extern t_log *loguear;
	(void) offset;
	(void) fi;

	printf("Funcion readdir \n");
	printf("path :%s \n", path);

	char *buff = getValor('2', path); // Consulto en la cache.
	if (buff == NULL) {
		NIPC respuesta;
		respuesta = enviarOperacion(2, (char*) path, 0, 0, NULL, 0);

		if (respuesta.payload != NULL) {
			char *raiz = "/";
			uint32_t tamPath = (respuesta.PayloadLength + 1);
			char ruta[tamPath];
			memset(ruta, 0, tamPath);
			char* palabra;
			memcpy(ruta, respuesta.payload, tamPath);

			palabra = strtok(ruta, "/");

			while (palabra != NULL) {

				filler(buf, (char*) palabra, NULL, 0);
				printf("%s\n", palabra);
				palabra = strtok(NULL, raiz);
			}
			setearValor('2', path, respuesta.payload);
			char* nombreDeArchivo = dameElNombre((char*) path);
			log_debug(loguear, " Nombre De Archivo: %s- Operacion readdir",
					nombreDeArchivo);


			chauNIPC(&respuesta);
			return 0;

		} else {
			char* nombreDeArchivo = dameElNombre((char*) path);
			log_debug(loguear, " Nombre De Archivo: %s- Operacion readdir",
					nombreDeArchivo);

			return -ENOENT;
		}

	} else {
		char *raiz = "/";
		uint32_t tamPath = (strlen(buff) + 1);
		char ruta[tamPath];
		memset(ruta, 0, tamPath);
		char* palabra;
		memcpy(ruta, buff, tamPath);

		palabra = strtok(ruta, "/");

		while (palabra != NULL) {
			filler(buf, palabra, NULL, 0);
			palabra = strtok(NULL, raiz);
		}
		char* nombreDeArchivo = dameElNombre((char*) path);
		log_debug(loguear, " Nombre De Archivo: %s- Operacion readdir",
				nombreDeArchivo);

		return 0;

	}
}

int32_t fsc_mkdir(const char *path, mode_t mode) {
	printf("Hola  mkdir \n");
	extern t_log *loguear;
	NIPC respuesta;
	respuesta = enviarOperacion(11, (char*) path, 0, 0, NULL, (int32_t) mode);

	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, "Nombre De Archivo:%s- Operacion mkdir",
			nombreDeArchivo);

	switch (respuesta.size) {
	case 1:
		return 0;
	case 2:
		return -EEXIST;
	case 3:
		return -ENOENT;
	case 4:
		return -ENOSPC;
	default:
		return -EINVAL;

	}
}

int32_t fsc_truncate(const char *path, off_t size) {
	printf("Hola  truncate \n");
	extern t_log *loguear;
	NIPC respuesta;
	respuesta = enviarOperacion(9, (char*) path, 0, size, NULL, 0);
	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion truncate",
			nombreDeArchivo);

	switch (respuesta.size) {
	case 1:
		return 0;
	case 2:
		return -EEXIST;
	case 3:
		return -ENOENT;
	case 4:
		return -ENOSPC;
	default:
		return -EINVAL;

	}

}

int32_t fsc_open(const char *path, struct fuse_file_info *fi) {
	extern t_log *loguear;
	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, " Nombre De Archivo:%s- Operacion open",
			nombreDeArchivo);


	return 0;


}

int32_t fsc_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	extern t_log *loguear;

	printf("Funcion  create \n");
	NIPC respuesta;
	respuesta = enviarOperacion(7, (char*) path, 0, 0, NULL, (int32_t) mode);
	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion create",
			nombreDeArchivo);


	switch (respuesta.size) {
	case 2:
		return -EEXIST;
	case 3:
		return -ENOENT;
	case 4:
		return -ENOSPC;
	default:
		fi->fh = respuesta.size;
		return 0;
	}
}

int32_t fsc_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {
	extern t_log *loguear;
	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear,
			"Nombre De Archivo: %s- Operacion read- Size: %d- Offset: %d",
			nombreDeArchivo, size, offset);

	printf("Funcion read\n");
	NIPC respuesta;
	respuesta = enviarOperacion(5, (char*) path, offset, size, NULL, 0);
	if (respuesta.offset == 1) {
		memset(buf, 0, size);
		return respuesta.PayloadLength;
	}
	if (respuesta.payload != NULL) {

		memset(buf, 0, size);
		memcpy(buf, respuesta.payload, respuesta.PayloadLength);
		chauNIPC(&respuesta);
		return respuesta.PayloadLength;

	} else {

		switch (respuesta.size) {
		case 2:
			return -EEXIST;
		case 3:
			return -ENOENT;
		case 4:
			return -ENOSPC;
		default:
			return -EINVAL;

		}

	}

}

int32_t fsc_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi) {

	extern t_log *loguear;
	printf("Funcion write \n");
	char *buffer = malloc(strlen(path) + size +1);
	memcpy(buffer, path, strlen(path));
	memcpy(buffer + strlen(path), "|", sizeof(char));
	memcpy(buffer + strlen(path) + sizeof(char), buf, size);

	NIPC respuesta;
	respuesta = enviarOperacion(6, (char*) path, offset, size, buffer, 0);
	free(buffer);
	if (respuesta.offset == 1) {
		char* nombreDeArchivo = dameElNombre((char*) path);
		log_debug(loguear,
				"Nombre De Archivo: %s- Operacion write- Size: %d- Offset: %d",
				nombreDeArchivo, size, offset);

		return respuesta.size;
	} else {
		char* nombreDeArchivo = dameElNombre((char*) path);
		log_debug(loguear,
				"Nombre De Archivo: %s- Operacion write- Size: %d- Offset: %d",
				nombreDeArchivo, size, offset);

		switch (respuesta.size) {
		case 2:
			return -EEXIST;
		case 3:
			return -ENOENT;
		case 4:
			return -ENOSPC;
		default:
			return -EINVAL;

		}
	}

}

int32_t fsc_release(const char *path, struct fuse_file_info *fi) {
	return 0;

}
int32_t fsc_unlink(const char *path) {

	extern t_log *loguear;
	printf("Funcion  unlink \n");
	NIPC respuesta;
	respuesta = enviarOperacion(8, (char*) path, 0, 0, NULL, 0);
	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion unlink",
			nombreDeArchivo);

	switch (respuesta.size) {
	case 1:
		return 0;
	case 2:
		return -EEXIST;
	case 3:
		return -ENOENT;
	case 4:
		return -ENOSPC;
	default:
		return -EINVAL;

	}

}

// Si es rm -R Fuse llama a Rmdir Recursivamente SOLO
int32_t fsc_rmdir(const char *path) {

	extern t_log *loguear;
	printf("Funcion  rmdir\n");
	NIPC respuesta;
	respuesta = enviarOperacion(10, (char*) path, 0, 0, NULL, 0);

	char* nombreDeArchivo = dameElNombre((char*) path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion rmdir",
			nombreDeArchivo);

	switch (respuesta.size) {
	case 1:
		return 0;
	case 2:
		return -EEXIST;
	case 3:
		return -ENOENT;
	case 4:
		return -ENOSPC;
	case 5:
		return -ENOTEMPTY;
	default:
		return -EINVAL;

	}
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
