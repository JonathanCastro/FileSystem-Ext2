#include "libRFS.h"

void switchFunciones(NIPC paquete, int32_t sock, NIPC paqueteEnviar) {

	switch (paquete.PayloadDescriptor) {
	case 0:
		handshakeRFS(sock, paquete);
		break;
	case 1:
		rfs_getattr(sock, paqueteEnviar, paquete.payload);
		break;
	case 2:
		rfs_readdir(sock, paqueteEnviar, paquete.payload);
		break;
	case 3:
		rfs_open(sock, paqueteEnviar, paquete.payload);
		break;
	case 4:
		rfs_release(sock, paqueteEnviar, paquete.payload);
		break;
	case 5:
		rfs_read(sock, paqueteEnviar, paquete.payload, paquete.size,
				paquete.offset);
		break;
	case 6:
		rfs_write(sock, paqueteEnviar, paquete.payload, paquete.size,
				paquete.offset); //falta el buffer en paquete
		break;
	case 7:
		rfs_create(sock, paqueteEnviar, paquete.payload, paquete.modo);
		break;
	case 8:
		rfs_unlink(sock, paqueteEnviar, paquete.payload);
		break;
	case 9:
		rfs_truncate(sock, paqueteEnviar, paquete.payload, paquete.size);
		break;
	case 10:
		rfs_rmdir(sock, paqueteEnviar, paquete.payload);
		break;
	case 11:
		rfs_mkdir(sock, paqueteEnviar, paquete.payload, paquete.modo);
		break;
	default:
		break;

	}
}

/** ____________________Listar Subdirectorios segun un path______________ **/

int32_t buscarEnBloqueDeDato(int32_t bloqueDato, char* palabra) {
	int32_t offset = 0;
	Ext2Directory *directorio = NULL;
	extern int32_t TAM_BLOCK;
	char* buffer = NULL;
	buffer = malloc(TAM_BLOCK);
	char* nombre = NULL;
	leerBloqueDisco(bloqueDato, buffer, 1);

	directorio = (Ext2Directory *) buffer;

	while (offset < TAM_BLOCK) {
		offset += directorio->rec_len;
		nombre = (char*) calloc(directorio->name_len + 1, sizeof(char));
		memset(nombre, '\0', directorio->name_len + 1);
		memcpy(nombre, directorio->name, directorio->name_len);

		if ((strcmp(nombre, palabra)) == 0) {
			free(nombre);
			return directorio->inode;
		}
		free(nombre);
		directorio = (Ext2Directory *) (buffer + offset);
	}
	free(buffer);
	return -1;
}

int32_t buscarEntrada(int32_t nInodoDirectorio, char* palabra) {

	int32_t numeroDeInodo = 0;
	int32_t i = 1;
	int32_t bloqueDato = -1;

	Ext2Inodo *inodoAbuscarEntrada;
	inodoAbuscarEntrada = leerInodoDeTabla(nInodoDirectorio);
	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAbuscarEntrada);

	if (tipoDeArchivo != IFDIR) {
		return -1; // Controlar estooo con joni
	}
	bloqueDato = leerBloqueDeDato(i, inodoAbuscarEntrada);
	do {

		numeroDeInodo = buscarEnBloqueDeDato(bloqueDato, palabra);
		if (numeroDeInodo != -1)
			break;
		i++;
		bloqueDato = leerBloqueDeDato(i, inodoAbuscarEntrada);
	} while (bloqueDato != 0);

	free(inodoAbuscarEntrada); // free nuevo

	return numeroDeInodo;
}

int32_t leerSubDirectorios(char * path) {

	char *raiz = "/";
	int32_t tamPath = (strlen(path) + 1);
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra;
	int32_t nInodoAListar = EXT2_ROOT_INODE_INDEX;
	int32_t nInodoDirectorio;

	int8_t comparaEntrada = (strcmp(path, raiz));

	if (comparaEntrada == 0) /* ----> Quiero Listar El Directorio Raiz, Ya tengo el Nro de Inodo */
		return nInodoAListar;

	memcpy(ruta, path, tamPath);

	palabra = strtok(ruta, "/");
	while (nInodoAListar != -1 && palabra != NULL) {
		nInodoDirectorio = nInodoAListar;
		nInodoAListar = buscarEntrada(nInodoDirectorio, palabra);
		palabra = strtok(NULL, raiz);
	}
	return nInodoAListar;

}

/** _____________________________LISTAR DIRECTORIO_______________ **/

char* listarDirectorio(int32_t nInodo) {

	extern int32_t TAM_BLOCK;
	int32_t i = 1;
	int32_t offset = 0;
	int32_t offsetLs;

	Ext2Inodo *inodoAlistar = NULL; // = malloc(sizeof(Ext2Inodo));
	Ext2Directory *directorio = NULL;
	char* buffer = NULL;
	buffer = calloc(TAM_BLOCK, sizeof(char));
	char* nombre = NULL;
	char* lsDirectorio = NULL;

	inodoAlistar = leerInodoDeTabla(nInodo);

	int32_t cantDeBloqueDatos = (inodoAlistar->size / TAM_BLOCK);
	if ((inodoAlistar->size % TAM_BLOCK) != 0)
		cantDeBloqueDatos++;

	lsDirectorio = calloc(TAM_BLOCK * cantDeBloqueDatos, sizeof(char));

	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAlistar);
	if (tipoDeArchivo != IFDIR) {
		printf("No es Directorio, NO SE PUEDE LISTAR");
		free(buffer);
		free(lsDirectorio);
		free(inodoAlistar);

		return NULL;
	}

	int32_t bloqueDato = leerBloqueDeDato(i, inodoAlistar);
	offsetLs = 0;
	do {

		if (bloqueDato != 0) {
			leerBloqueDisco(bloqueDato, buffer, 1);
			directorio = (Ext2Directory *) buffer;
			while (offset < TAM_BLOCK) {

				offset += directorio->rec_len;
				nombre = (char*) calloc(directorio->name_len + 1, sizeof(char));
				memset(nombre, '\0', directorio->name_len + 1);
				memcpy(nombre, directorio->name, directorio->name_len);

				lsDirectorio[offsetLs] = '/';
				offsetLs += 1;
				memcpy(lsDirectorio + offsetLs, nombre, directorio->name_len);
				offsetLs += directorio->name_len;

				free(nombre);

				directorio = (Ext2Directory *) (buffer + offset);

			}
		}
		offset = 0;
		memset(buffer, 0, TAM_BLOCK); //NUEVO
		i++;
		bloqueDato = leerBloqueDeDato(i, inodoAlistar);

	} while (i <= cantDeBloqueDatos);

	lsDirectorio[strlen(lsDirectorio) + 1] = '\0';
	lsDirectorio = realloc(lsDirectorio, (offsetLs + 1) * sizeof(char));

	free(buffer);
	free(inodoAlistar);

	return lsDirectorio;
}

