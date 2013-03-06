#include "funciones.h"
#define LRU 0
#define FIFO 1
#define PARTICIONES_DINAMICAS 0
#define BUDDY 1
#define BEST 0
#define FIRST 1




void inicializo_vector(estructura_elemento_cache* vector_cache, void* cache, size_t tam_max_cache, int cant_max_elementos){

int j;

//inicializo primero el primer elemento
	vector_cache[0].data = cache;              //hago que el primer elemento del vector apunte al inicio de la memoria
	vector_cache[0].tam_data = 0; //y sea de todo el tamaño disponible
    vector_cache[0].key[0]= '\0';
	vector_cache[0].tam_key = 0;
	vector_cache[0].tam_particion= tam_max_cache;
	vector_cache[0].flags = 0;
	vector_cache[0].stored =  false;
	vector_cache[0].tiempo_exp= 0;
	vector_cache[0].libre= true;         //para saber si es agujero o no
	vector_cache[0].ultimo_tiempo_uso.tv_nsec=0;
	vector_cache[0].ultimo_tiempo_uso.tv_sec=0;//para el LRU o FIFO
	vector_cache[0].pos =0;             //para saber en que pos del vector esta el struct

//incializo el resto
	for (j=1; j <= cant_max_elementos; j++) {

        vector_cache[j].data = NULL;              //hasta q se den de "alta" para representar una particion seran null
		vector_cache[j].tam_data = 0;
		vector_cache[j].tam_particion= 0;
		vector_cache[j].key[0]= '\0'; //pongo todo el vector en 0
		vector_cache[j].tam_key = 0;
		vector_cache[j].flags = 0;
		vector_cache[j].stored =  false;
		vector_cache[j].tiempo_exp= 0;
		vector_cache[j].libre= true;
		vector_cache[j].ultimo_tiempo_uso.tv_nsec=0;
		vector_cache[j].ultimo_tiempo_uso.tv_sec=0;
		vector_cache[j].pos = j;
	}

} // fin de inicializo_vector

//reordeno por posicion de memoria a partir de la posicion en la q inserto, lo anterior ya se q esta ordenado
void ordeno_vector(estructura_elemento_cache* vector_cache, int pos, int nueva_pos){

	int i, inicio=pos+1, fin=nueva_pos;

	estructura_elemento_cache aux;

	aux= vector_cache[fin];
	/*aux->data= vector_cache[fin].data;
	aux->flags = vector_cache[fin].flags;
	memcpy (aux->key, vector_cache[fin].key,vector_cache[fin].tam_key);
	aux->libre= vector_cache[fin].libre;
	aux->pos= vector_cache[fin].pos;
	aux->stored= vector_cache[fin].stored;
	aux->tam_data= vector_cache[fin].tam_data;
	aux->tam_key= vector_cache[fin].tam_key;
	aux->tiempo_exp= vector_cache[fin].tiempo_exp;
	aux->ultimo_tiempo_uso= vector_cache[fin].ultimo_tiempo_uso; */


	int j= fin; //desplazo todas las posiciones en circulo hacia la derecha
	for ( i=1 ; (j-inicio)>=0 ; i++){
		vector_cache[fin-i+1]=vector_cache[fin-i];
	        j--;
	}

	vector_cache[inicio]= aux;
	/*
	vector_cache[inicio].data= aux->data;
	memcpy (vector_cache[inicio].key, aux->key,aux->tam_key);
	vector_cache[inicio].libre=aux->libre;
	vector_cache[inicio].pos=aux->pos;
	vector_cache[inicio].stored=aux->stored;
	vector_cache[inicio].tam_data=aux->tam_data;
	vector_cache[inicio].tam_key=aux->tam_key;
	vector_cache[inicio].tiempo_exp=aux->tiempo_exp;
	vector_cache[inicio].ultimo_tiempo_uso=aux->ultimo_tiempo_uso; */
}

