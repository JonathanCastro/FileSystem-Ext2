#include "Motor.h"
#include "archivo_Configuracion.h"
#include "funciones.h"


/*************************** Dummy Functions **************************
 * Estas funciones son dummy, son necesarias para que el engine las tengas
 * pero no tienen logica alguna y no seran necesarias implementar
  */

static ENGINE_ERROR_CODE dummy_ng_get_stats(ENGINE_HANDLE* , const void* cookie, const char* stat_key, int nkey, ADD_STAT add_stat);
static void dummy_ng_reset_stats(ENGINE_HANDLE* , const void *cookie);
static ENGINE_ERROR_CODE dummy_ng_unknown_command(ENGINE_HANDLE* , const void* cookie, protocol_binary_request_header *request, ADD_RESPONSE response);
static void dummy_ng_item_set_cas(ENGINE_HANDLE *, const void *cookie, item* item, uint64_t val);
static const engine_info* dummy_ng_get_info(ENGINE_HANDLE* );

/*Funciones dummy a implementar*/

static ENGINE_ERROR_CODE inicializar_cache(ENGINE_HANDLE* handle , const char* config_str);
static void destruir_motor(ENGINE_HANDLE *handle, const bool force);
static ENGINE_ERROR_CODE alocar(ENGINE_HANDLE* , const void* cookie, item **item, const void* key,const size_t tam_key, const size_t tam_data, const int flags, rel_time_t tiempo_exp);
static bool get_elemento_info(ENGINE_HANDLE *, const void *cookie, const item* item, item_info *item_info);
static ENGINE_ERROR_CODE get(ENGINE_HANDLE *, const void* cookie, item** item, const void* key,const int tam_key, uint16_t vbucket);
static ENGINE_ERROR_CODE store(ENGINE_HANDLE *, const void* cookie, item* item, uint64_t *cas, ENGINE_STORE_OPERATION opertation, uint16_t vbucket);
static ENGINE_ERROR_CODE elemento_delete(ENGINE_HANDLE* , const void* cookie, const void* key, const size_t tam_key, uint64_t cas, uint16_t vbucket);
static void elemento_release(ENGINE_HANDLE* , const void *cookie, item* item);
static ENGINE_ERROR_CODE flush(ENGINE_HANDLE *, const void* cookie, time_t when);

void cache_dump(int);

pthread_rwlock_t milock, milock_get;


estructura_elemento_cache* vector_cache; //la estructura para administrar sera un vector de tantas posiciones como cant max de elementos haya.
void* cache;
int cant_max_elementos;




MEMCACHED_PUBLIC_API ENGINE_ERROR_CODE create_instance(uint64_t interface, GET_SERVER_API get_server_api, ENGINE_HANDLE **handle) {

    if (interface == 0) {
		return ENGINE_ENOTSUP;
	}


	estructura_motor *motor = calloc(1, sizeof(estructura_motor));
	if (motor == NULL) {
		return ENGINE_ENOMEM;
	}


	motor->motor.interface.interface = 1;

	motor->motor.initialize = inicializar_cache;
	motor->motor.destroy = destruir_motor;
	motor->motor.get_info = dummy_ng_get_info;
	motor->motor.allocate = alocar;
	motor->motor.remove = elemento_delete;
	motor->motor.release = elemento_release;
	motor->motor.get = get;
	motor->motor.get_stats = dummy_ng_get_stats;
	motor->motor.reset_stats = dummy_ng_reset_stats;
	motor->motor.store = store;
	motor->motor.flush = flush;
	motor->motor.unknown_command = dummy_ng_unknown_command;
	motor->motor.item_set_cas = dummy_ng_item_set_cas;
	motor->motor.get_item_info = get_elemento_info;

	motor->get_server_api = get_server_api;

	/*
	 * memcached solo sabe manejar la estructura ENGINE_HANDLE
	 * el cual es el primer campo de nuestro t_dummy_ng
	 * El puntero de engine es igual a &engine->engine
	 *
	 * Retornamos nuestro engine a traves de la variable handle
	 */
	*handle = (ENGINE_HANDLE*) motor;

	/* creo la cache de almacenamiento */



	return ENGINE_SUCCESS;
}