/*_________________________Leer Archivo_____________________*/

char * leerArchivo2(int32_t nInodo, size_t size, off_t offset) {
	extern int32_t TAM_BLOCK;
	div_t divCantBloques;
	divCantBloques = div(size, TAM_BLOCK);

	char * buffer = NULL;
	buffer = (char*) calloc(TAM_BLOCK, sizeof(char));
	int primeraVez = 0;
	// En en que numero de bloque Archivo esta ese offset)
	int32_t numeroBloqueArchivo = (offset / TAM_BLOCK) + 1;

	//cantidad de Bytes a leer.
	int32_t cantidadDeBytesALeer = (divCantBloques.quot * TAM_BLOCK)
			+ divCantBloques.rem;

	int32_t cantBloquesAleer = (size / TAM_BLOCK);
	if ((size % TAM_BLOCK) != 0)
		cantBloquesAleer++;
	int32_t bloqueDato;
	int32_t i = numeroBloqueArchivo;
	int32_t offsetBuff = 0;
	Ext2Inodo *inodoAleer = NULL; //= (Ext2Inodo*) malloc(sizeof(Ext2Inodo));
	inodoAleer = leerInodoDeTabla(nInodo);

	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAleer);
	if (tipoDeArchivo != IFREG) {
		printf("No es Archivo Regular, NO SE PUEDE LISTAR"); //se saca
		return (char*) NOARCHIVO;
	}

	char *buf = NULL;
	buf = (char*) calloc(cantidadDeBytesALeer, sizeof(char));

	do {
		if (cantidadDeBytesALeer >= TAM_BLOCK) {

			if (primeraVez == 0 && (offset % TAM_BLOCK) != 0) {
				primeraVez = 1;
				bloqueDato = leerBloqueDeDato(i, inodoAleer);
				leerBloqueDisco(bloqueDato, buffer, 1);

				memcpy(buf + offsetBuff, buffer + offset, TAM_BLOCK - offset);
				cantidadDeBytesALeer -= TAM_BLOCK - offset;
				offsetBuff += (TAM_BLOCK - offset);
				i++;
			} else {

				bloqueDato = leerBloqueDeDato(i, inodoAleer);
				leerBloqueDisco(bloqueDato, buffer, 1);
				memcpy(buf + offsetBuff, buffer, TAM_BLOCK);
				offsetBuff += TAM_BLOCK;
				cantidadDeBytesALeer -= TAM_BLOCK;
				i++;
			}
		} else {
			if (primeraVez == 0 && (offset % TAM_BLOCK) != 0) {
				primeraVez = 1;
				if (TAM_BLOCK - offset > divCantBloques.rem) {
					bloqueDato = leerBloqueDeDato(i, inodoAleer);
					leerBloqueDisco(bloqueDato, buffer, 1);
					memcpy(buf + offsetBuff, buffer + offset,
							divCantBloques.rem);
					cantidadDeBytesALeer -= divCantBloques.rem;
					offsetBuff += divCantBloques.rem;
					i++;
				} else {
					bloqueDato = leerBloqueDeDato(i, inodoAleer);
					leerBloqueDisco(bloqueDato, buffer, 1);
					memcpy(buf + offsetBuff, buffer + offset,
							TAM_BLOCK - offset);
					cantidadDeBytesALeer -= TAM_BLOCK - offset;
					offsetBuff += TAM_BLOCK - offset;
					i++;

				}
			} else {
				bloqueDato = leerBloqueDeDato(i, inodoAleer);
				leerBloqueDisco(bloqueDato, buffer, 1);
				memcpy(buf + offsetBuff, buffer, cantidadDeBytesALeer);
				cantidadDeBytesALeer -= cantidadDeBytesALeer;
				i++;
			}
		}

	} while (cantidadDeBytesALeer > 0);

	printf("%s", buf);
	free(inodoAleer);
	free(buffer);
	return buf;
}

char * leerArchivo(int32_t nInodo, size_t size, off_t offset, char* path,
		int32_t *sizeNuevo) {

	extern int32_t TAM_BLOCK;
	Ext2Inodo *inodoAleer = NULL;
	inodoAleer = leerInodoDeTabla(nInodo);
	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAleer);
	if (tipoDeArchivo != IFREG) {
		printf("No es Archivo Regular, NO SE PUEDE LISTAR"); //se saca
		return NULL;
	}
	if (inodoAleer->size == 0) {
		*sizeNuevo = 0;
		return "";
	}

	if (offset + size > inodoAleer->size) {
		size = inodoAleer->size - offset;
	}
	if (size == 0) {
		*sizeNuevo = 0;
		return "";
	}

	div_t divOffSet = div(offset, TAM_BLOCK);
	int32_t cantBloquesAleer = ((offset + size) / TAM_BLOCK);
	if (((size + offset) % TAM_BLOCK) != 0)
		cantBloquesAleer++;

	char* buffTotal = NULL;
	buffTotal = (char*) calloc((cantBloquesAleer - divOffSet.quot) * TAM_BLOCK,
			sizeof(char));

	char* buffLecturaArchivo = NULL;
	buffLecturaArchivo = (char*) calloc(size, sizeof(char));

	char* buffLecturaDisco = NULL;

	int i;
	int32_t offsetBuffTotal = 0;
	for (i = 1; i <= (cantBloquesAleer - divOffSet.quot); i++) {

		int32_t bloqueDato = leerBloqueDeDato(divOffSet.quot + i, inodoAleer);
		//busco en la cache
		buffLecturaDisco = getValorRW(bloqueDato, path);
		if (buffLecturaDisco == NULL) {

			buffLecturaDisco = (char*) calloc(TAM_BLOCK, sizeof(char));
			leerBloqueDisco(bloqueDato, buffLecturaDisco, 1);
			//cacheo
			setearValorRW(bloqueDato, path, buffLecturaDisco);
		}
		memcpy(buffTotal + offsetBuffTotal, buffLecturaDisco, TAM_BLOCK);

		offsetBuffTotal += TAM_BLOCK;
		free(buffLecturaDisco);
	}

	memcpy(buffLecturaArchivo, buffTotal + divOffSet.rem, size);
	*sizeNuevo = size;

	free(buffTotal);
	free(inodoAleer);

	return buffLecturaArchivo;

}