//habilito una de las posiciones en null, lo hago apuntar a dnd corresponda y seteo su tamaño
void habilito_nueva_particion(estructura_elemento_cache* vector_cache, int pos,void* direccion, int cant_max_elementos, int tam_max_cache){

int j= pos;

//busco un elemento del vector que no sea una particion habilitada
while (vector_cache[j].data != NULL) {j++;}

//hago que la nueva particion apunte debajo del dato
vector_cache[j].data = direccion;

//reordeno
ordeno_vector (vector_cache, pos, j);

//seteo el tamaño de la nueva partcion
if ((pos+2)<= cant_max_elementos){
	if (vector_cache[pos+2].data != NULL){
		//el tamaño del nuevo agujero es la diferencia entre el comienzo donde apunta y el inicio de la particion siguente
		vector_cache[pos+1].tam_particion = vector_cache[pos+2].data - vector_cache[pos+1].data;
	    }
	else {
		vector_cache[pos+1].tam_particion = (vector_cache[0].data + tam_max_cache) - vector_cache[pos+1].data;
	}
}
else {
		vector_cache[pos+1].tam_particion = (vector_cache[0].data + tam_max_cache) - vector_cache[pos+1].data;
		}

} //fin habilito_nueva_particion



int algoritmoBestFit( estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos) {

	int j, tam_min, pos_tam_min=-1;


 //mientras sea una posicion activa
  for(j=0; (vector_cache[j].data != NULL) && (cant_max_elementos > j); j++){
	  //si es un agujero suficientemente grande lo comparo a ver si es el menos grande.
	  if ((vector_cache[j].libre == true) && (vector_cache[j].tam_particion >= tam_data)){

		  if(pos_tam_min == -1 || vector_cache[j].tam_particion <tam_min){

			  tam_min = vector_cache[j].tam_particion;
			  pos_tam_min = j;
		  }
	  }
  }

  return pos_tam_min;


}

int algoritmoFirstFit(estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos) {

	int i=0, pos=-1;

	//busco uno libre que pueda guardarlo
	 while ((pos ==-1)&& (vector_cache[i].data != NULL) && (cant_max_elementos > i)){
		 if( (vector_cache[i].libre == true) && (vector_cache[i].tam_particion >= tam_data))
		    	pos=i;
		  i++;
		    }

    return pos;
}



int buscarElementoVector(estructura_elemento_cache* vector_cache, char* key, int cant_max_elementos){

	int i;

    for (i=0; (cant_max_elementos > i) && (vector_cache[i].data != NULL) ; ++i) {
        if((vector_cache[i].libre==false)&& (vector_cache[i].stored==true)&& (memcmp(key, vector_cache[i].key,41)==0))
           return i;
    }
    return -1;
} //fin buscar

int buscarParticion(estructura_motor* motor, estructura_elemento_cache* vector_cache, size_t tam_data){

	int i;
	if(motor->alg_seleccion_libre==BEST) {
	        i= (algoritmoBestFit(vector_cache, tam_data, motor->cant_max_elementos));
	    }
	    else{
	        if(motor->alg_seleccion_libre==FIRST) {
	        i= (algoritmoFirstFit(vector_cache, tam_data, motor-> cant_max_elementos));
	        }
	           }
	return i;
} //fin buscar particion


int eliminarParticion(estructura_motor* motor, estructura_elemento_cache* vector_cache){

	    int i;
		int pos=-1;
		long  nano_min , nano;
		long seg_min,seg;
		struct timespec tiempo_actual;
		clock_gettime(CLOCK_MONOTONIC, &tiempo_actual);


		seg_min= tiempo_actual.tv_sec;
		nano_min= tiempo_actual.tv_nsec;

		for(i=0; (motor-> cant_max_elementos > i) && (vector_cache[i].data!=NULL); i++){
		    seg= vector_cache[i].ultimo_tiempo_uso.tv_sec;
			nano= vector_cache[i].ultimo_tiempo_uso.tv_nsec;

			if  ((seg_min > seg) && vector_cache[i].libre == false ){
			seg_min= seg;
		    nano_min= nano;
			pos = i;}
			else {
				if  ((seg_min == seg) && vector_cache[i].libre == false )
					if  ((nano_min > nano)){
						seg_min= seg;
					    nano_min= nano;
						pos = i;
					}
			}

		} //fin de for

		if (pos!= -1) {
			//elimino
			vector_cache[pos].libre= true;
			vector_cache[pos].stored= false;

		}

log_debug(flogger,"Victima: %s - Algoritmo: %i ", vector_cache[pos].key,motor->alg_seleccion_victima );
return pos;


} //fin eliminar particion

