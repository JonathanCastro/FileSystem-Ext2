#include "libRFS.h"

/**__________________ Lista  ___________________**/

struct nodoPedido{
	NIPC paquete;
	int32_t socket;
	int32_t pos;
	struct nodoPedido *next;
};

typedef struct nodoPedido t_nodoPedido;

typedef struct{
	t_nodoPedido *head;
}t_listaPedidos;

 // en mainnnnnnnnnnnnnnnnnn

void agregarEnLaLista( NIPC paquete, int32_t socketFD);
int32_t sacarDeLaLista(t_nodoPedido **nodoAux);
t_listaPedidos *crearListaPedidos();


struct nodoIntocable{
	int32_t inodo;
	pthread_rwlock_t estado;
	int32_t solicitudes;
	struct nodoIntocable *next;
};

typedef struct nodoIntocable t_nodoIntocable;

typedef struct{
	t_nodoIntocable *head;
}t_listaIntocables;