/**_______________________Truncar Archivo______________**/

void actualizarSize(int32_t numeroInodo, Ext2Inodo *inodo, size_t nuevoTamanio) {

	inodo->size = nuevoTamanio;
	escribirInodoTabla(numeroInodo, inodo);
}

Ext2Inodo* agrandarArchivo(int32_t numeroDeinodo, Ext2Inodo * inodo,
		size_t nuevoTamanio) {
	extern int32_t TAM_BLOCK;
	extern Ext2SuperBloque *superBloque;
	int i;
	int cantBloquesAgregar = 0;
	char* buff = NULL;

	int cantBloquesActuales = inodo->size / TAM_BLOCK;
	if (inodo->size % TAM_BLOCK != 0) {
		cantBloquesActuales++;
	}
	int cantBloquesNuevos = nuevoTamanio / TAM_BLOCK;
	if (nuevoTamanio % TAM_BLOCK != 0) {
		cantBloquesNuevos++;
	}

	if (cantBloquesNuevos > cantBloquesActuales) {
		cantBloquesAgregar = cantBloquesNuevos - cantBloquesActuales;

		if (cantBloquesAgregar > (superBloque->s_free_blocks_count)) {

			return NULL;
		}

		buff = (char*) calloc(TAM_BLOCK, sizeof(char)); // LO MOVI YO : Joni

		for (i = 1; i <= cantBloquesAgregar; i++) {
			//busca bloque libre y lo ocupa (ver Semaforos)
			int32_t bloqueLibre = buscarBloqueLibre(numeroDeinodo);
			if (bloqueLibre == -1) {
				free(buff);
				return NULL;
			}
			//Escribir con /0.

			buff = string_repeat('\0', TAM_BLOCK);

			escribirBloqueDisco(bloqueLibre, 0, TAM_BLOCK, buff);
			//setear bloquelibre al campo Block de inodo
			asignarBloqueEnInodo(cantBloquesActuales + i, bloqueLibre, inodo,
					numeroDeinodo);

		}
		free(buff);
	}

	div_t divCantBloques;
	divCantBloques = div(inodo->size, TAM_BLOCK);

	if (divCantBloques.rem != 0) {
		buff = (char*) calloc(TAM_BLOCK - (divCantBloques.rem), sizeof(char));
		buff = string_repeat('\0', TAM_BLOCK - divCantBloques.rem);

		int ultimoBloqueDato = leerBloqueDeDato(cantBloquesActuales, inodo);

		escribirBloqueDisco(ultimoBloqueDato, divCantBloques.rem,
				TAM_BLOCK - divCantBloques.rem, buff);
		free(buff);

	}
	actualizarSuperBloque(0, cantBloquesAgregar, 0, 0);

	return (inodo);
}

Ext2Inodo* achicarArchivo(int32_t numeroDeinodo, Ext2Inodo* inodo,
		size_t nuevoTamanio) {

	extern int32_t TAM_BLOCK;
	int i;
	int cantBloquesALiberar = 0;
	int cantBloquesActuales = inodo->size / TAM_BLOCK;
	if (inodo->size % TAM_BLOCK != 0) {
		cantBloquesActuales++;
	}
	int cantBloquesNuevos = nuevoTamanio / TAM_BLOCK;
	if (nuevoTamanio % TAM_BLOCK != 0) {
		cantBloquesNuevos++;
	}

	if (cantBloquesNuevos < cantBloquesActuales) {
		cantBloquesALiberar = cantBloquesActuales - cantBloquesNuevos;
		for (i = 0; i < cantBloquesALiberar; i++) {

			int32_t bloqueDisco = leerYasiganrBloqueDeDato(cantBloquesActuales,
					inodo);
			liberarBloque(bloqueDisco);
			//Setearle 0 en el campoBlock
			//asignarBloqueEnInodo(cantBloquesActuales, 0, inodo, numeroDeinodo);
			cantBloquesActuales--;
		}
		//asignarBloqueEnInodo(cantBloquesActuales - cantBloquesALiberar + 1, 0,
		//		inodo, numeroDeinodo);
	}
	actualizarSuperBloque(cantBloquesALiberar, 0, 0, 0);
	return (inodo);
}

Ext2Inodo* agrandarArchivoNuevo(int32_t numeroInodo, Ext2Inodo *inodo,
		size_t nuevoTamanio) {
	int i;
	extern Ext2SuperBloque *superBloque;

	extern int32_t TAM_BLOCK;
	char* buff = NULL;
	buff = (char*) calloc(TAM_BLOCK, sizeof(char));

	int cantBloquesNuevos = nuevoTamanio / TAM_BLOCK;
	if (nuevoTamanio % TAM_BLOCK != 0) {
		cantBloquesNuevos++;
	}

	if (cantBloquesNuevos > superBloque->s_free_blocks_count) { // esto es nuevo TESTIAR CON KARRRRRRRRRR
		free(buff);
		return NULL;
	}

	for (i = 1; i <= cantBloquesNuevos; i++) {

		//busca bloque libre y lo ocupa (ver Semaforos)
		int32_t bloqueLibre = buscarBloqueLibre(numeroInodo);
		if (bloqueLibre == -1) {
			return NULL;
		}
		//Escribir con /0.
		buff = string_repeat('\0', TAM_BLOCK);
		escribirBloqueDisco(bloqueLibre, 0, TAM_BLOCK, buff);
		//setear bloquelibre al campo Block de inodo
		asignarBloqueEnInodo(i, bloqueLibre, inodo, numeroInodo);
	}
	actualizarSuperBloque(0, cantBloquesNuevos, 0, 0);
	free(buff);
	return (inodo);
}

int32_t truncarArchivo(int32_t numeroInodo, size_t nuevoTamanio) {

	Ext2Inodo *inodo = NULL; //= (Ext2Inodo*) malloc(sizeof(Ext2Inodo));

	inodo = leerInodoDeTabla(numeroInodo);

	size_t tamanioActual = inodo->size;

	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodo);
	if (tipoDeArchivo != IFREG) {
		printf("No es Archivo Regular, NO SE PUEDE TRUNCAR"); //se saca
		return 3;
	}

	if (nuevoTamanio > tamanioActual) {
		if (tamanioActual == 0) {
			//caso cuando el tamanio actual es igual 0;
			inodo = agrandarArchivoNuevo(numeroInodo, inodo, nuevoTamanio);
			if (inodo == NULL) {
				return 4; //No hay Recurso
			}

		} else {
			//El tamaño nuevo es mas grande. tengo que asignarle bloques si es necesario
			inodo = agrandarArchivo(numeroInodo, inodo, nuevoTamanio);
			if (inodo == NULL) {
				return 4; //No hay Recurso
			}
		}
	} else {
		//El tamaño nuevo es mas chico , tengo que liberar bloques si es necesario
		inodo = achicarArchivo(numeroInodo, inodo, nuevoTamanio);
	}
