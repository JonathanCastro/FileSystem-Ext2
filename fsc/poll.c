#include "libFSC.h"

void crearPoolConexionesFSC (char *pDirIp, int16_t puerto){
  int32_t pool = 0;
  int32_t socketConexion;
  printf("\n# Creando Pool de conexiones :- \n");
  extern poolMutex *poolConexiones;
  poolConexiones= (poolMutex*) calloc(cantConexiones,sizeof(poolMutex));
 
  while (pool < cantConexiones){
      socketConexion = conectar (pDirIp, puerto);
      if (socketConexion == -1) // Loguear Error
        exit(0);
      if (pool == 0)
        handshakeFSC(socketConexion);
		
     (poolConexiones[pool]).nroConexion = socketConexion;
      pthread_mutex_init(&poolConexiones[pool].mutex, NULL);
      poolConexiones[pool].estado=0;
      pool++;
    }


}

void buscarConexionLibre (int32_t *buscoConexion){
  
	extern sem_t semaforo;

	sem_wait(&semaforo);

	extern pthread_mutex_t mutexConexiones;
	pthread_mutex_lock(&mutexConexiones);

	extern poolMutex *poolConexiones;

	int32_t recorroPool;

	for(recorroPool=0; poolConexiones[recorroPool].estado!=0 &&  recorroPool<cantConexiones; recorroPool++);

	poolConexiones[recorroPool].estado=1;
	(*buscoConexion)=poolConexiones[recorroPool].nroConexion;
	pthread_mutex_unlock(&mutexConexiones);


}

NIPC enviarOperacion (uint32_t id,const char *path, uint32_t offset, off_t size,const char *buf,int32_t mode){
  
  int32_t nroSocket;
  NIPC nipcAEnviar, nipcRecibido;
  memset (&nipcRecibido, '\0', sizeof (nipcRecibido));

  
  buscarConexionLibre (&nroSocket);

  uint32_t tamPath= strlen(path)+1;
  crearNIPC (id, offset, size, tamPath, (char*)path,&nipcAEnviar,mode);
  enviarNIPC (nroSocket,nipcAEnviar);
  chauNIPC(&nipcAEnviar);
  recibirNIPC (nroSocket, &nipcRecibido);

  liberarConexion(nroSocket);
  
  return nipcRecibido;
}


void liberarConexion (int32_t nroSocket){
	extern sem_t semaforo;

	extern pthread_mutex_t mutexConexiones;
	pthread_mutex_lock(&mutexConexiones);

	extern poolMutex *poolConexiones;

	int32_t i;
	for(i=0;i<cantConexiones && (poolConexiones[i].nroConexion!=nroSocket);i++);

	poolConexiones[i].estado=0;

	pthread_mutex_unlock(&mutexConexiones);
	sem_post(&semaforo);
}
