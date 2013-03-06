#include "libRFS.h"

#include <pthread.h>

void seteoFD(struct pollfd *descriptores,int32_t nroSocket){
	int32_t i=0;
	while(descriptores[i].fd != 0 )
		i++;
		
	descriptores[i].fd = nroSocket;
	descriptores[i].events = POLLIN;
	descriptores[i].revents = 0;
}



void pollConexiones(){
	int32_t socketConexion;
	int32_t socketAccept;
	int32_t cantidadSockets=1;
	int32_t i,j;
	int32_t cantAceptados=0;
	int32_t cantRespuestas=0;
	int32_t timeout_msecs = -1;
	int32_t cantRecibido=0;
	NIPC nipcRecibido;
	
	socketConexion = abrir_conexion(puerto);
	struct pollfd *descriptoresLectura;
	descriptoresLectura = (struct pollfd *)calloc(cantidadSockets,sizeof(struct pollfd));
	printf("\n Esperando Pedidos #~ \n");

	seteoFD(descriptoresLectura,socketConexion);
	while(1){
		i=0;
		j=0;
		cantRespuestas = poll(descriptoresLectura,cantidadSockets + cantAceptados,timeout_msecs);
		
		while( i < cantidadSockets + cantAceptados && j < cantRespuestas){
			if (descriptoresLectura[i].revents == 17){
				printf("Error en el poll \n");
				exit(0);
			}else if ((descriptoresLectura[i].revents & POLLIN )&& descriptoresLectura[i].fd == socketConexion){
				 //Nuevo socket del PFS
				socketAccept = aceptar_conexion(socketConexion);
				if(socketAccept==-1){
					printf("Error en el conect del sockt\n");
					break;
				}
				cantAceptados++;
				descriptoresLectura = realloc(descriptoresLectura,(cantAceptados + cantidadSockets) * sizeof(struct pollfd));
				memset(&descriptoresLectura[cantAceptados + cantidadSockets - 1],'\0',sizeof(struct pollfd));
				seteoFD(descriptoresLectura,socketAccept);
				j++;
			}else if (descriptoresLectura[i].revents & POLLIN){
			
				//Recibo el pedido del PFS, lo enlisto y aumento el semaforo que activa el thread ResponderPedidos.
				cantRecibido =  recibirNIPC(descriptoresLectura[i].fd,&nipcRecibido);
				if(cantRecibido==0){
					printf("El FSC se ha desconectado...\n");
					free(descriptoresLectura);
					descriptoresLectura = NULL;
					descriptoresLectura = (struct pollfd *)calloc(cantidadSockets,sizeof(struct pollfd));
					seteoFD(descriptoresLectura,socketConexion);
					cantAceptados = 0;
				}else if (cantRecibido != 0){
				
					parametrosHilo *paramH=malloc(sizeof(parametrosHilo));
					paramH->socket=descriptoresLectura[i].fd;
					paramH->nipcRecibido= nipcRecibido;
					
					pthread_t idOperacion;
					pthread_create(&idOperacion , NULL , ( void*)recibirOperacion , (void*)paramH);

					j++;
				}
			}
			i++;
		}
	}
}



void realizarOperacion(void* arg){
	int32_t socket=(int32_t)arg;
	NIPC nipcRecibido;
	NIPC nipcAEnviar;
	recibirNIPC(socket,&nipcRecibido);
	switchFunciones(nipcRecibido,socket,nipcAEnviar);
}





/**________________________ POLLLL POSTAAA_________________________________**/

//
//void crearPoolConexionesRFS()
//{
//	printf("\n ______Poll de conexiones_______________\n");
//	extern int32_t *vectorSockets;
//	int32_t pool = 0;