//cambiar el size al inodo.
	actualizarSize(numeroInodo, inodo, nuevoTamanio);

	printf("fin truncar \n");
	return 1;
}

/** ________________Escribir Archivo_____________________ **/

int32_t escribirArchivo(int32_t nInodo, size_t size, off_t offset,
		char* buffEscritura, char *path) {

	extern int32_t TAM_BLOCK;
	Ext2Inodo *inodoAleer = NULL; // = (Ext2Inodo*) calloc(1, sizeof(Ext2Inodo));
	inodoAleer = leerInodoDeTabla(nInodo);
	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAleer);
	int32_t resultado;
	if (tipoDeArchivo != IFREG) {
		printf("No es Archivo Regular, NO SE PUEDE ESCRIBIR"); //se saca
		return 3;
	}

	int32_t sizeNuevo = (offset + size) - (inodoAleer->size);
	if (sizeNuevo > 0) {
		resultado = truncarArchivo(nInodo, inodoAleer->size + sizeNuevo);
		inodoAleer = leerInodoDeTabla(nInodo);

	}
	if (resultado != 4) {

		div_t divOffSet = div(offset, TAM_BLOCK);
		int32_t cantBloquesAleer = ((size + offset) / TAM_BLOCK);
		if (((size + offset) % TAM_BLOCK) != 0)
			cantBloquesAleer++;

		char* buffTotal = NULL;
		buffTotal = (char*) calloc(
				(cantBloquesAleer - divOffSet.quot) * TAM_BLOCK, sizeof(char));

		char* buffLecturaDisco = NULL;
		int i;
		int32_t offsetBuffTotal = 0;

		t_listaBloques * lista = crearListaBloques();

		for (i = 1; i <= (cantBloquesAleer - divOffSet.quot); i++) {

			int32_t bloqueDato = leerBloqueDeDato(divOffSet.quot + i,
					inodoAleer);
			//busco en la cache
			buffLecturaDisco = getValorRW(bloqueDato, path);
			if (buffLecturaDisco == NULL) {
				buffLecturaDisco = (char*) calloc(TAM_BLOCK, sizeof(char));
				leerBloqueDisco(bloqueDato, buffLecturaDisco, 1);
			}

			agregarEnLaListaBloques(bloqueDato, lista);
			memcpy(buffTotal + offsetBuffTotal, buffLecturaDisco, TAM_BLOCK);
			offsetBuffTotal += TAM_BLOCK;
			free(buffLecturaDisco); // borraba solo 1 al final.
		}

		if ((strcmp(buffEscritura, "")) == 0) {
			int32_t j;
			for (j = divOffSet.rem; j < (divOffSet.rem + size); j++) {
				memcpy(buffTotal + j, buffEscritura, 1);

			}
		}else{
			memcpy(buffTotal +divOffSet.rem, buffEscritura, size);
		}

		offsetBuffTotal = 0;
		t_nodoBloques *bloque;
		char* pedasito = NULL;
		pedasito = (char*) calloc(TAM_BLOCK, sizeof(char));

		for (i = 1; i <= (cantBloquesAleer - divOffSet.quot); i++) {

			sacarDeLaListaBloques(&bloque, lista);
			memset(pedasito, 0, TAM_BLOCK);
			memcpy(pedasito, buffTotal + offsetBuffTotal, TAM_BLOCK);
			offsetBuffTotal += TAM_BLOCK;
			escribirBloqueDisco(bloque->bloque, 0, TAM_BLOCK, pedasito);
			setearValorRW(bloque->bloque, path, pedasito); //cacheo
			free(bloque); // nuevo , estaba abajo :O
		}

		free(pedasito);
		free(buffTotal);
		free(inodoAleer);
		return 0;
	} else {
		return resultado;
	}
}

int32_t escribirArchivo2(int32_t nInodo, size_t size, off_t offset,
		char* buffEscritura) {

	extern int32_t TAM_BLOCK;
	div_t divBArchivo;
	divBArchivo = div(offset, TAM_BLOCK);

	div_t divCantBEsc;
	divCantBEsc = div(size, TAM_BLOCK);
	Ext2Inodo *inodoAEscribir = NULL; // = malloc(sizeof(Ext2Inodo));
	inodoAEscribir = leerInodoDeTabla(nInodo);

	type_tipo_archivo tipoDeArchivo = tipoDeFichero(inodoAEscribir);
	if (tipoDeArchivo != IFREG) {
		printf("No es un Archivo Regular, NO SE PUEDE ESCRIBIR"); //se saca
		return NOARCHIVO;
	}

	int32_t sizeBloquesNuevos = obtenerCantBloquesNuevos(inodoAEscribir,
			(int32_t) offset, (int32_t) size);

	if (sizeBloquesNuevos == -1) {
		printf("Hay problemas con los bloques nuevos \n");
		return -1;
	}
	if (sizeBloquesNuevos > 0)
		truncarArchivo(nInodo, inodoAEscribir->size + sizeBloquesNuevos);

	escribirBolques(divBArchivo, divCantBEsc, inodoAEscribir, buffEscritura);

	escribirInodoTabla(nInodo, inodoAEscribir);
	return 0;
}

void escribirBolques(div_t offsetArchivo, div_t sizeAEscribir,
		Ext2Inodo *inodoAEsc, char* buffEscritura) {
	extern int32_t TAM_BLOCK;
	int32_t i = offsetArchivo.quot;
	int32_t size = sizeAEscribir.quot;
	int32_t bloqueDato;
	int32_t offBuffEscritura = 0;
	int32_t cantEscribir = (sizeAEscribir.quot * TAM_BLOCK) + sizeAEscribir.rem
			- 1;
	int32_t offEscritura = offsetArchivo.rem;
	int32_t tamEscribir = 0;
	char* buf = NULL;
	char* nuevoCharDato = NULL;

	do {

		if (cantEscribir < TAM_BLOCK)
			tamEscribir = cantEscribir;
		else
			tamEscribir = TAM_BLOCK - offEscritura;

		bloqueDato = leerBloqueDeDato(i + 1, inodoAEsc);
		buf = malloc(tamEscribir);
		memcpy(buf,
				buffEscritura
						+ (offBuffEscritura * (tamEscribir + offEscritura)),
				(tamEscribir));

		nuevoCharDato = malloc(TAM_BLOCK);
		nuevoCharDato = armarNuevoBloque(tamEscribir, offEscritura, bloqueDato,
				buf);
		escribirBloqueDisco(bloqueDato, 0, (TAM_BLOCK), nuevoCharDato);

		if (offEscritura != 0)
			cantEscribir -= offEscritura;
		else
			cantEscribir -= tamEscribir;

		offEscritura = 0;
		i++;
		offBuffEscritura++;
		free(nuevoCharDato);

	} while ((i <= (offsetArchivo.quot + size)) && cantEscribir > 0);
}

