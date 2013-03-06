#include "libRFS.h"

/*
 * Operacion Ok :1
 * Ya Existe el nombre: 2
 * Path Erroneo: 3
 * No hay recursos: 4
 */

int32_t rfs_getattr(int32_t sock, NIPC paqueteEnviar, char *path) {
	// tiene que retornar un Struct Stat

	extern t_log *loguear;

	usleep(retardo);
	Ext2Inodo * inodo;
	int32_t nInodo = leerSubDirectorios(path);

	if (nInodo != -1) {
		subirOperacion(nInodo, 1);

		inodo = leerInodoDeTabla(nInodo);

		liberarMutexOperacion(nInodo);
		crearNIPC(1, (int32_t) inodo->linksCount, (int32_t) inodo->size, 0,
				NULL, &paqueteEnviar, (int32_t) inodo->mode);

		free(inodo); // free nuevo
	} else {
		crearNIPC(1, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}

	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, " Nombre Be Archivo: %s- Operacion getattr",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;

}

int32_t rfs_readdir(int32_t sock, NIPC paqueteEnviar, char *path) {

	extern t_log *loguear;

	int32_t numeroInodo = leerSubDirectorios(path);
	char* buf = NULL;
	if (numeroInodo != 1) {

		subirOperacion(numeroInodo, 0);

		usleep(retardo);
		buf = listarDirectorio(numeroInodo);

		liberarMutexOperacion(numeroInodo);

		if (buf != NULL){
			crearNIPC(2, 0, 0, strlen(buf) + 1, buf, &paqueteEnviar, 0);
			free(buf); // lo agrege recien ni estaba u-u
		}else{
			crearNIPC(2, 0, 3, 0, buf, &paqueteEnviar, 0);
		}
	} else {
		crearNIPC(2, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}

	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, " Nombre De Archivo: %s- Operacion readdir",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

int32_t rfs_open(int32_t sock, NIPC paqueteEnviar, char *path) {
	extern t_log *loguear;

	usleep(retardo);
	int32_t numeroInodo = leerSubDirectorios(path);

	if (numeroInodo != -1) {
		//Mando el numero de Inodo en el size y un 1 en el offset//
		crearNIPC(3, 1, numeroInodo, 0, NULL, &paqueteEnviar, 0); //Preg
	} else {
		crearNIPC(3, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}
	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, " Nombre De Archivo:%s- Operacion open",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}
int32_t rfs_release(int32_t sock, NIPC paqueteEnviar, char *path) {

	//Operacion 4;
	return 0;
}

int32_t rfs_read(int32_t sock, NIPC paqueteEnviar, char *path, size_t size,
		off_t offset) {

	extern t_log *loguear;

	int32_t sizeOriginal = size;

	char * buff = NULL;
	int32_t numeroInodo = leerSubDirectorios(path);

	if (numeroInodo != -1) {

		subirOperacion(numeroInodo, 0);

		usleep(retardo);
		buff = leerArchivo(numeroInodo, size, offset, path, &sizeOriginal);

		liberarMutexOperacion(numeroInodo);
		if (buff != NULL) {
			if (sizeOriginal == 0) {
				crearNIPC(5, 1, sizeOriginal, sizeOriginal, buff,
										&paqueteEnviar, 0);
				// aca no va free porqe buf es "" :P
			} else {
				crearNIPC(5, 0, sizeOriginal, sizeOriginal, buff,
						&paqueteEnviar, 0);
				free(buff);  // Ojo lo agrege reciennnnnn verificar

			}

		} else {
			crearNIPC(5, 0, 3, 0, NULL, &paqueteEnviar, 0);
		}
	} else {
		crearNIPC(5, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}

	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear,
			"Nombre De Archivo: %s- Operacion read- Size: %d- Offset: %d",
			nombreDeArchivo, size, offset);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

int32_t rfs_write(int32_t sock, NIPC paqueteEnviar, char *bufPath, size_t size,
		off_t offset) {

	char* separador = malloc(strlen("|") + 1);
	separador = "|";

	char ** path = string_split(bufPath, separador);

	char*bufferDeEscritura = NULL;
	bufferDeEscritura=calloc(size,sizeof(char));
	memcpy(bufferDeEscritura,bufPath +strlen(*path),size);
	//bufferDeEscritura = malloc((strlen(bufPath) + 1) - (strlen(*path) + 1));
	//bufferDeEscritura = bufPath + strlen(*path) + 1;

	extern t_log *loguear;

	int32_t numeroInodo = leerSubDirectorios(*path);

	if (numeroInodo != -1) {

		subirOperacion(numeroInodo, 1);

		usleep(retardo);
		int32_t i = escribirArchivo(numeroInodo, size, offset,
				bufferDeEscritura, *path);
		liberarMutexOperacion(numeroInodo);

		if (i == 0) {
			crearNIPC(6, 1, size, 0, NULL, &paqueteEnviar, 0); //le envio el size en size y un 1 en offset

		} else {
			crearNIPC(6, 0, i, 0, NULL, &paqueteEnviar, 0);
		}
	} else {
		crearNIPC(6, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}

	char* nombreDeArchivo = dameElNombre(*path);
	log_debug(loguear,
			"Nombre De Archivo: %s- Operacion write- Size: %d- Offset: %d",
			nombreDeArchivo, size, offset);

	//free(bufferDeEscritura) ; // ver que pasa en la escritura con este nuevo
	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

int32_t rfs_create(int32_t sock, NIPC paqueteEnviar, char *path, int32_t modo) {
	extern t_log *loguear;

	usleep(retardo);
	int32_t i = crearArchivo(path, modo);

	crearNIPC(7, 0, i, 0, NULL, &paqueteEnviar, 0);

	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion create",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

int32_t rfs_unlink(int32_t sock, NIPC paqueteEnviar, char *path) {
	extern t_log *loguear;

	usleep(retardo);
	int32_t i = eliminarArchivo(path);
	if (i == 1)
		eliminarValor('8', path);

	crearNIPC(8, 0, i, 0, NULL, &paqueteEnviar, 0);
	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion unlink",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

int32_t rfs_truncate(int32_t sock, NIPC paqueteEnviar, char *path, size_t size) {

	extern t_log *loguear;

	int32_t numeroInodo = leerSubDirectorios(path);
	int32_t i;
	if (numeroInodo != -1) {

		eliminarValor('1', path);

		subirOperacion(numeroInodo, 1);

		usleep(retardo);

		i = truncarArchivo(numeroInodo, size);

		liberarMutexOperacion(numeroInodo);

		crearNIPC(9, 0, i, 0, NULL, &paqueteEnviar, 0);
	} else {
		crearNIPC(9, 0, 3, 0, NULL, &paqueteEnviar, 0);
	}
	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion truncate",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

// Si es rm -R Fuse llama a Rmdir Recursivamente SOLO
int32_t rfs_rmdir(int32_t sock, NIPC paqueteEnviar, char *path) {
	extern t_log *loguear;

	usleep(retardo);
	int32_t i = eliminarDirectorio(path);
	crearNIPC(10, 0, i, 0, NULL, &paqueteEnviar, 0);

	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, "Nombre De Archivo: %s- Operacion rmdir",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
	//tiene que devolver fi.
	//return rfs_unlink(sock, paqueteEnviar, path);
}

int32_t rfs_mkdir(int32_t sock, NIPC paqueteEnviar, char *path, int32_t modo) {

	extern t_log *loguear;

	usleep(retardo);
	int32_t i = crearDirectorio(path, modo);
	crearNIPC(11, 0, i, 0, NULL, &paqueteEnviar, 0);
	char* nombreDeArchivo = dameElNombre(path);
	log_debug(loguear, "Nombre De Archivo:%s- Operacion mkdir",
			nombreDeArchivo);

	enviarNIPC(sock, paqueteEnviar);
	chauNIPC(&paqueteEnviar);
	return 0;
}