/*---------------------------------------------------
 * 					INITIALIZE
 *-----------------------------------------------------
 */
/*lee el archivo de configuracion*/
/* pide memoria para la cache, el motor y estructuras administrativas y las inicializa */
static ENGINE_ERROR_CODE inicializar_cache(ENGINE_HANDLE* handle , const char* config_str)
{


	/* ENGINE_HANDLE* handle seria el puntero de nuestra estructura_motor*/
	estructura_motor *motor = (estructura_motor*)handle;

	mtrace(); //para registrar los mallocs

	// archivo log
   flogger= malloc(sizeof(t_log));
   flogger= log_create(flogger, "/home/utnso/2012-1c-thelasttime/motor/logCache.txt","motor",0, LOG_LEVEL_DEBUG);

struct config_item items[] = {
	{	.key = "cache_size",
		.datatype = DT_SIZE,
		.value.dt_size = &motor->tam_max_cache},
	{	.key = "chunk_size",
		.datatype = DT_SIZE,
		.value.dt_size = &motor->tam_min_bloque},
	{	.key = "item_size_max",
		.datatype = DT_SIZE,
		.value.dt_size = &motor->tam_max_cache},
	{	.key = NULL}
};

parse_config(config_str, items, NULL);


//	//borrar esto para la entrega
//	motor->tam_max_cache=32;
//	motor->tam_min_bloque=8;

	int es_pot_dos= potencia_dos (motor->tam_max_cache, motor->tam_min_bloque );

	//calculo la cantidad d elementos que trendra el vector;
	motor->cant_max_elementos = (motor->tam_max_cache/motor->tam_min_bloque);

	//leo el resto de los datos del archivo de configuracion, si no mandan lo esperado no inicializo nada.
	int error = abrirConfig();
	motor->cant_busquedas_fallidas= cantBusqFallidas;
	motor->alg_seleccion_victima= algEleccionVictima;
	motor->alg_seleccion_libre= algEleccionLibre;
	motor->esquema= esquema;

	if((error == -1) || (motor->esquema==1 && es_pot_dos==-1)){

		log_debug(flogger,"Parametros erroneos en el parse/arch. de conf.");
		return EXTENSION_LOG_WARNING;
	}

	   //en base los parametros recibidos, pido memoria para almacenar los datos
    		cache = malloc(motor->tam_max_cache);

    		//la estructura para administrar sera un vector de tantas posiciones como cant max de elementos haya.
    		vector_cache = (estructura_elemento_cache*) malloc((motor->cant_max_elementos)*sizeof(estructura_elemento_cache));

     		//inicializo el rwlock
    		pthread_rwlock_init (&milock, NULL);

    		mlock(cache, motor->tam_max_cache); //para que no pagine en disco
    		mlock(vector_cache, (motor-> cant_max_elementos)*sizeof(estructura_elemento_cache));

    		inicializo_vector(vector_cache, cache, (*motor).tam_max_cache, (*motor).cant_max_elementos);

    		cant_max_elementos = motor->cant_max_elementos;

    		signal(SIGUSR1, cache_dump);

    		log_debug(flogger,"Se inicializo el motor Correctamente");

	return ENGINE_SUCCESS;
}


//funcion que llama signal para imprimir lo de la cache
void cache_dump( int signal){


	int j=0;
	int i=0;
	FILE * fdummp;

	fdummp = fopen ("/home/utnso/2012-1c-thelasttime/motor/dummpCache.txt","a");

    fprintf(fdummp, "----------------------------------------------------- \n\n\n");
    fprintf(fdummp, "DUMMP:  \n");

	pthread_rwlock_rdlock(&milock);
	//para todos los nodos activos, los imprime
	while ((vector_cache[j].data != NULL) && (cant_max_elementos > j)){

			fprintf(fdummp, "Partición %d: %p - %p. %s . size: %d bytes. ",
					j+1,
					vector_cache[j].data,
					vector_cache[j].data + vector_cache[j].tam_particion,
					(vector_cache[j].libre==true)? "Libre": "Ocupada",
					vector_cache[j].tam_particion);
			if (algEleccionVictima==LRU && vector_cache[j].libre==false)
				fprintf(fdummp, " LRU: %lu. ", vector_cache[j].ultimo_tiempo_uso.tv_sec);
			if(vector_cache[j].libre== false){
				fprintf(fdummp, " Key: ");
				for(i=0;i<vector_cache[j].tam_key;i++)
					fprintf(fdummp, "%c",vector_cache[j].key[i]);

				}
			fprintf(fdummp,"\n");
		j++;
	}
	pthread_rwlock_unlock(&milock);
	fclose (fdummp);

} //fin de imprime_cache