char* armarNuevoBloque(int32_t cantEscribir, int32_t offEscritura,
		int32_t bloqueDato, char* aEscribir) {
	extern int32_t TAM_BLOCK;
	if (cantEscribir == TAM_BLOCK) // escribio un bloque
		return aEscribir;

	char* buffer = NULL;
	buffer = malloc(TAM_BLOCK);
	leerBloqueDisco(bloqueDato, buffer, 1);

	memcpy(buffer + offEscritura, aEscribir, cantEscribir);
	return buffer;

}

int32_t obtenerCantBloquesNuevos(Ext2Inodo *inodoAEscribir, int32_t offset,
		int32_t size) {
	extern int32_t TAM_BLOCK;
	int32_t cantBloquesDelArchivo = inodoAEscribir->size / TAM_BLOCK;
	if ((inodoAEscribir->size % TAM_BLOCK) != 0)
		cantBloquesDelArchivo++;

	if (offset < (cantBloquesDelArchivo * TAM_BLOCK)) {
		if ((offset + size) <= (cantBloquesDelArchivo * TAM_BLOCK))
			return 0;
		else
			return ((offset + size) - (cantBloquesDelArchivo * TAM_BLOCK));

	} else {
		return size;
	}

	return -1;

}

/**____________________ Obtener nombre del Path ____________________**/

int32_t buscarInodoDelPathSinNombre(char* path, int32_t cantToken) {
	char *raiz = "/";
	int32_t tamPath = (strlen(path) + 1);
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra = NULL;
	int32_t nInodoAListar = EXT2_ROOT_INODE_INDEX;
	int32_t nInodoDirectorio;

	int32_t i = 0;
	memcpy(ruta, path, tamPath); // ***  /home/utn/lala.txt   , con cantToken=2
	palabra = strtok(ruta, "/");
	while (nInodoAListar != -1 && palabra != NULL && i < cantToken) {
		nInodoDirectorio = nInodoAListar;
		nInodoAListar = buscarEntrada(nInodoDirectorio, palabra);
		i++;
		palabra = strtok(NULL, raiz);
	}
	if (nInodoAListar == -1) {

		printf("Path Errone\n");
		return -1;
	}
	return nInodoAListar;
}

int32_t obtenerInodoQueLoContiene(char* path) { // le paso la var nombre para que me la complete
	char *raiz = "/";
	int32_t tamPath = (strlen(path) + 1);
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra = NULL;
	int32_t cantToken = 0;

	memcpy(ruta, path, tamPath);

	palabra = strtok(ruta, "/");
	while (palabra != NULL) {
		cantToken++;
		palabra = strtok(NULL, raiz);
	}
	cantToken--;

	if (cantToken == 0) {
		return 2; // Quiere agregar en el Raiz el gay.
	} else {
		return buscarInodoDelPathSinNombre(path, cantToken); // Quiere agregar en otro u.u
	}
}

/**_________________________ Consultar que tenga . y .. solo_________________**/

int32_t tieneSoloElPyPP(int32_t inode, int32_t tipo) {
	if (tipo != 2)
		return 1;
	extern int32_t TAM_BLOCK;
	Ext2Inodo *inodo = NULL;
	inodo = leerInodoDeTabla(inode);
	int32_t offsetSiguiente = 0;

	Ext2Directory *directorio = NULL;
	Ext2Directory *directorioSiguiente = NULL;

	char * buff = NULL;
	buff = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = leerBloqueDeDato(1, inodo);
	leerBloqueDisco(bloque, buff, 1);

	directorio = (Ext2Directory *) buff;
	offsetSiguiente += directorio->rec_len;
	directorioSiguiente = (Ext2Directory *) (buff + offsetSiguiente);

	if ((directorio->rec_len + directorioSiguiente->rec_len) == TAM_BLOCK) {
		free(inodo);
		free(buff); // OJO :P
		return 1;
	}
	free(buff); // OJO :P
	free(inodo);
	return 0;

}

/**_________________________ Borrar una ENTRADA del Directorio __________________________**/

