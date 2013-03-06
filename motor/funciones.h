#include <stdio.h>
#include <math.h>
#include "Motor.h"

#define LRU 0
#define FIFO 1
#define PARTICIONES_DINAMICAS 0
#define BUDDY 1
#define BEST 0
#define FIRST 1
#define CLOCK_MONOTONIC 1


void  inicializo_vector(estructura_elemento_cache* vector_cache, void* cache, size_t tam_max_cache, int cant_max_elementos);
void habilito_nueva_particion(estructura_elemento_cache* vector_cache, int pos,void* direccion, int cant_max_elementos, int tam_max_cache);
void  ordeno_vector(estructura_elemento_cache* vector_cache, int pos, int nueva_pos);
void  agrego_al_vector(estructura_elemento_cache* vector_cache, char* strkey, estructura_elemento_cache* it);
int   algoritmoBestFit(estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos);
int   algoritmoFirstFit(estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos);
void  imprime_cache(estructura_elemento_cache* vector_cache, int cant_max_elementos);
int buscarElementoVector(estructura_elemento_cache* vector_cache, char* key, int cant_max_elementos);
void  vector_clean(estructura_elemento_cache* vector_cache);
int   buscarPosVector(estructura_elemento_cache* vector_cache, char* key);
int  algoritmoFifo(estructura_elemento_cache* vector_cache, size_t tam_data);
int  algoritmoLru(estructura_elemento_cache* vector_cache, size_t tam_data);
void compactacion(estructura_elemento_cache* vector_cache,int cant_max_elementos, int tam_max_cache);
int eliminarParticion(estructura_motor* motor, estructura_elemento_cache* vector_cache);
void habilito_nueva_particion_buddy(estructura_elemento_cache* vector_cache, int pos);
void unionParticiones(estructura_elemento_cache* vector_cache, int pos, int cant_max_elementos);

int esquemaParticionesDinamicas(estructura_motor* motor, estructura_elemento_cache* vector_cache, size_t tam_data);
int esquemaBuddySystem(estructura_motor* motor,estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos);
int seleccionParticion(estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos);

int potencia_dos (int numero1, int numero2);