/*---------------------------------------------------
 * 	Alocate: puedo guardar tal data en mi cache? si no hay fallo, dps llamo a stored y lo guardo
 *-----------------------------------------------------
 */
static ENGINE_ERROR_CODE alocar(ENGINE_HANDLE *handler, const void* cookie, item **it, const void* key,
											const size_t tam_key, const size_t tam_data, const int flags, const rel_time_t tiempo_exp)
{
	 estructura_motor *motor = (estructura_motor*)handler;

	 int pos;
     char strkey[41];
  	 memset(strkey, 0, 41);
  	 memcpy (strkey, key, tam_key);

  	// si el dato q me quieren mandar es mas grande q la cache ya se que no entra asiq no hago nada
  		 if(tam_data > motor->tam_max_cache){

  			 log_debug(flogger,"La key %s no pudo ser alocada xq su valor supera el tam. max. de la cache", strkey );
  			 return ENGINE_E2BIG;
  		 }

  	 // abre semaforo
	 pthread_rwlock_wrlock(&milock);

     	pos= buscarElementoVector(vector_cache,strkey, (*motor).cant_max_elementos);

     	 if(pos!=-1 && vector_cache[pos].tam_data== tam_data){ //Si existe la key y tiene el mismo tamaño. Piso los datos.

     		struct timespec tiempo_actual;
        	clock_gettime(CLOCK_MONOTONIC, &tiempo_actual);


        	vector_cache[pos].flags = flags;
        	vector_cache[pos].tiempo_exp = tiempo_exp;
        	vector_cache[pos].tam_key = tam_key;
        	memset(vector_cache[pos].key,0,41);
        	memcpy(vector_cache[pos].key,key,tam_key);
            vector_cache[pos].stored = false;
        	vector_cache[pos].libre=false; //para saber si es agujero o no
        	vector_cache[pos].ultimo_tiempo_uso= tiempo_actual ; //para el LRU o FIFO
        	vector_cache[pos].pos=pos; //para saber en que pos del vector esta el struct


            *it= ((item*)&vector_cache[pos]);

            // cierra semaforo
            //pthread_rwlock_unlock(&milock);
            log_debug(flogger,"Se actualizo el valor de la key: %s - código de operación: %c  ", strkey,strkey[0] );
        	return ENGINE_SUCCESS;

        } //Fin si existe la key
        else{

        	//si existe pero es otro tamaño la elimino e intento alocarlo nuevamente
        	if(pos!=-1) {
        		vector_cache[pos].libre=true;
        		vector_cache[pos].stored = false;
        		if (motor->esquema== BUDDY)
        			unionParticiones(vector_cache, pos, motor->cant_max_elementos);
        		log_debug(flogger,"Actualizando el tamaño/valor de la key: %s ", strkey);
        	}

        	//en principio el tamaño de la particion sera del tamaño del dato
        	int tam_particion = tam_data;

        	     if(tam_data <= (motor->tam_min_bloque) ){
        			     tam_particion= motor->tam_min_bloque;
        			     			     }

        	if((*motor).esquema== PARTICIONES_DINAMICAS){

        		     pos= esquemaParticionesDinamicas(motor,vector_cache, tam_particion);

        	    	 //si el espacio sobrante no es grande como para una nueva particion no habilito
       		        	if((vector_cache[pos].tam_particion - tam_particion) >= motor->tam_min_bloque){
       				    	   habilito_nueva_particion(vector_cache, pos, vector_cache[pos].data + tam_particion, motor->cant_max_elementos, motor->tam_max_cache);
       				     }
       					 else{
       					    	//sino que dejo la particion que ya tenia, creando fragmentacion interna.
       					    	tam_particion= vector_cache[pos].tam_particion;
       				    	    }

       		 } //fin particiones dinamicas
           else {
        	   //empieza buddy syetem
        	   pos= esquemaBuddySystem(motor,vector_cache, tam_particion, motor->cant_max_elementos);
        	   tam_particion= vector_cache[pos].tam_particion;
           } //fin Buddy

        				struct timespec tiempo_actual;
        				clock_gettime(CLOCK_MONOTONIC, &tiempo_actual);

        				vector_cache[pos].flags = flags;
        				vector_cache[pos].tiempo_exp = tiempo_exp;
        				vector_cache[pos].tam_key = tam_key;
        				memset(vector_cache[pos].key,0,41);
        				memcpy(vector_cache[pos].key,key,tam_key);
        				vector_cache[pos].tam_data = tam_data;
        				vector_cache[pos].tam_particion= tam_particion;
        			    vector_cache[pos].stored = false;
        				vector_cache[pos].libre=false; //para saber si es agujero o no
        				vector_cache[pos].ultimo_tiempo_uso= tiempo_actual ; //para el LRU o FIFO
        				vector_cache[pos].pos=pos; //para saber en que pos del vector esta el struct

        			    *it= ((item*)&vector_cache[pos]);

        			   log_debug(flogger,"key alocada: %s - código de operación: %c  ", strkey,strkey[0] );
        			   // pthread_rwlock_unlock(&milock);
        				return ENGINE_SUCCESS;


        	 } //fin inserte dato nuevo

} //fin funcion





