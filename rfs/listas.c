#include "libRFS.h"

/* ______________________ Lista de Bloques ___________________ */

t_listaBloques *crearListaBloques() {
	t_listaBloques *lista = (t_listaBloques *) malloc(sizeof(t_listaBloques));
	lista->head = NULL;
	return lista;
}

void sacarDeLaListaBloques(t_nodoBloques **nodoAux,
		t_listaBloques *listaBloques) {

	if (listaBloques->head == NULL) {
		printf("No hay elementos en la lista\n");
		*nodoAux=NULL;
	} else {
		*nodoAux = listaBloques->head;
		listaBloques->head = listaBloques->head->next;
	}

}

void agregarEnLaListaBloques(int32_t bloque, t_listaBloques *listaBloques) {
	t_nodoBloques* nodo = NULL;
	t_nodoBloques *nuevoNodo;

	nuevoNodo = (t_nodoBloques *) calloc(1, sizeof(t_nodoBloques));
	nuevoNodo->bloque = bloque;

	if (listaBloques->head == NULL) { // Es el primero que voy a insertar.
		nuevoNodo->next = listaBloques->head;
		listaBloques->head = nuevoNodo;
	} else {
		nodo = listaBloques->head; // agrego al ultimo.
		while (nodo->next != NULL) {
			nodo = nodo->next;
		}
		nuevoNodo->next = NULL;
		nodo->next = nuevoNodo;
	}

}

/*_____________________ Lista de intocables ____________________*/

t_listaIntocables *crearListaIntocables() {
	t_listaIntocables *lista = (t_listaIntocables *) malloc(
			sizeof(t_listaIntocables));
	lista->head = NULL;
	return lista;
}

void liberarMutexOperacion(int32_t inodo) {

	extern t_listaIntocables *listaIntocables;
	extern pthread_rwlock_t semIntocables;
	pthread_rwlock_wrlock(&semIntocables); // pido candado ESCRITURA

	t_nodoIntocable *nodo = NULL;
	nodo = listaIntocables->head;
	while (nodo != NULL && nodo->inodo != inodo)
		nodo = nodo->next;

	pthread_rwlock_unlock(&nodo->estado);

	pthread_rwlock_unlock(&semIntocables);
//	printf("-      # Libere de la lista de intocables el inodo %d \n", inodo);

}

int32_t verSiEsIntocable(int32_t inodo, int32_t tipoOperacion) {

	extern t_listaIntocables *listaIntocables;
	extern pthread_rwlock_t semIntocables;
	t_nodoIntocable* nodo = NULL;
	int32_t resultado;

	pthread_rwlock_rdlock(&semIntocables);

	nodo = listaIntocables->head;
	while (nodo != NULL && nodo->inodo != inodo) //busco el inodo en la lista
		nodo = nodo->next;

	if (nodo == NULL) {
		pthread_rwlock_unlock(&semIntocables);
		resultado = 1; // no esta en la lista
	} else {
		//	nodo->solicitudes++;

		if (tipoOperacion == 1) { //  si es una grosa
			pthread_rwlock_unlock(&semIntocables);
			pthread_rwlock_wrlock(&nodo->estado); // me quedo esperando a que desbloqueen el mutex
			/*printf(
					"-- ES UNA GROSA : Cambie el mutex del inodo %d a Escritura \n",
					nodo->inodo);*/
			resultado = 0;
		} else { // es una op no grosa

			pthread_rwlock_unlock(&semIntocables);
			pthread_rwlock_rdlock(&nodo->estado); // si no pude me bloque a esperarlo.
			/*printf(
					"--NO ES UNA GROSA : Cambie el mutex del inodo %d a Lectura \n",
					nodo->inodo);*/
			resultado = 0;
		}
	}
	return resultado;
}

int32_t subirOperacion(int32_t inodo, int32_t estado) {

	int32_t esta = verSiEsIntocable(inodo, estado);
	if (esta) {
		agregarEnLaListaIntocables(inodo, estado);
	}
	return 1;
}

void agregarEnLaListaIntocables(int32_t inodo, int32_t estado) // estado: 0 OpNoGrosa, 1 OpGrosa
{
	extern t_listaIntocables *listaIntocables;
	extern pthread_rwlock_t semIntocables;
	t_nodoIntocable* nodo = NULL;
	t_nodoIntocable *nuevoNodo;

	nuevoNodo = (t_nodoIntocable *) calloc(1, sizeof(t_nodoIntocable));

//	pthread_rwlock_t semOperacion;
	nuevoNodo->inodo = inodo;
//	nuevoNodo->estado = semOperacion;

	pthread_rwlock_init(&nuevoNodo->estado, NULL);

	if (estado == 1)
		pthread_rwlock_wrlock(&nuevoNodo->estado);
	else
		pthread_rwlock_rdlock(&nuevoNodo->estado);

	pthread_rwlock_wrlock(&semIntocables);

	if (listaIntocables->head == NULL) { // Es el primero que voy a insertar.
		nuevoNodo->next = listaIntocables->head;
		listaIntocables->head = nuevoNodo;
	} else {
		nodo = listaIntocables->head; // agrego al ultimo.
		while (nodo->next != NULL)
			nodo = nodo->next;

		nuevoNodo->next = NULL;
		nodo->next = nuevoNodo;
	}
	pthread_rwlock_unlock(&semIntocables);
	/*printf(
			" \n # ¬¬ Agrege en la lista de intocables al inodo %d como operacion :%d \n",
			nuevoNodo->inodo, estado);*/

}