int32_t borrarEsaEntradaDelDirectorio(char* nombreDir,
		Ext2Inodo *inodoDirectorio, int32_t tipo, int32_t nInodoDirectorio) {

	int32_t i = 1;
	int32_t offset = 0;
	int32_t offsetSiguiente = 0;
	Ext2Directory *directorio = NULL;
	Ext2Directory *directorioSiguiente = NULL;
	extern int32_t TAM_BLOCK;
	int32_t cantDeBloqueDatos = (inodoDirectorio->size / TAM_BLOCK);
	if ((inodoDirectorio->size % TAM_BLOCK) != 0)
		cantDeBloqueDatos++;

	int32_t loEncontro = 0;
	char* buffer = NULL;
	int32_t bloqueDato;
	char* nombreSig = NULL;
	char *nombreAct = NULL;

	int32_t inodoEliminado = -1;

	bloqueDato = leerBloqueDeDato(i, inodoDirectorio);
	buffer = (char*) calloc(TAM_BLOCK, sizeof(char));
	do {

		if (bloqueDato != 0) {

			leerBloqueDisco(bloqueDato, buffer, 1);
			directorio = (Ext2Directory *) buffer;

			nombreAct = calloc(directorio->name_len + 1, sizeof(char));
			memset(nombreAct, '\0', directorio->name_len + 1);
			memcpy(nombreAct, directorio->name, directorio->name_len);

			if ((strcmp(nombreAct, nombreDir)) != 0) {

				offsetSiguiente += directorio->rec_len;
				directorioSiguiente = (Ext2Directory *) (buffer
						+ offsetSiguiente);

				while (offset < TAM_BLOCK) {

					nombreSig = calloc(directorioSiguiente->name_len + 1,
							sizeof(char));
					memset(nombreSig, '\0', directorioSiguiente->name_len + 1);
					memcpy(nombreSig, directorioSiguiente->name,
							directorioSiguiente->name_len);

					if ((strcmp(nombreSig, nombreDir)) == 0) {
						type_tipo_archivo tipoDeArchivo = tipoDeFicheroDirect(
								directorioSiguiente->inode);
						if (tipoDeArchivo != tipo) {
							printf(
									"No se puede eliminar porque no es un Directorio \n");
							free(nombreSig);
							free(buffer);
							return -1;
						}

						if (!tieneSoloElPyPP(directorioSiguiente->inode,
								tipo)) {
							printf("El directorio no esta vacio \n");
							free(nombreSig);
							free(buffer);
							return -1;

						}
						int32_t lenD = directorio->rec_len;
						directorio->rec_len += directorioSiguiente->rec_len;
						memcpy(buffer + offset, (char*) directorio, lenD); //NUEVO
						loEncontro = 1;
						free(nombreSig);
						inodoEliminado = directorioSiguiente->inode;
						escribirBloqueDisco(bloqueDato, 0, TAM_BLOCK, buffer);
						break;

					}

					offset += directorio->rec_len;
					offsetSiguiente += directorioSiguiente->rec_len;

					directorio = directorioSiguiente;
					directorioSiguiente = (Ext2Directory *) (buffer
							+ offsetSiguiente);
					free(nombreSig);
					nombreSig = NULL;

				}

			} else { // si es el primero borro y compacto o libero el bloque
				loEncontro = 1;
				if (directorio->rec_len == TAM_BLOCK) { //libero el bloque
					liberarBloque(bloqueDato);
					actualizarSuperBloque(1, 0, 0, 0);
					asignarBloqueEnInodo(i, 0, inodoDirectorio,
							nInodoDirectorio);
					free(buffer);
				} else { //compacto

					char* nuevoBuf = (char*) calloc(TAM_BLOCK, sizeof(char));
					memcpy(nuevoBuf, buffer + directorio->rec_len,
							TAM_BLOCK - directorio->rec_len);
					escribirBloqueDisco(bloqueDato, 0, TAM_BLOCK, nuevoBuf);
					free(nuevoBuf);
				}

				free(nombreAct);
				inodoEliminado = directorio->inode;
			}

		}
		i++;
		offset = 0;
		offsetSiguiente = 0;
		memset(buffer, 0, TAM_BLOCK);
		bloqueDato = leerBloqueDeDato(i, inodoDirectorio);

	} while (i <= cantDeBloqueDatos && !loEncontro);

	return inodoEliminado;

}

int32_t borrarEntradaDelDirectorio(char* nombreDir, Ext2Inodo *inodoDirectorio,
		int32_t tipo, int32_t nInodoDirectorio) {
	int32_t i = 1;
	int32_t offset = 0;
	int32_t offsetSiguiente = 0;
	Ext2Directory *directorio = NULL;
	Ext2Directory *directorioSiguiente = NULL;
	extern int32_t TAM_BLOCK;
	int32_t cantDeBloqueDatos = (inodoDirectorio->size / TAM_BLOCK);
	if ((inodoDirectorio->size % TAM_BLOCK) != 0)
		cantDeBloqueDatos++;

	int32_t loEncontro = 0;
	char* buffer = NULL;
	int32_t bloqueDato;
	char* nombre = NULL;

	int32_t inodoEliminado = -1;

	bloqueDato = leerBloqueDeDato(i, inodoDirectorio);
	buffer = (char*) calloc(TAM_BLOCK, sizeof(char));
	do {

		if (bloqueDato != 0) {

			leerBloqueDisco(bloqueDato, buffer, 1);
			directorio = (Ext2Directory *) buffer;

			if (i > 1 && directorio->rec_len == TAM_BLOCK) {
				liberarBloque(bloqueDato);
				actualizarSuperBloque(1, 0, 0, 0);
				asignarBloqueEnInodo(i, 0, inodoDirectorio, nInodoDirectorio);
				free(buffer);

				return directorio->inode;
			}

			offsetSiguiente += directorio->rec_len;
			directorioSiguiente = (Ext2Directory *) (buffer + offsetSiguiente);

			while (offset < TAM_BLOCK) {

				nombre = calloc(directorioSiguiente->name_len + 1,
						sizeof(char));
				memset(nombre, '\0', directorioSiguiente->name_len + 1);
				memcpy(nombre, directorioSiguiente->name,
						directorioSiguiente->name_len);

				if ((strcmp(nombre, nombreDir)) == 0) {
					type_tipo_archivo tipoDeArchivo = tipoDeFicheroDirect(
							directorioSiguiente->inode);
					if (tipoDeArchivo != tipo) {
						printf(
								"No se puede eliminar porque no es un Directorio \n");
						free(nombre);
						free(buffer);
						return -1;
					}

					if (!tieneSoloElPyPP(directorioSiguiente->inode, tipo)) {
						printf("El directorio no esta vacio \n");
						free(nombre);
						free(buffer);
						return -1;

					}
					int32_t lenD = directorio->rec_len;
					directorio->rec_len += directorioSiguiente->rec_len;
					memcpy(buffer + offset, (char*) directorio, lenD); //NUEVO
					loEncontro = 1;
					free(nombre);
					inodoEliminado = directorioSiguiente->inode;
					break;

				}

				offset += directorio->rec_len;
				offsetSiguiente += directorioSiguiente->rec_len;

				directorio = directorioSiguiente;
				directorioSiguiente = (Ext2Directory *) (buffer
						+ offsetSiguiente);
				free(nombre);
				nombre = NULL;
			}

			if (loEncontro) {

				escribirBloqueDisco(bloqueDato, 0, TAM_BLOCK, buffer);
				break;
			}
		}

		i++;
		offset = 0;
		offsetSiguiente = 0;
		memset(buffer, 0, TAM_BLOCK);
		bloqueDato = leerBloqueDeDato(i, inodoDirectorio);

	} while (i <= cantDeBloqueDatos && !loEncontro);

	free(buffer);
	return inodoEliminado;

}

/**_____________________ Eliminar Directorio _______________________________**/