/*---------------------------------------------------
 * 	Stored: ahora si ya guardo la data en la cache y lo inserto en el vector
 *-----------------------------------------------------
 */
static ENGINE_ERROR_CODE store(ENGINE_HANDLE* handle, const void* cookie, item* item, uint64_t *cas, ENGINE_STORE_OPERATION opertation, uint16_t vbucket)
{
	//pthread_rwlock_wrlock(&milock);
	estructura_elemento_cache *it = (estructura_elemento_cache*) item;

	it->stored = true;


   *cas = 0;

   //pthread_rwlock_unlock(&milock);
   return ENGINE_SUCCESS;
}

/*---------------------------------------------------
 * 	Get: devuelve el puntero a donde esta el dato guardado en la cache
 *-----------------------------------------------------
 */

static ENGINE_ERROR_CODE get(ENGINE_HANDLE* handle, const void* cookie, item** it, const void* key,const int tam_key, uint16_t vbucket){



    estructura_motor *motor = (estructura_motor*)handle;

	char strkey[41];
	memset(strkey, 0, 41);
	memcpy (strkey, key, tam_key);

	pthread_rwlock_rdlock(&milock);
    int elem= buscarElementoVector(vector_cache,strkey, (*motor).cant_max_elementos);


	if(elem==-1){
		pthread_rwlock_unlock(&milock);
		//return ENGINE_NOT_STORED;
		return ENGINE_KEY_ENOENT;
	}

//mutex
	pthread_rwlock_wrlock(&milock_get);

    if((*motor).alg_seleccion_victima== LRU ){
    				struct timespec tiempo_actual;
    				clock_gettime(CLOCK_MONOTONIC, &tiempo_actual);
    	            vector_cache[elem].ultimo_tiempo_uso=tiempo_actual;
    }
    pthread_rwlock_unlock(&milock_get);
//mutex



	*it= (item*)(&vector_cache[elem]);
	//pthread_rwlock_unlock(&milock);

    return ENGINE_SUCCESS;

	}


/*---------------------------------------------------
 * 	Get elemento info: mapear el item_info el cual es el tipo que memcached sabe manejar con el tipo de item
 * nuestro el cual es el que nosotros manejamos
 *-----------------------------------------------------
 */
static bool get_elemento_info(ENGINE_HANDLE * handle, const void *cookie, const item* item, item_info *item_info){

	estructura_elemento_cache* elem=  (estructura_elemento_cache*)item;
	if(item_info->nvalue < 1 ){
		return false;
	}
	item_info->cas= 0;
	item_info->clsid=0;
	item_info->exptime= elem->tiempo_exp;
	item_info->flags= elem->flags;
	item_info->key= elem-> key;
	item_info-> nkey= elem-> tam_key;
	item_info-> nbytes= elem-> tam_data;
	item_info-> nvalue= 1;
	item_info->value[0].iov_base= elem-> data;
	item_info->value[0].iov_len= elem-> tam_data;

	return true;


}


