/*
 * Motor.h
 *
 *  Created on: 05/05/2012
 *      Author: utnso
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#include<stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/mman.h>
#include  <mcheck.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <limits.h>
#include  <sys/mman.h>
#include <memcached/engine.h>
#include <memcached/visibility.h>
#include <memcached/config_parser.h>
#include <memcached/types.h>
#include <pthread.h>
#include <time.h>
#include "archivo_Configuracion.h"

#include "log.h"




/* Estructura para almacenar y representar un elemento almacenado en la cache*/

typedef struct {
		char key[41]; //** 40 tamaño max key mas 1 de tipo de operacion*/
		size_t tam_key;
		void *data;
		size_t tam_data;
		size_t tam_particion;
		int flags;
		bool stored;
		bool libre; //para saber si es agujero o no
		struct timespec ultimo_tiempo_uso; //para el LRU o FIFO
		int pos; //para saber en que pos del vector esta el struct
		rel_time_t tiempo_exp;

} estructura_elemento_cache;


/*Estructura que representa el motor*/

typedef struct {
	ENGINE_HANDLE_V1 motor;
	GET_SERVER_API get_server_api;
    size_t tam_max_cache;
	size_t tam_min_bloque;
	int cant_max_elementos;
	int esquema;
	int alg_seleccion_victima;
	int alg_seleccion_libre;
	int cant_busquedas_fallidas;
	} estructura_motor;

typedef struct {
	   size_t cache_max_size;
	   size_t block_size_max;
	   size_t chunk_size;
	}t_dummy_ng_config;

t_log* flogger;

/*Funcion que usa memcached para ejecutar cuando levanta la shared library*/
MEMCACHED_PUBLIC_API ENGINE_ERROR_CODE create_intance(uint64_t interface, GET_SERVER_API get_server_api, ENGINE_HANDLE **handle);



#endif /* MOTOR_H_ */
