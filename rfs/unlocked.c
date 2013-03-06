#include "libRFS.h"
#define _GNU_SOURCE

FILE * ptr;

void abrirArchivoDisco() {
	ptr = fopen(pathDisco, "r+");

	if (ptr != NULL) {
		posix_fadvise((int) ptr, 0, 0, POSIX_FADV_RANDOM); /*revisar si es random*/
	} else {
		exit(1);
	}
}

int leerSuperBloque(int bloque, char*buffLectura,int cantB){
	size_t result;
	char bloqueLeer[1024];
	memset(bloqueLeer, 0, 1024);
	extern pthread_mutex_t bloqueLeerBloques;
	pthread_mutex_lock(&bloqueLeerBloques);

	fseek(ptr, 1024, SEEK_SET);
	result = fread_unlocked(bloqueLeer, 1, 1024, ptr);

	if (result == (size_t) 1024) {
		memcpy(buffLectura, bloqueLeer, 1024);
		pthread_mutex_unlock(&bloqueLeerBloques);
		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);
	return 1;

}

int escribirSuperBloque(char*buffEscritura){
	size_t result;
	extern pthread_mutex_t bloqueEscribirBloques;
	pthread_mutex_lock(&bloqueEscribirBloques);

	fseek(ptr, 1024, SEEK_SET);
	result = fwrite_unlocked(buffEscritura, 1,1024, ptr);
	if (result == (size_t) 1024) {
			fflush_unlocked(ptr);
			pthread_mutex_unlock(&bloqueEscribirBloques);

			return 0;
		}
	pthread_mutex_unlock(&bloqueEscribirBloques);

		return 1;
}

int leerSuperBloque2(int bloque,Ext2SuperBloque *superBloque,int cantB){
	size_t result;
	char bloqueLeer[1024];
	extern pthread_mutex_t bloqueLeerBloques;
	memset(bloqueLeer, 0, 1024);
	pthread_mutex_lock(&bloqueLeerBloques);

	fseek(ptr, 1024, SEEK_SET);
	result = fread_unlocked(bloqueLeer, 1, 1024, ptr);

	if (result == (size_t) 1024) {

		memcpy(&superBloque, bloqueLeer, 1024);
		pthread_mutex_unlock(&bloqueLeerBloques);
		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);
	return 1;

}

int leerBloqueDisco(int32_t bloque, char *buffLectura,int cantB) {
	size_t result;
	extern int32_t TAM_BLOCK;
	extern pthread_mutex_t bloqueLeerBloques;
	//char *bloqueLeer=malloc(TAM_BLOCK);//[TAM_BLOCK*cantB];

	char *bloqueLeer=NULL;
	bloqueLeer=(char*) calloc(TAM_BLOCK*cantB, sizeof(char));
	memset(bloqueLeer, 0, TAM_BLOCK*cantB);


	pthread_mutex_lock(&bloqueLeerBloques);

	fseek(ptr, bloque * TAM_BLOCK, SEEK_SET);
	result = fread_unlocked(bloqueLeer, 1, TAM_BLOCK*cantB, ptr);

	if (result == (size_t) TAM_BLOCK*cantB) {
		memcpy(buffLectura, bloqueLeer, TAM_BLOCK*cantB);
		free(bloqueLeer);
		pthread_mutex_unlock(&bloqueLeerBloques);
		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);
	free(bloqueLeer);
	return 1;
}

int leerTdescriptora2(int offset,char *buffLectura){
	size_t result;
	extern pthread_mutex_t bloqueLeerBloques;
	char *entradaALeer=malloc(32);
	memset(entradaALeer, 0, 32);

	pthread_mutex_lock(&bloqueLeerBloques);
	fseek(ptr, offset, SEEK_SET);
	result = fread_unlocked(entradaALeer, 1, 32, ptr);

	if (result == (size_t) 32) {
		memcpy(buffLectura, entradaALeer, 32);
		pthread_mutex_unlock(&bloqueLeerBloques);
		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);
	return 1;
}
int leerTdescriptora(int offset,char *buffLectura,int32_t cantGrupos){
	size_t result;
	extern pthread_mutex_t bloqueLeerBloques;
	char *entradaALeer=malloc(32*cantGrupos);
	memset(entradaALeer, 0, 32*cantGrupos);

	pthread_mutex_lock(&bloqueLeerBloques);
	fseek(ptr, offset, SEEK_SET);
	result = fread_unlocked(entradaALeer, 1, 32*cantGrupos, ptr);

	if (result == (size_t) (32*cantGrupos)) {
		memcpy(buffLectura, entradaALeer, 32*cantGrupos);
		pthread_mutex_unlock(&bloqueLeerBloques);
		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);
	return 1;
}
int escribirTdescriptora(int offset, char*buffEscritura,int32_t cantGrupos){
	size_t result;
	extern pthread_mutex_t bloqueEscribirBloques;
	pthread_mutex_lock(&bloqueEscribirBloques);
	fseek(ptr, offset, SEEK_SET);
	result = fwrite_unlocked(buffEscritura, 1, (32*cantGrupos), ptr);
	if (result == (size_t)(32*cantGrupos)) {
			fflush_unlocked(ptr);
			pthread_mutex_unlock(&bloqueEscribirBloques);

			return 0;
		}
	pthread_mutex_unlock(&bloqueEscribirBloques);

		return 1;
}


int leerInodo(int offset,char *buffLectura){
	size_t result;
	char *inodoALeer=malloc(128);
	memset(inodoALeer, 0, 128);
	extern pthread_mutex_t bloqueLeerBloques;
	pthread_mutex_lock(&bloqueLeerBloques);

	fseek(ptr, offset, SEEK_SET);
	result = fread_unlocked(inodoALeer, 1, 128, ptr);

	if (result == (size_t) 128) {
		memcpy(buffLectura, inodoALeer, 128);
		pthread_mutex_unlock(&bloqueLeerBloques);

		return 0;
	}
	pthread_mutex_unlock(&bloqueLeerBloques);

	return 1;
}

int escribirInodo(int offset, char*buffEscritura){
	size_t result;
	extern pthread_mutex_t bloqueEscribirBloques;
	pthread_mutex_lock(&bloqueEscribirBloques);
	fseek(ptr, offset, SEEK_SET);
	result = fwrite_unlocked(buffEscritura, 1, 128, ptr);
	if (result == (size_t) 128) {
			fflush_unlocked(ptr);
			pthread_mutex_unlock(&bloqueEscribirBloques);

			return 0;
		}
	pthread_mutex_unlock(&bloqueEscribirBloques);

		return 1;
}

int escribirBloqueDisco(int bloque,int offet,int size ,char* buffEscritura) {
	size_t result;
	extern pthread_mutex_t bloqueEscribirBloques;
	pthread_mutex_lock(&bloqueEscribirBloques);
	extern int32_t TAM_BLOCK;
	fseek(ptr, (bloque * TAM_BLOCK) +offet , SEEK_SET);
	result = fwrite_unlocked(buffEscritura, 1, size, ptr);
	if (result == (size_t) size) {
		fflush_unlocked(ptr);
		pthread_mutex_unlock(&bloqueEscribirBloques);

		return 0;
	}
	pthread_mutex_unlock(&bloqueEscribirBloques);

	return 1;
}
