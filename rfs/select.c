//
//#include <sys/time.h>
//#include <sys/types.h>
//#include <unistd.h>
//
//#include "libRFS.h"
//
//
//
//
//void conexionSelect()
//{
//	int socketServidor;				/* Descriptor del socket servidor */
//	int socketCliente[cantConexiones];/* Descriptores de sockets con clientes */
//	int numeroClientes = 0;			/* N�mero clientes conectados */
//	fd_set descriptoresLectura;	/* Descriptores de interes para select() */
//	int maximo;							/* N�mero de descriptor m�s grande */
//	int i;								/* Para bubles */
//	int32_t cantRecibido=0;
//	NIPC nipcRecibido;
//
//	socketServidor = abrir_conexion(puerto);
//	if (socketServidor == -1){
//		perror ("Error al abrir servidor");
//		exit (-1);
//	}
//
//	while (1){
//		compactaClaves (socketCliente, &numeroClientes);
//		FD_ZERO (&descriptoresLectura);
//		FD_SET (socketServidor, &descriptoresLectura);
//		for (i=0; i<numeroClientes; i++)
//			FD_SET (socketCliente[i], &descriptoresLectura);
//
//		maximo = dameMaximo (socketCliente, numeroClientes);
//
//		if (maximo < socketServidor)
//			maximo = socketServidor;
//
//		select (maximo + 1, &descriptoresLectura, NULL, NULL, NULL);
//
//		/* Se comprueba si alg�n cliente ya conectado ha enviado algo */
//		for (i=0; i<numeroClientes; i++){
//			if (FD_ISSET (socketCliente[i], &descriptoresLectura)){
//				cantRecibido =  recibirNIPC(socketCliente[i],&nipcRecibido);
//				if (cantRecibido==0){
//					printf("El FSC se ha desconectado , %d cerro conexion ...\n", i+1);
//					socketCliente[i] = -1;
//				}else if(cantRecibido!=0){
//
//					parametrosHilo *paramH=malloc(sizeof(parametrosHilo));
//					paramH->socket=socketCliente[i];
//					paramH->nipcRecibido=nipcRecibido;
//
//					pthread_t idOp;
//					pthread_create(&idOp, NULL, (void*)recibirOperacion, (void*)paramH);
//				}
//			}
//		}
//
//		if (FD_ISSET (socketServidor, &descriptoresLectura))
//			nuevoCliente (socketServidor, socketCliente, &numeroClientes);
//	}
//}
//
//void nuevoCliente (int servidor, int *clientes, int *nClientes)
//{
//	clientes[*nClientes] = aceptar_conexion(servidor);
//	(*nClientes)++;
//	if ((*nClientes) >= cantConexiones)
//	{
//		close (clientes[(*nClientes) -1]);
//		(*nClientes)--;
//		return;
//	}
//
//	/* Escribe en pantalla que ha aceptado al cliente y vuelve */
//	printf ("Aceptado cliente nro:- %d\n", *nClientes);
//	return;
//}
//
//int dameMaximo (int *tabla, int n)
//{
//	int i;
//	int max;
//
//	if ((tabla == NULL) || (n<1))
//		return 0;
//
//	max = tabla[0];
//	for (i=0; i<n; i++)
//		if (tabla[i] > max)
//			max = tabla[i];
//
//	return max;
//}
//
//void compactaClaves (int *tabla, int *n)
//{
//	int i,j;
//
//	if ((tabla == NULL) || ((*n) == 0))
//		return;
//
//	j=0;
//	for (i=0; i<(*n); i++)
//	{
//		if (tabla[i] != -1)
//		{
//			tabla[j] = tabla[i];
//			j++;
//		}
//	}
//
//	*n = j;
//}