int32_t eliminarDirectorio(char* path) {

	int8_t comparaEntrada = (strcmp(path, "/"));
	if (comparaEntrada == 0) {
		printf("Queres eliminar el RAIZ estas loco ? \n");
		return -3;
	}

	char*nombre = dameElNombre(path);

	int32_t inodoAeliminar;

	Ext2Inodo *inodoQueLoContiene = NULL; // = malloc(sizeof(Ext2Inodo));

	int32_t nInodoQueLoContiene = obtenerInodoQueLoContiene(path);
	if (nInodoQueLoContiene == -1) {

		return 3;
	}
	subirOperacion(nInodoQueLoContiene, 1);

	inodoQueLoContiene = leerInodoDeTabla(nInodoQueLoContiene);

	inodoAeliminar = borrarEntradaDelDirectorio(nombre, inodoQueLoContiene, 2,
			nInodoQueLoContiene);
	if (inodoAeliminar == -1) {
		liberarMutexOperacion(nInodoQueLoContiene);

		return 5;
	}

	char* pathContiene = obtenerpathQueLoContiene(path);
	eliminarValor('2', pathContiene);

	subirOperacion(inodoAeliminar, 1);

	Ext2Inodo* inodoEliminar = NULL; // = (Ext2Inodo*) malloc(sizeof(Ext2Inodo));
	inodoEliminar = leerInodoDeTabla(inodoAeliminar);
	liberarTodosLosBloques(inodoEliminar);
	liberarInodo(inodoAeliminar);
	actualizarSuperBloque(0, 0, 1, 0);

	eliminarValor('1', path);

	escribirInodoTabla(nInodoQueLoContiene, inodoQueLoContiene);

	free(inodoEliminar);

	liberarMutexOperacion(inodoAeliminar);
	liberarMutexOperacion(nInodoQueLoContiene);

	printf("termino \n");
	return 1;

}

/** ______________________ Eliminar Archivo ________________________________**/

int32_t eliminarArchivo(char* path) {

	char*nombre = dameElNombre(path);

	int32_t inodoAeliminar;
	Ext2Inodo *inodoQueLoContiene = NULL; // = malloc(sizeof(Ext2Inodo));

	int32_t nInodoQueLoContiene = obtenerInodoQueLoContiene(path);
	if (nInodoQueLoContiene == -1) {
		return 3;
	}

	inodoQueLoContiene = leerInodoDeTabla(nInodoQueLoContiene);
	subirOperacion(nInodoQueLoContiene, 1);

	inodoAeliminar = borrarEntradaDelDirectorio(nombre, inodoQueLoContiene, 1,
			nInodoQueLoContiene);
	if (inodoAeliminar == -1) {
		liberarMutexOperacion(nInodoQueLoContiene);
		return 3;
	}

	char* pathContiene = obtenerpathQueLoContiene(path);
	eliminarValor('2', pathContiene);

	subirOperacion(inodoAeliminar, 1);

	Ext2Inodo* inodoEliminar = NULL;
	inodoEliminar = leerInodoDeTabla(inodoAeliminar);
	liberarTodosLosBloques(inodoEliminar);
	liberarInodo(inodoAeliminar);
	actualizarSuperBloque(0, 0, 1, 0);

	eliminarValor('1', path);
	escribirInodoTabla(nInodoQueLoContiene, inodoQueLoContiene);

	free(inodoEliminar); // NUEVOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
	free(nombre);

	liberarMutexOperacion(inodoAeliminar);
	liberarMutexOperacion(nInodoQueLoContiene);

	printf("termino \n");
	return 1;

}

/**_____________________Agregar entrada ___________________**/

void actualizarLinks(int32_t inode) {
	Ext2Inodo *inodo; //= (Ext2Inodo*) calloc(1, sizeof(Ext2Inodo));
	inodo = leerInodoDeTabla(inode);
	inodo->linksCount += 1;
	escribirInodoTabla(inode, inodo);
}

int32_t insertarEntrada(Ext2Directory * directorio, Ext2Inodo* inodoDirectorio,
		char * nombreNuevo, int32_t nroInodoQueLoContiene) {
	int32_t i = 1;
	int32_t resultado = -1;
	int32_t offset = 0;
	extern int32_t TAM_BLOCK;
	Ext2Directory *directorioActual = NULL;

	int32_t cantDeBloqueDatos = (inodoDirectorio->size / TAM_BLOCK);
	if ((inodoDirectorio->size % TAM_BLOCK) != 0)
		cantDeBloqueDatos++;

	int32_t loInserto = 0;
	int32_t esNuevo = 0;

	int32_t bloqueDato;
	int32_t tamanioVerdadero = 0;
	int32_t disponible = 0;
	char* buffer = NULL;
	bloqueDato = leerBloqueDeDato(i, inodoDirectorio);
	do {
		buffer = (char*) calloc(TAM_BLOCK, sizeof(char));


		if (bloqueDato == 0) {
			bloqueDato = buscarBloqueLibre(i);
			if (bloqueDato == -1) {
				return 4;
			}

			actualizarSuperBloque(0, 1, 0, 0);
			asignarBloqueEnInodo(i, bloqueDato, inodoDirectorio,
					nroInodoQueLoContiene);
			esNuevo = 1;
			offset = 0;
			actualizarSize(nroInodoQueLoContiene, inodoDirectorio,
					inodoDirectorio->size + TAM_BLOCK);

		}

		leerBloqueDisco(bloqueDato, buffer, 1);

		if (!esNuevo) {

			directorioActual = (Ext2Directory *) buffer;

			while (offset < TAM_BLOCK) {

				tamanioVerdadero = 8 + directorioActual->name_len;
				while ((tamanioVerdadero % 4) != 0)
					tamanioVerdadero++;

				offset += directorioActual->rec_len;

				if (tamanioVerdadero != (directorioActual->rec_len)) {

					disponible = directorioActual->rec_len - tamanioVerdadero;

					if (directorio->rec_len <= disponible) {
						directorio->rec_len = disponible;
						directorioActual->rec_len = tamanioVerdadero;
						memcpy(buffer + (offset - disponible),
								(char*) directorio, directorio->rec_len);
						loInserto = 1;
						break;
					}
				}
				directorioActual = (Ext2Directory *) (buffer + offset);

			}
		} else {
			int32_t n = directorio->rec_len;
			directorio->rec_len = TAM_BLOCK;
			memcpy(buffer, (char*) directorio, n);
			loInserto = 1;
		}

		if (loInserto) {
			escribirBloqueDisco(bloqueDato, 0, TAM_BLOCK, buffer);
			free(buffer);
			resultado = 1;
			break;
		}

		i++;
		offset = 0;
		bloqueDato = leerBloqueDeDato(i, inodoDirectorio);
		free(buffer);
		buffer=NULL;
		resultado = -1;


	} while (!loInserto);


	return resultado;

}