/*---------------------------------------------------
 * 	Flush: limpia la cache
 *-----------------------------------------------------
 */
static ENGINE_ERROR_CODE flush(ENGINE_HANDLE * handle, const void* cookie, time_t when){

	estructura_motor *motor = (estructura_motor*)handle;

	void* cache= vector_cache[0].data; //para pasarle a la funcion de abajo

	pthread_rwlock_wrlock(&milock);
	inicializo_vector(vector_cache, cache, (*motor).tam_max_cache, (*motor).cant_max_elementos) ;
	pthread_rwlock_unlock(&milock);

	return ENGINE_SUCCESS;
}

/*---------------------------------------------------
 * Destruir motor: libera memoria
 *-----------------------------------------------------
 */

static void destruir_motor(ENGINE_HANDLE* handle, const bool force){
	log_debug(flogger,"Liberando memoria - Destruyendo Motor TheLastTime" );
	free(handle);
	free((void*) cache);
	free((estructura_elemento_cache*) vector_cache);
	free(flogger);
}

/*---------------------------------------------------
 * Elemento delete: borra un elemento de la cache
 *-----------------------------------------------------
 */
static ENGINE_ERROR_CODE elemento_delete(ENGINE_HANDLE* handle, const void* cookie, const void* key, const size_t tam_key, uint64_t cas, uint16_t vbucket)
{

		char strkey[41];
		memset(strkey, 0, 41);
		memcpy (strkey, key, tam_key);

		estructura_motor *motor = (estructura_motor*)handle;

		pthread_rwlock_wrlock(&milock);
		int pos= buscarElementoVector(vector_cache, strkey, (*motor).cant_max_elementos);


		if(pos==-1){
			pthread_rwlock_unlock(&milock);
			return ENGINE_NOT_STORED;
		}

		vector_cache[pos].stored = false;

		elemento_release(handle, NULL, (item*)&vector_cache[pos]);

		return ENGINE_SUCCESS;

}
static void elemento_release(ENGINE_HANDLE *handler, const void *cookie, item* item){

	estructura_motor *motor = (estructura_motor*)handler;
	estructura_elemento_cache* it= (estructura_elemento_cache*) item;

	//pthread_rwlock_wrlock(&milock);

	if(vector_cache[(it->pos)].stored == false){

		//sino esta storeado, liberamos la particion
		vector_cache[(it->pos)].libre = true;

		if (motor->esquema==BUDDY) //si el esquema es buddy system no alcanza solo con ponerla en free
			unionParticiones(vector_cache, it->pos, motor->cant_max_elementos);

	}
		pthread_rwlock_unlock(&milock);
}



/*
 * ************************************* Funciones Dummy *************************************
 */

static ENGINE_ERROR_CODE dummy_ng_get_stats(ENGINE_HANDLE* handle, const void* cookie, const char* stat_key, int nkey, ADD_STAT add_stat) {
	return ENGINE_SUCCESS;
}

static void dummy_ng_reset_stats(ENGINE_HANDLE* handle, const void *cookie) {

}

static ENGINE_ERROR_CODE dummy_ng_unknown_command(ENGINE_HANDLE* handle, const void* cookie, protocol_binary_request_header *request, ADD_RESPONSE response) {
	return ENGINE_ENOTSUP;
}

static void dummy_ng_item_set_cas(ENGINE_HANDLE *handle, const void *cookie, item* item, uint64_t val) {

}

/*
 * Esto retorna algo de información la cual se muestra en la consola
 */
static const engine_info* dummy_ng_get_info(ENGINE_HANDLE* handle) {
	static engine_info info = {
	          .description = "motor TheLastTime",
	          .num_features = 0,
	          .features = {
	               [0].feature = ENGINE_FEATURE_LRU,
	               [0].description = " "
	           }
	};

	return &info;
}


