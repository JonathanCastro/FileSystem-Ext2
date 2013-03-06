#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fuse.h>
#include <fcntl.h>
#include <libmemcached/memcached.h>
#include <memcached/protocol_binary.h>
#include <poll.h>
#include <pthread.h>
#include <errno.h>
#include <error.h>
#include <semaphore.h>
#include "archivo_Configuracion.h"
#include "log.h"

typedef struct{
	int32_t nroConexion;
	int32_t estado;
	pthread_mutex_t mutex;
}__attribute__ ((__packed__)) poolMutex;



/**_________ VARIABLES GLOBALES __________**/

int32_t *vectorSockets;
poolMutex *poolConexiones;
memcached_st* memcached;
pthread_mutex_t mutexConexiones;


/**______________MEMCACHED_______________**/

void inicializarMemcached();
char * armarKey(char operacion, const char *path);
void setearValor(char operacion, const char *path, char* buffer);
char* getValor(char operacion, const char *path);
void liberarCache();

char* dameElNombre(char *path);
/**_______________FUSE_______________**/

int fsc_create(const char *, mode_t, struct fuse_file_info *);
int fsc_open(const char *, struct fuse_file_info *);
int fsc_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int fsc_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int fsc_release(const char *, struct fuse_file_info *);
int fsc_truncate(const char *, off_t);
int fsc_unlink(const char *);
int fsc_mkdir(const char*, mode_t);
int fsc_readdir(const char *, void *, fuse_fill_dir_t, off_t , struct fuse_file_info *);
int fsc_rmdir(const char *);
int fsc_getattr(const char *, struct stat *);


/**_____________ NIPC _______________**/

typedef enum {
	HANDSHAKE = 0, GETATTR = 1, READDIR = 2, OPEN = 3, RELEASE = 4, READ= 5,
	WRITE = 6, CREATE = 7, UNLINK = 8, TRUNCATE = 9, RMDIR = 10, MKDIR= 11,
} type_p;

typedef struct{
	int32_t PayloadDescriptor;
	int32_t offset;
	int32_t size;
	int32_t modo;
	int32_t PayloadLength;
	char *payload;  // cuando envio es el Path , cuando vuelve la respuesta.
}__attribute__ ((__packed__)) NIPC;

char* serializar(NIPC paquete);
void deserializar (char *buffer, NIPC* paquete);
void crearNIPC (int32_t PayloadDescriptor,int32_t offset,int32_t size, int32_t PayloadLength, char *Payload, NIPC *ret,int32_t modo);
void chauNIPC (NIPC *paquete);

typedef struct{
	int32_t size;
	int32_t links;
	int32_t tamanio;
	char *tipo;
}__attribute__ ((__packed__)) getAttr;

char *serializarGetAttr(getAttr paquete);
void deserializarGetAttr(char *buffer, getAttr* paquete) ;


/**____________SOCKETS_________________**/

int32_t abrir_conexion (unsigned short puerto);
int32_t aceptar_conexion (int32_t sockfd);
int32_t conectar (char *dirIP, unsigned short puerto);
int32_t enviarNIPC (int32_t sock, NIPC paquete);
int32_t recibirNIPC (int32_t sock, NIPC *paquete);
int32_t sendall (int32_t socket, char *buf, int32_t *len);
void handshakeFSC (int32_t socket);
int32_t handshakeRFS (int32_t socketRFS, NIPC paqueteRecibido);


/**_____________POLL___________________**/



void crearPoolConexionesFSC (char *pDirIp, int16_t puerto);
void crearPoolConexionesRFS();
void setearFD(struct pollfd *descriptores);
void recibirPedidos(struct pollfd *descriptoresLectura);
void realizarOperacion(void* arg);
void buscarConexionLibre (int32_t *buscoConexion);
NIPC enviarOperacion (uint32_t id,const char *path, uint32_t offset, off_t size,const char *buf,int32_t mode);
void liberarConexion (int32_t nroSocket);



/** _____________ FUNCIONES FUSE _________________ **/

void* imprimirListado(char * lista ,fuse_fill_dir_t filler);

char* obtenerpathQueLoContiene(char* path);


/*______________________________________*/
typedef struct{
	int32_t size;
	int32_t mode;
	int32_t links;
}__attribute__ ((__packed__)) getAttrCache;

char* serializarGetAttrCache(getAttrCache *paquete);
void deserializarGetAttrCache(char *buffer, getAttrCache *paquete );