int esquemaParticionesDinamicas(estructura_motor* motor, estructura_elemento_cache* vector_cache, size_t tam_data){


	   int pos_eliminada=-2;
	   int pos= buscarParticion(motor,vector_cache, tam_data); //me da la pos en el vector q apunta a esa particion capaz de guardar el dato

	  while (pos==-1){ //si no encontro un lugar
		       int cant_intentos=motor->cant_busquedas_fallidas;

		       if (cant_intentos> 0)
		    	   	   cant_intentos--; //ya habia quemado un intento cuando busque la 1era vez.
		       //si es -1 elimino todas las particiones y recien ahi compacto
		       if (cant_intentos== -1){

		    	   while (pos_eliminada != -1){

		   		   pos_eliminada= eliminarParticion(motor,vector_cache);

				   if ((pos_eliminada != -1)&& (vector_cache[pos_eliminada].tam_particion >= tam_data)){
					   return pos_eliminada;
					   			}
				   }
		    	   compactacion(vector_cache, motor->cant_max_elementos, motor->tam_max_cache);
		    	   pos= buscarParticion(motor,vector_cache, tam_data);
		    	   }
		       else {
		    	   if(cant_intentos==0)
		    	   {
		    		   //si era cero compacto- veo- sino borro.
		    		   compactacion(vector_cache, motor->cant_max_elementos, motor->tam_max_cache);
		    		   pos= buscarParticion(motor,vector_cache, tam_data);

		    		   if (pos==-1) {
		    			   pos_eliminada= eliminarParticion(motor,vector_cache);

					   if (vector_cache[pos_eliminada].tam_particion >= tam_data){
						   return pos_eliminada;
						   			}
		    		   }
		    	   }
		    	   else {
		    		   //sino borro solo hasta agotar los intentos o hasta que no quede nada
		    		   while ((cant_intentos > 0)&& (pos_eliminada!=-1)){

		    			   	   pos_eliminada= eliminarParticion(motor,vector_cache);

		    			   	   if ((pos_eliminada != -1) &&(vector_cache[pos_eliminada].tam_particion >= tam_data)){
		    			   		   return pos_eliminada;
		    			   	   	  }
		    			   	   --cant_intentos;
		    		   }
		    		   compactacion(vector_cache, motor->cant_max_elementos, motor->tam_max_cache);
		            	pos= buscarParticion(motor,vector_cache, tam_data);

		        }//fin if

		       }
	  }//fin del while

return pos;
 }// fin particiones dinamicas


//muevo todos los datos de una particion a otra, en la cache y el vector
void muevo_datos_particion(estructura_elemento_cache* vector_cache, int pos_libre, int i){

	            //muevo los datos
        	   memcpy(vector_cache[pos_libre].data, vector_cache[i].data, vector_cache[i].tam_particion);

                //actualizo el vector
	        	vector_cache[pos_libre].tam_data = vector_cache[i].tam_data;
                vector_cache[pos_libre].tam_particion = vector_cache[i].tam_particion;
                memset(vector_cache[pos_libre].key, 0, 41);
                memcpy (vector_cache[pos_libre].key, vector_cache[i].key, vector_cache[i].tam_key);
           		vector_cache[pos_libre].tam_key = vector_cache[i].tam_key;
           		vector_cache[pos_libre].flags = vector_cache[i].flags ;
           		vector_cache[pos_libre].stored =  true;
           		vector_cache[pos_libre].tiempo_exp= vector_cache[i].tiempo_exp;
           		vector_cache[pos_libre].libre= false;
          		vector_cache[pos_libre].ultimo_tiempo_uso=vector_cache[i].ultimo_tiempo_uso;

           		//la pos siguiente ahora es mi nueva libre, mi nuevo agujero estara ahi.Lo instancio: lo hago apuntar dps del ultimo dato.
   	          vector_cache[pos_libre+1].data =  vector_cache[pos_libre].data + vector_cache[pos_libre].tam_particion;
   	          vector_cache[pos_libre+1].libre=true;
   	          vector_cache[pos_libre+1].stored=false;


} //fin funcion