//
//	int32_t socketConexion, socketAccept;
//	// para indicar que viene de otro .c ponemos --> extern tipoDato variable;
//	vectorSockets = (int32_t *)calloc(cantConexiones,sizeof(int32_t));
//	socketConexion = abrir_conexion(puerto);
//	while (pool < cantConexiones){
//		socketAccept = aceptar_conexion(socketConexion);
//		vectorSockets[pool] = socketAccept;
//			pool++;
//	}
//
//
//	struct pollfd descriptoresLectura[cantConexiones];
//	setearFD(descriptoresLectura);
//	recibirPedidos(descriptoresLectura);
//
//
//}
//
//void setearFD(struct pollfd *descriptores)
//{
//	extern int32_t *vectorSockets;
//	int32_t setea = 0;
//	while (setea < cantConexiones){
//		descriptores[setea].fd = vectorSockets[setea];
//		descriptores[setea].events = POLLIN;
//		descriptores[setea].revents = 0;
//		setea++;
//	}
//
//}
///* es del RFS  */
//void recibirPedidos(struct pollfd *descriptoresLectura)
//{
//	int32_t cantRespuestas=0;
//	int32_t timeout_msecs = -1;
//	int32_t valorDeRetorno=0;
//	NIPC nipcRecibido;
//
//	int32_t i=0;
//	int32_t j=0;
//
//	while (1){
//		i=0;
//		j=0;
//		cantRespuestas = poll(descriptoresLectura,cantConexiones,timeout_msecs);
//		if (cantRespuestas == -1)
//				exit(0);
//		else{
//			while( i < cantConexiones && j< cantRespuestas){
//
//				if (descriptoresLectura[i].revents == 17){// Ocurrio un Error, log y finalizo proceso
//					printf("Ocurrio un error en el revents - %d ",descriptoresLectura[i].revents);
//					exit(0);
//				}else if (descriptoresLectura[i].revents & POLLIN){
//
//					valorDeRetorno =  recibirNIPC (descriptoresLectura[i].fd, &nipcRecibido);
//					if(valorDeRetorno==0){
//						printf("El FSC se ha desconectado...\n");
//
//					}else if(valorDeRetorno!=0){
//
//						parametrosHilo *paramH=malloc(sizeof(parametrosHilo));
//						paramH->socket=descriptoresLectura[i].fd;
//						paramH->nipcRecibido=nipcRecibido;
//
//						pthread_t idOp;
//						pthread_create(&idOp, NULL, (void*)recibirOperacion, (void*)paramH);
//
//						}
//					j++;
//				}
//			i++;
//			}
//		}
//	}
//}
//
//
//
//
//
//void buscarConexionLibre (int32_t *buscoConexion){
//
//  int32_t  recorroPool = 0, bloqueaSocket = 0,
//      conexionEncontrada = 0, loEncontro=0;
//  extern poolMutex *poolConexiones;
//
//	while ( !loEncontro && recorroPool < cantConexiones)
//	{
//	  bloqueaSocket =0 ;// pthread_mutex_trylock(&poolConexiones[recorroPool].mutex);
//	  if (bloqueaSocket !=0 )//EBUSY
//		{
//		  (*buscoConexion)=poolConexiones[recorroPool].nroConexion;
//		  conexionEncontrada++;
//		  loEncontro=1;
//		  bloqueaSocket=pthread_mutex_unlock(&poolConexiones[recorroPool].mutex);
//		}
//	  recorroPool++;
//	}
//}
//
//NIPC enviarOperacion (int32_t id,const char *path, int32_t offset, off_t size,const char *buf){
//
//  int32_t nroSocket;
//  NIPC nipcAEnviar, nipcRecibido;
//  memset (&nipcRecibido, '\0', sizeof (nipcRecibido));
//
//
//  buscarConexionLibre (&nroSocket);
//  int32_t tamPath= strlen(path)+1;
//  crearNIPC (id, offset, size, tamPath, (char*)path,&nipcAEnviar);
//  enviarNIPC (nroSocket,nipcAEnviar);
//  chauNIPC(&nipcAEnviar);
//  recibirNIPC (nroSocket, &nipcRecibido);
//  liberarConexion(nroSocket);
//
//  return nipcRecibido;
//}
//
//
//void liberarConexion (int32_t nroSocket)
//{
//	extern poolMutex *poolConexiones;
//  int32_t buscoConexion = 0, loEncontre=0;
//  while (buscoConexion < cantConexiones  && !loEncontre )
//    {
//       if (poolConexiones [buscoConexion].nroConexion == nroSocket)
//           pthread_mutex_unlock (&(poolConexiones[buscoConexion].mutex));
//
//      buscoConexion++;
//    }
//}
