#include "libFSC.h"

void inicializarFscTest() {
	pthread_mutex_init(&mutexConexiones, NULL);
	inicializarMemcached();
	crearPoolConexionesFSC(ip, puerto);
}

int pepe(int argc, char *argv[]) {
	//abrirConfig();
	//inicializarFscTest();
	getAttrCache *paqueteCache = (getAttrCache *)malloc(sizeof(getAttrCache));

	char* buff=NULL;
	paqueteCache->mode = 2;
				paqueteCache->size =0;
				paqueteCache->links = 5;
				buff=serializarGetAttrCache(paqueteCache);
				printf("%s",buff);
				memset(paqueteCache,0,sizeof(getAttrCache));
				deserializarGetAttrCache(buff,paqueteCache);
				printf("%d\n%d\n%d\n",paqueteCache->mode,paqueteCache->links,paqueteCache->size);

	//NIPC respuesta;
	//respuesta = enviarOperacion(1, (char*) path, 0, 0, NULL);

	return 1;
}