/*______________________ Lista de Pedidos _______________________*/

t_listaPedidos *crearListaPedidos() {

	t_listaPedidos *lista = (t_listaPedidos *) malloc(sizeof(t_listaPedidos));
	lista->head = NULL;
	return lista;
}

int32_t sacarDeLaLista(t_nodoPedido **nodoAux) {

	extern t_listaPedidos *listaPedidosRFS;
	extern pthread_mutex_t bloqueaLista;

	pthread_mutex_lock(&bloqueaLista);

	if (listaPedidosRFS->head == NULL) {
		pthread_mutex_unlock(&bloqueaLista);
		printf("la lista de pedidos esta vacia \n");
		return 0;
	}
	*nodoAux = listaPedidosRFS->head;
	listaPedidosRFS->head = listaPedidosRFS->head->next;

	pthread_mutex_unlock(&bloqueaLista);
	return 1;

}

void agregarEnLaLista(NIPC paquete, int32_t socketFD) {
	extern t_listaPedidos *listaPedidosRFS;
	extern pthread_mutex_t bloqueaLista;

	t_nodoPedido* nodo = NULL;

	t_nodoPedido *nuevoNodo;
	nuevoNodo = (t_nodoPedido *) calloc(1, sizeof(t_nodoPedido));
	crearNIPC(paquete.PayloadDescriptor, paquete.offset, paquete.size,
			paquete.PayloadLength, paquete.payload, &nuevoNodo->paquete,
			paquete.modo);
	nuevoNodo->socket = socketFD;
	nuevoNodo->pos=0;

	chauNIPC(&paquete); // lo puse nuevo.

	pthread_mutex_lock(&bloqueaLista);

	if (listaPedidosRFS->head == NULL) { // Es el primero que voy a insertar.
		nuevoNodo->next = listaPedidosRFS->head;
		listaPedidosRFS->head = nuevoNodo;
		pthread_mutex_unlock(&bloqueaLista);

	} else {
		nodo = listaPedidosRFS->head; // agrego al ultimo.

		while (nodo->next != NULL) {
			nodo = nodo->next;
		}
		nuevoNodo->next = NULL;
		nodo->next = nuevoNodo;
		pthread_mutex_unlock(&bloqueaLista);
	}

}

/* ________________ Lista de Pedidos En Espera ___________________*/

t_listaPedidos *crearListaEnEspera() {
	t_listaPedidos *lista = (t_listaPedidos *) malloc(sizeof(t_listaPedidos));
	lista->head = NULL;
	return lista;
}

int32_t sacarDeLaListaEnEspera(t_nodoPedido **nodoAux) {

	extern t_listaPedidos *listaEspera;
	extern pthread_mutex_t bloqueaListaEspera;

	pthread_mutex_lock(&bloqueaListaEspera);

	if (listaEspera->head == NULL)
		return 0;

	*nodoAux = listaEspera->head;
	listaEspera->head = listaEspera->head->next;

	pthread_mutex_unlock(&bloqueaListaEspera);
	return 1;

}

void agregarEnLaListaEspera(NIPC paquete, int32_t socketFD) {
	extern t_listaPedidos *listaEspera;
	extern pthread_mutex_t bloqueaListaEspera;

	t_nodoPedido* nodo = NULL;

	t_nodoPedido *nuevoNodo;
	nuevoNodo = (t_nodoPedido *) calloc(1, sizeof(t_nodoPedido));
	crearNIPC(paquete.PayloadDescriptor, paquete.offset, paquete.size,
			paquete.PayloadLength, paquete.payload, &nuevoNodo->paquete,
			paquete.modo);
	nuevoNodo->socket = socketFD;

	pthread_mutex_lock(&bloqueaListaEspera);

	if (listaEspera->head == NULL) { // Es el primero que voy a insertar.
		nuevoNodo->next = listaEspera->head;
		listaEspera->head = nuevoNodo;
		pthread_mutex_unlock(&bloqueaListaEspera);
	} else {
		nodo = listaEspera->head; // agrego al ultimo.

		while (nodo != NULL) {
			nodo = nodo->next;
		}
		nuevoNodo->next = NULL;
		nodo->next = nuevoNodo;

		pthread_mutex_unlock(&bloqueaListaEspera);
	}
}

