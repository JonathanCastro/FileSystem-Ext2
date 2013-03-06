#include "libFSC.h"

void* imprimirListado(char * lista, fuse_fill_dir_t filler) {
	void *buf = NULL;
	char *raiz = "/";
	uint32_t tamPath = (strlen(lista) + 1);
	char ruta[tamPath];
	memset(ruta, 0, tamPath);
	char* palabra;
	memcpy(ruta, lista, tamPath);

	palabra = strtok(ruta, "/");
	char * nombre;
	//printf(" %s\n",palabra);
	while (palabra != NULL) {

		nombre = calloc(strlen(palabra) + 1, sizeof(char));
		memcpy(nombre, palabra, sizeof(palabra));
		//nombre[sizeof(palabra)+1]='\0';
		filler(buf, nombre, NULL, 0);
		printf("%s\n", palabra);
		palabra = strtok(NULL, raiz);
		free(nombre);
	}
	return buf;
}

char* serializarGetAttrCache(getAttrCache *paquete) {

	int32_t tamanio = (3 * sizeof(int32_t)) + 4;
	char* buffer = (char*) malloc(tamanio);
	char modeS[10];
	char sizeS[10];
	char linksS[10];

	sprintf(sizeS, "%d", paquete->size);
	sprintf(modeS, "%d", paquete->mode);
	sprintf(linksS, "%d", paquete->links);
	strcpy(buffer, sizeS);
	strcat(buffer, "*");
	strcat(buffer, modeS);
	strcat(buffer, "*");
	strcat(buffer, linksS);
	strcat(buffer, "*");
	return buffer;

}

void deserializarGetAttrCache(char *buffer, getAttrCache *paquete) {
	int32_t base, offset = 0;
	char modeS[10];
	char sizeS[10];
	char linksS[10];

	for (offset = 0; buffer[offset] != '*'; offset++);

	memcpy(sizeS, buffer, offset );
		paquete->size = atoi(sizeS);


	offset++;
	for (base = offset; buffer[offset] != '*'; offset++)
		;

	memcpy(modeS, buffer + base, offset- base);
		paquete->mode = atoi(modeS);

	offset++;
	for (base = offset; buffer[offset] != '*'; offset++)
		;

	memcpy(linksS, buffer + base, offset - base);
	paquete->links = atoi(linksS);

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

