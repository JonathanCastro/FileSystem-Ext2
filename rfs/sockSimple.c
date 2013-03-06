#include "libRFS.h"

int32_t abrir_conexion (unsigned short puerto)
{
  int32_t sockfd;
  struct sockaddr_in dirlocal;
  int32_t resp = 1;
  dirlocal.sin_family = AF_INET;
  dirlocal.sin_port = htons (puerto);
  dirlocal.sin_addr.s_addr = htonl (INADDR_ANY);
  if ( (sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
      perror ("socket");

      return sockfd;
    }
  if (setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &resp, sizeof(int32_t)) < 0)
    {
      perror ("setsockopt");
      return -1;
    }
  if (bind (sockfd, ( struct sockaddr*) &dirlocal, sizeof (dirlocal)) < 0)
    {
      perror ("bind");
      return -1;
    }
  if (listen (sockfd, 50) < 0) //Cantidad Maxima de Conexiones, Se puede setear por cfg
    {
      perror ("listen");
      return -1;
    }
  return sockfd;
}


int32_t aceptar_conexion (int32_t sockfd)
{
  int32_t fd;
  int32_t tam;
  struct sockaddr cliente;
  tam = sizeof(struct sockaddr);

  if ( (fd = accept (sockfd, &cliente,(socklen_t *)&tam)) < 0)
    {
      perror ("accept");
      return fd;
    }
	printf("Acepte la conexion ... HAPPY ! \n");
  return fd;
}


int32_t conectar (char *dirIP, unsigned short puerto)
{
  int32_t sockfd;
  struct sockaddr_in dirserver;

  dirserver.sin_family = AF_INET;
  dirserver.sin_port = htons (puerto);
  dirserver.sin_addr.s_addr = inet_addr (dirIP);

  sockfd = socket (AF_INET, SOCK_STREAM, 0);

  if (connect (sockfd, ( struct sockaddr*) &dirserver, sizeof (dirserver)) < 0)
    {
      perror ("connect");
      return -1;
    }

  return sockfd;
}

int32_t enviarNIPC (int32_t sock, NIPC paquete){
  int32_t ret = 0; /* Para lo que retornan las funciones de sockets. */
  int32_t tam = 0;
  char *buffer = NULL;

  tam = sizeof(paquete.PayloadDescriptor) + sizeof(paquete.offset)
	  + sizeof (paquete.size) + sizeof (paquete.modo)+ sizeof (paquete.PayloadLength)
      + paquete.PayloadLength;
  buffer = serializar(paquete);
  ret = sendall (sock, buffer, &tam);
  free(buffer);

  if (ret < 0)
    return -1;
  else
    return tam;
}


int32_t recibirNIPC (int32_t sock, NIPC *paquete)
{
  int32_t tam_paquete = 0;
  int32_t tamHeader;
  int32_t payloadLength = 0;


  tamHeader = sizeof(paquete->PayloadDescriptor) + sizeof(paquete->offset) + sizeof (paquete->size)+ sizeof (paquete->modo)+ sizeof (paquete->PayloadLength);
  char *header = calloc(1,tamHeader);
  tam_paquete = recv(sock, header, tamHeader, MSG_WAITALL); // recibo solo los primeros para obterner el Length
  if(tam_paquete <= 0)
	  return 0;
  else{
	memcpy(&payloadLength, header + sizeof(paquete->PayloadDescriptor) + sizeof(paquete->offset)+ sizeof (paquete->size)+ sizeof (paquete->modo) , sizeof(int32_t));

	char* buffer = malloc(tamHeader + payloadLength);
	memcpy(buffer, header, tamHeader);

	if (payloadLength > 0 )
		recv(sock, buffer + tamHeader, payloadLength, MSG_WAITALL); // recibo todo el paquete

	free(header);

	tam_paquete = tamHeader + payloadLength;
	if (tam_paquete > 0)
		deserializar (buffer, paquete);

	free (buffer);
	buffer = NULL;
	return tam_paquete;
  }
}

int32_t sendall (int32_t socket, char *buf, int32_t *len){
	int32_t total = 0; // cu�ntos bytes hemos enviado
	int32_t bytesleft = *len; // cu�ntos se han quedado pendientes
	int32_t n=0;

	while (total < *len){
		n = send (socket, buf + total, bytesleft, MSG_NOSIGNAL);
		if (n == -1){
			return -1;
		}
		total += n;
		bytesleft -= n;
	}

	*len = total;

	return  0;
}


void handshakeFSC (int32_t socket){
	NIPC paqueteRecibido, respuesta;

	recibirNIPC (socket, &paqueteRecibido);
	if (paqueteRecibido.PayloadDescriptor == 0){
		if(paqueteRecibido.PayloadLength != 0){
		  printf("Error:%s",paqueteRecibido.payload);
		  exit(0);
		}else{
			printf(" Handshake realizado #- \n");
			crearNIPC(0,0,0,0,NULL,&respuesta,0);
			enviarNIPC(socket,respuesta);
		}
	}else{
	  printf("No Recibi el Handshake al inicio, Finalizo Proceso\n");
	  exit(0);
	}

}

int32_t handshakeRFS (int32_t socketRFS, NIPC paqueteRecibido)
{
	NIPC respuesta;
	int32_t valorRetorno;

//	valorRetorno = recibirNIPC (socketRFS, &paqueteRecibido);
	if (paqueteRecibido.PayloadDescriptor == 0)
	{
		crearNIPC(0,0,0,0,NULL,&respuesta,0);
		valorRetorno = enviarNIPC(socketRFS,respuesta);
		chauNIPC(&respuesta);
		printf(" .- Handshake con Exito #\n ");
	}

//	chauNIPC(&paqueteRecibido);

	if (valorRetorno < 0)
		return -1;
	else
		return 1;
}



