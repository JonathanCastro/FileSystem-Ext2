#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include "libRFS.h"
#include "ext2.h"

#include "bitarray.h"
#include "string.h"

/**_______ ____Poll conexiones____________ **/
int32_t *vectorSockets;
poolMutex *poolConexiones;
pthread_rwlock_t bloqueaPoll;
pthread_rwlock_t cantConex;
pthread_mutex_t mutexPedidosActivos;
sem_t semaforoLista;
int32_t pedidosActivos;

/**____________ Sincronizacion __________ **/
t_listaIntocables *listaIntocables;
pthread_rwlock_t semIntocables;

/**_____________Pedidos __________________**/
int32_t cantOperacionesEnCola;
pthread_mutex_t bloqueaLista;
t_listaPedidos *listaPedidosRFS;
t_listaPedidos *listaEspera;
pthread_mutex_t bloqueaListaEspera;

/**____________ Estructuras ______________**/
Ext2SuperBloque *superBloque;
Ext2TablaDeDescriptores *tablaDescriptora;
int32_t cantGrupos;

/**___________ Semaforos__________**/

pthread_mutex_t BitMapBloque;
pthread_mutex_t BitMapInodo;
pthread_mutex_t SuperBloque;
/*______log___*/

t_log * loguear;

/**_________ Poll Hilos _________**/
t_pollThread *pollHilos;
pthread_rwlock_t mutexHilos;

/**__________ memcached _____________ **/
memcached_st *memcached;

/**__________Variables Globales_________**/

int retardo;
int32_t cantPunteros;
pthread_attr_t attr;
int32_t TAM_BLOCK;
int32_t BLOQXPUNT;
bitmapsIndices *indiceBitmap;
pthread_mutex_t bloqueLeerBloques;
pthread_mutex_t bloqueEscribirBloques;
pthread_mutex_t bloqueoCache;
pthread_rwlock_t mutexSuperBloque;
pthread_rwlock_t mutexTablaDescriptora;



void inicializarRFS() {

	loguear = log_create("log.txt", "rfs", 1, LOG_LEVEL_INFO | LOG_LEVEL_DEBUG);

	pedidosActivos = 0;

	pthread_mutex_init(&mutexPedidosActivos, NULL);
	pthread_mutex_init(&BitMapBloque, NULL);
	pthread_mutex_init(&BitMapInodo, NULL);
	pthread_mutex_init(&SuperBloque, NULL);
	pthread_mutex_init(&bloqueLeerBloques, NULL);
	pthread_mutex_init(&bloqueEscribirBloques, NULL);
	pthread_mutex_init(&bloqueoCache, NULL);

	pthread_rwlock_init(&mutexHilos, NULL);
	pthread_rwlock_init(&bloqueaPoll, NULL);
	pthread_rwlock_init(&semIntocables, NULL);
	pthread_rwlock_init(&mutexSuperBloque, NULL);
	pthread_rwlock_init(&mutexTablaDescriptora, NULL);

	/** Attr para hilos Operaciones  **/

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	sem_init(&semaforoLista, 0, 0);
	abrirArchivoDisco();
	superBloque = (Ext2SuperBloque*) malloc(sizeof(Ext2SuperBloque));
	leerSuper_Bloque();

	switch (superBloque->s_log_block_size) {
	case 0:
		TAM_BLOCK = 1024;
		break;
	case 1:
		TAM_BLOCK = 2048;
		break;
	case 2:
		TAM_BLOCK = 4096;
		break;
	default:
		break;
	}

	cantGrupos = (superBloque->s_blocks_count
			/ superBloque->s_blocks_per_group);

	if ((superBloque->s_blocks_count % superBloque->s_blocks_per_group) != 0)
		cantGrupos++;

	tablaDescriptora = malloc (cantGrupos*sizeof(Ext2TablaDeDescriptores));
	leerTablaDeDescriptores();
	BLOQXPUNT = TAM_BLOCK / 4;
	cantPunteros = TAM_BLOCK / 4;
	inicializarMemcached();

}

int main() {

	printf(" _____ _          _              _  _____ _                \n");
	printf("|_   _| |__   ___| |    __ _ ___| ||_   _(_)_ __ ___   ___  \n");
	printf(
			"  | | | '_ \\ / _ \\ |   / _` / __| __|| | | | '_ ` _ \\ / _ \\  \n");
	printf(
			"  | | | | | |  __/ |__| (_| \\__ \\ |_ | | | | | | | | |  __/   \n");
	printf(
			"  |_| |_| |_|\\___|_____\\__,_|___/\\__||_| |_|_| |_| |_|\\___|   \n");

	printf("                                      ____   __        \n");
	printf("                                     |  _ \\ / _|___      \n");
	printf("                                     | |_) | |_/ __|     \n");
	printf("                                  _  |  _ <|  _\\__ \\  _    \n");
	printf("                                 (_) |_| \\_\\_| |___/ (_)  \n\n ");

	pthread_t idResponder;
	pthread_attr_t attrR;
	abrirConfig();

	inicializarRFS();

	listaPedidosRFS = crearListaPedidos();
	listaIntocables = crearListaIntocables();

	/*____________________________________*/


	pollHilos = calloc(cantOperaciones, sizeof(t_pollThread));
	pthread_attr_init(&attrR);
	pthread_attr_setdetachstate(&attrR, PTHREAD_CREATE_DETACHED);
	pthread_create(&idResponder, NULL, (void*) responderPedidos, NULL); // creo el hilo que responderá operaciones

	crearPollConexiones();

	log_destroy(loguear);

	pthread_join(idResponder, NULL);
	pthread_attr_destroy(&attrR);
	free(pollHilos);
	return 1;

}