void compactacion (estructura_elemento_cache* vector_cache, int cant_max_elementos, int tam_max_cache){

	int  pos_libre=0, i=0, j=0, cont_tamanio=0;

    //encuentro la primer posicion libre, a partir de esta posicion, siempre sumandole 1 tendre la nueva libre.
	while (cant_max_elementos > i && vector_cache[i].libre== false && vector_cache[i].data!= NULL ){
		i++;
	    pos_libre=i;
	}

	//si la posicion libre esta en la ultima posicion ya no hay nada que compactar
	if ((pos_libre+1)== cant_max_elementos ||  vector_cache[pos_libre+1].data== NULL){

		log_debug(flogger,"Compactando- Estaba todo ordenado, no hay nada que compactar");
		return;
	}

	while ((cant_max_elementos> i)&& (vector_cache[i].data!= NULL )){

		//ahora copio en el agujero el primer dato que encuentre, dps actualiza la pos siguiente para q sea el nuevo agujero libre.
        i++;
		//encuentro el proximo dato que haya
		while((cant_max_elementos> i) && (vector_cache[i].libre== true)&& (vector_cache[i].data!= NULL)){
			    	  				i++;
			    	  				}

			if((cant_max_elementos> i)&&(vector_cache[i].data!= NULL)){

						  log_debug(flogger,"Compactando- la particion: %p quedo en: %p - Su tamaño es: %i", vector_cache[i].data,vector_cache[pos_libre].data,vector_cache[i].tam_particion );

					      muevo_datos_particion(vector_cache, pos_libre, i);
		    	          //la pos siguiente ahora es mi nueva libre
		    	          pos_libre= pos_libre + 1;

		    	          }

	 }  //fin del while

	//hago un recuento para saber cuanta cache ocupada hay ahora
	for  (j=0; (pos_libre >= j) && (vector_cache[j].libre==false); j++){
			cont_tamanio= cont_tamanio + vector_cache[j].tam_particion;

	}

	//reajusto el tamaño de la unica posicion libre y sus datos
	if (j!=0)
	vector_cache[j].data = vector_cache[j-1].data+ vector_cache[j-1].tam_particion;
	vector_cache[j].tam_particion= tam_max_cache - cont_tamanio;
	vector_cache[j].tam_data= 0;
	vector_cache[j].key[0]= '\0';
	vector_cache[j].tam_key = 0;
	vector_cache[j].flags = 0;
	vector_cache[j].stored =  false;
	vector_cache[j].tiempo_exp= 0;
	vector_cache[j].libre= true;
	vector_cache[j].ultimo_tiempo_uso.tv_nsec=0;
	vector_cache[j].ultimo_tiempo_uso.tv_sec=0;
	vector_cache[j].pos = j;

	//el resto de la posiciones vuelven a inhabilitarse, o sea, a apuntar a null
	j= j + 1;
	while (cant_max_elementos > j){
			vector_cache[j].data= NULL;
			vector_cache[j].libre= true;
			vector_cache[j].stored= false;
			j++;
			}

} //fin compactacion


//tomo una particion, achico su tamaño a la mitad y pongo al lado otra de igual tamaño
void habilito_nueva_particion_buddy(estructura_elemento_cache* vector_cache, int pos)
{
	int j= pos;

	vector_cache[pos].tam_particion= (vector_cache[pos].tam_particion/2);

	//busco un elemento del vector que no sea una particion habilitada
	while (vector_cache[j].data != NULL) {j++;}

	//hago que la nueva particion apunte debajo del dato
	vector_cache[j].data = vector_cache[pos].data + vector_cache[pos].tam_particion;
	vector_cache[j].tam_particion= vector_cache[pos].tam_particion;

	//reordeno
	ordeno_vector (vector_cache, pos, j);
} //fin habilito_nueva_particion

void ordenovectorBuddy(estructura_elemento_cache* vector_cache, int pos, int cant_max_elementos)
 {
	if ((pos + 1 != cant_max_elementos)	&& (vector_cache[pos+1].data != NULL)) {
		//piso el quiero borrar con el que le sigue
		vector_cache[pos] = vector_cache[pos + 1];
		vector_cache[pos].pos = pos; //le arreglo la posicion sino quedaba corrida
		int i = pos + 1;

		//al que le seguia con el proximo y asi
		while ((vector_cache[i].data != NULL) && cant_max_elementos > i) {
			vector_cache[i] = vector_cache[i + 1];
			vector_cache[i].pos = i; //le arreglo la posicion sino quedaba corrida
			i++;
		}
		//el eliminado queda en la ultima posicion :)
		vector_cache[i].libre = true;
		vector_cache[i].stored = false;
		vector_cache[i].data = NULL;

	}
	else {
		//me elimino
		vector_cache[pos].libre = true;
		vector_cache[pos].stored = false;
		vector_cache[pos].data = NULL;
	}

}