int32_t agregarEntradaEnElDirectorio(Ext2Inodo *inodoQueLoContiene,
		int32_t nroInodo, char *nombre, uint8_t type,
		int32_t nroInodoDelDirectorio) {

	Ext2Directory *directorio = NULL;
	directorio = calloc(1, sizeof(struct Ext2Directory) + strlen(nombre) + 1);
	int32_t len = 8 + strlen(nombre);
	while ((len % 4) != 0) // tiene que ser multiplo de 4
		len++;

	directorio->inode = nroInodo;
	directorio->rec_len = len;
	directorio->name_len = strlen(nombre);
	directorio->file_type = type;
	//strcat(directorio->name, nombre);
	memcpy(directorio->name, nombre, strlen(nombre));

	return insertarEntrada(directorio, inodoQueLoContiene, nombre,
			nroInodoDelDirectorio);

}

/**_________________ Dame el nombre ___________________________**/

char* dameElNombre(char *path) {

	if (strcmp(path, "/") == 0)
		return "Raiz";

	char * ultimo = rindex(path, '/');
	char *nombre = NULL;
	nombre = calloc(strlen(ultimo), sizeof(char));
	memcpy(nombre, ultimo + 1, strlen(ultimo));

	return nombre;

}

/**_________________ Crear Directorio _________________________**/

void agregarlesLasEntradasPyPP(int32_t nroInodo, int32_t padre, int32_t modo) {

	Ext2Directory *punto = NULL;
	punto = (Ext2Directory*) malloc(sizeof(Ext2Directory) + 1);

	Ext2Directory *puntoPunto = NULL;
	puntoPunto = (Ext2Directory*) malloc(sizeof(Ext2Directory) + 2);

	extern int32_t TAM_BLOCK;
	punto->inode = nroInodo;
	punto->rec_len = 12;
	punto->name_len = 1;
	punto->file_type = 0;
	memcpy(punto->name, ".", 1);

	puntoPunto->inode = padre;
	puntoPunto->rec_len = (TAM_BLOCK - punto->rec_len);
	puntoPunto->name_len = 2;
	puntoPunto->file_type = 0;
	memcpy(puntoPunto->name, "..", 2);

	int32_t nroBloque = buscarBloqueLibre(nroInodo);
	actualizarSuperBloque(0, 1, 0, 0);
	char* buff = (char*) malloc(TAM_BLOCK);
	memset(buff, 0, TAM_BLOCK);

	memcpy(buff, (char*) punto, 12);
	memcpy(buff + 12, (char*) puntoPunto, 12);

	escribirBloqueDisco(nroBloque, 0, TAM_BLOCK, buff);

	Ext2Inodo *inodo;
	inodo = leerInodoDeTabla(nroInodo);

	asignarBloqueEnInodo(1, nroBloque, inodo, nroInodo);

	inodo->mode = S_IFDIR | modo;
	inodo->uid = 1000;
	inodo->size = TAM_BLOCK;
	inodo->linksCount = 2;

	escribirInodoTabla(nroInodo, inodo);

	free(buff);
	free(punto);
	free(puntoPunto);

}

int32_t crearDirectorio(char* path, int32_t modo) {

	int32_t esta = leerSubDirectorios(path);
	if (esta == -1) {
		char*nombre = dameElNombre(path);

		int32_t nInodoQueLoContiene = obtenerInodoQueLoContiene(path);
		if (nInodoQueLoContiene == -1) {
			return 3;
		}

		char* pathContiene = obtenerpathQueLoContiene(path);
		eliminarValor('2', pathContiene);

		subirOperacion(nInodoQueLoContiene, 1);

		Ext2Inodo *inodoQueLoContiene;

		inodoQueLoContiene = leerInodoDeTabla(nInodoQueLoContiene);
		int32_t nroInodo = buscarInodolibre();
		if (nroInodo == -1) {
			liberarMutexOperacion(nInodoQueLoContiene);
			return 4;

		}
		int32_t res = agregarEntradaEnElDirectorio(inodoQueLoContiene, nroInodo,
				nombre, 2, nInodoQueLoContiene);

		if (res != -1) {
			agregarlesLasEntradasPyPP(nroInodo, nInodoQueLoContiene, modo);
			actualizarSuperBloque(0, 0, 0, 1);
		}

		free(inodoQueLoContiene); //ESTAAAA
		free(nombre);

		liberarMutexOperacion(nInodoQueLoContiene);

		printf("termino \n");
		return 1;
	}
	printf("Ya existe un archivo/directorio con ese nombre \n");
	return 2;

}

/**________________________ Crear Archivo __________________________**/

void agregarDatosIniciales(int32_t nroInodo, int32_t modo) {
	Ext2Inodo *inodo = NULL; //= (Ext2Inodo*) calloc(1, sizeof(Ext2Inodo));
	inodo = leerInodoDeTabla(nroInodo);

	inodo->mode = S_IFREG | modo;
	inodo->uid = 1000;
	inodo->size = 0;
	inodo->linksCount = 1;

	escribirInodoTabla(nroInodo, inodo);

}

int32_t crearArchivo(char* path, int32_t modo) {
	int32_t esta = leerSubDirectorios(path);
	if (esta == -1) {

		char*nombre = dameElNombre(path);
		int32_t nInodoQueLoContiene = obtenerInodoQueLoContiene(path);
		if (nInodoQueLoContiene == -1) {
			return 3;
		}

		char* pathContiene = obtenerpathQueLoContiene(path);
		eliminarValor('2', pathContiene);

		subirOperacion(nInodoQueLoContiene, 1);

		Ext2Inodo *inodoQueLoContiene = NULL;

		inodoQueLoContiene = leerInodoDeTabla(nInodoQueLoContiene);

		int32_t nroInodo = buscarInodolibre();
		if (nroInodo == -1) {
			liberarMutexOperacion(nInodoQueLoContiene);
			return 4;

		}

		int32_t res = agregarEntradaEnElDirectorio(inodoQueLoContiene, nroInodo,
				nombre, 1, nInodoQueLoContiene);
		if (res != -1) {
			agregarDatosIniciales(nroInodo, modo);
			actualizarSuperBloque(0, 0, 0, 1);
		}

		free(inodoQueLoContiene);
		free(nombre);
		printf("termino\n");

		liberarMutexOperacion(nInodoQueLoContiene);

		return nroInodo;
	}
	printf("Ya existe un archivo/directorio con ese Nombre\n");

	return 2;

}
