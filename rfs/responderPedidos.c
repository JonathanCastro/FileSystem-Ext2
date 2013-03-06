#include "libRFS.h"

void inicializarHilos() {

	extern t_pollThread *pollHilos;
	int32_t i;

	extern pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	for (i = 0; i < cantOperaciones; i++) {
		pollHilos[i].enUso = 0;
	}
}

int32_t buscarHiloDisponible() {
	int32_t i = 0;
	extern pthread_rwlock_t mutexHilos;
	extern t_pollThread *pollHilos;
	pthread_rwlock_rdlock(&mutexHilos);
	while (pollHilos[i].enUso != 0 && i < cantOperaciones)
		i++;
	if (i < cantOperaciones)
		pollHilos[i].enUso = 1;

	pthread_rwlock_unlock(&mutexHilos);

	if (i == cantOperaciones)
		return -1;

	return i;
}

void liberarHilo(int32_t pos) {
	extern pthread_rwlock_t mutexHilos;
	extern t_pollThread *pollHilos;

	pthread_rwlock_wrlock(&mutexHilos);
	pollHilos[pos].enUso = 0;
	pthread_rwlock_unlock(&mutexHilos);
}

void liberarRecursosHilos() {

	extern pthread_attr_t attr;

	pthread_attr_destroy(&attr);

}

void responderPedidos() {

	t_nodoPedido *nodo;

	extern sem_t semaforoLista;
	int32_t pos;
	extern t_pollThread *pollHilos;
	extern pthread_rwlock_t mutexHilos;
	extern pthread_attr_t attr;
	inicializarHilos(); // creo el poll de hilos :P

	while (1) {

		sem_wait(&semaforoLista);

		pos = buscarHiloDisponible();

		if (pos != -1) {
			sacarDeLaLista(&nodo); // saco una operacion de la lista.

//			parametrosHilo *paramH = malloc(sizeof(parametrosHilo));
//			paramH->socket = nodo->socket;
//			paramH->nipcRecibido = nodo->paquete;
//			paramH->pos = pos;

			nodo->pos=pos;

			pthread_rwlock_rdlock(&mutexHilos); // por las dudas, es reg critica
			pthread_create(&pollHilos[pos].id, &attr, (void*) recibirOperacion,
					(void*) nodo);
			pthread_rwlock_unlock(&mutexHilos);

		} else {
			sem_post(&semaforoLista);
		}

	}
}

void recibirOperacion(void* arg) {

	t_nodoPedido *pHilo = (t_nodoPedido *) arg;
	NIPC nipcAEnviar;
	switchFunciones(pHilo->paquete, pHilo->socket, nipcAEnviar);
	liberarHilo(pHilo->pos);
	chauNIPC(&pHilo->paquete);
	free(pHilo);

}