int esquemaBuddySystem(estructura_motor* motor,estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos)
{

	   int pos_eliminada;
	   int pos= seleccionParticion(vector_cache, tam_data, motor->cant_max_elementos); //me da la pos en el vector q apunta a esa particion capaz de guardar el dato

	  while (pos==-1){ //si no encontro un lugar

		  pos_eliminada= eliminarParticion(motor,vector_cache);
		  if (pos_eliminada != -1)
			  unionParticiones(vector_cache, pos_eliminada, motor->cant_max_elementos);

	  pos =seleccionParticion(vector_cache, tam_data, motor->cant_max_elementos);


	  }//fin del while
return pos;
}

void unionParticiones(estructura_elemento_cache* vector_cache, int pos, int cant_max_elementos)
{
	//me fijo cuantos de mi tamaño entrarian antes de mi, si es 0 o par mi hno esta a la derecha, sino a la izq.
	int cantAntesDeMi=0;
	void *puntero_aux_buddy= vector_cache[0].data;

	while(puntero_aux_buddy < vector_cache[pos].data){

		puntero_aux_buddy= puntero_aux_buddy + vector_cache[pos].tam_particion;
		cantAntesDeMi++;
	}

	if(cantAntesDeMi % 2 ==0){
		//me uno con el de adelante
		if ((vector_cache[pos + 1].libre == true) && (vector_cache[pos + 1].data != NULL)&& (vector_cache[pos].tam_particion==vector_cache[pos+1].tam_particion )) {
			vector_cache[pos].tam_particion = vector_cache[pos].tam_particion + vector_cache[pos + 1].tam_particion;
			ordenovectorBuddy(vector_cache, pos + 1, cant_max_elementos);
			unionParticiones(vector_cache, pos, cant_max_elementos);
		}
	}
	else{
		//me uno con el de atras
		if ((vector_cache[pos - 1].libre == true) && (vector_cache[pos].tam_particion== vector_cache[pos-1].tam_particion )) {
			vector_cache[pos - 1].tam_particion = vector_cache[pos - 1].tam_particion + vector_cache[pos].tam_particion;
			ordenovectorBuddy(vector_cache, pos, cant_max_elementos);
			unionParticiones(vector_cache, pos - 1, cant_max_elementos);
		}
	}

} //fin funcion


int seleccionParticion(estructura_elemento_cache* vector_cache, size_t tam_data, int cant_max_elementos){
	int i=0, pos=-1;

		//busco uno libre que pueda guardarlo
	    while ((pos ==-1)&& (cant_max_elementos > i)&& (vector_cache[i].data != NULL)){
	    	if( (vector_cache[i].libre == true) && (vector_cache[i].tam_particion >= tam_data))
	    		pos=i;
	    	i++;
	    }
		//si encontre uno mayor tengo que partirlo hasta encontrar el tamaño que mejor se ajuste
		if ((pos != -1) && (vector_cache[pos].tam_particion > tam_data))
			{
				//si es mayor voy diviendo por dos hasta encontrar el tamaño justo
				while ((vector_cache[pos].tam_particion/2) >= tam_data)
					{
						 //tomo una particion, achico su tamaño a la mitad y pongo al lado otra de igual tamaño
						habilito_nueva_particion_buddy(vector_cache, pos);
					}
		     } //fin if.


	return pos;
	}

//para saber si en buddy los tamaños son potencias de dos
int potencia_dos (int numero1, int numero2){

	while (((numero1 % 2) == 0) && numero1 > 1) /* divido por dos hasta dos xD */
		     numero1 /= 2;

	while (((numero2 % 2) == 0) && numero2 > 1)
			     numero2 /= 2;

	if ((numero1 !=1 )||(numero2 !=1)) {

	     	  printf("El tamaño máximo de la cache y el tamaño minimo del bloque deben ser potencias de dos para buddy system");
	    	  return -1;
	      }

return 0;
}
