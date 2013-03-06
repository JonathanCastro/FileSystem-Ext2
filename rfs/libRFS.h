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
#include <sys/poll.h>
#include <semaphore.h>
#include "string.h"
#include "log.h"

#include "archivo_Configuracion.h"
#include "ext2.h"


/**_________ VARIABLES GLOBALES __________**/


//memcached_st* memcached;
//int32_t *vectorSockets;
//extern poolMutex *poolConexiones;


/**______________MEMCACHED_______________**/

void inicializarMemcached();
char * armarKey(char operacion, const char *path);
void setearValor(char operacion, const char *path, char* buffer);
char* getValor(char operacion, const char *path);
void liberarCache();
char* buscarPathSinNombre(char* path, int32_t cantToke);
char* obtenerpathQueLoContiene(char* path);
char * armarKeyParaRW(int32_t nroBolque, const char *path);
void setearValorRW (int32_t nroBolque, const char *path, char* buffer);
char* getValorRW(int32_t nroBolque, const char *path);

/**_____________ NIPC _______________**/

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

/**_______________FUSE_______________**/

int rfs_create(int32_t sock,NIPC paqueteEnviar,char *,int32_t);
int rfs_open(int32_t sock,NIPC paqueteEnviar,char *);
int rfs_read(int32_t sock,NIPC paqueteEnviar,char *, size_t, off_t);
int rfs_write(int32_t sock,NIPC paqueteEnviar,char *, size_t, off_t);
int rfs_release(int32_t sock,NIPC paqueteEnviar,char *);
int rfs_truncate(int32_t sock,NIPC paqueteEnviar,char *,size_t);
int rfs_unlink(int32_t sock,NIPC paqueteEnviar,char *);
int rfs_mkdir(int32_t sock,NIPC paqueteEnviar,char*,int32_t);
int rfs_readdir(int32_t sock,NIPC paqueteEnviar,char *);
int rfs_rmdir(int32_t sock,NIPC paqueteEnviar,char *);
int rfs_getattr(int32_t sock,NIPC paqueteEnviar,char *);

void switchFunciones (NIPC paquete,int32_t sock,NIPC paqueteEnviar);

void escribirBolques(div_t offsetArchivo,div_t sizeAEscribir,Ext2Inodo *inodoAEsc,char* buffEscritura);
int32_t obtenerCantBloquesNuevos(Ext2Inodo *inodoAEscribir, int32_t numeroBloqueArchivo, int32_t cantBloquesAEscribir);
int32_t escribirArchivo(int32_t nInodo, size_t size, off_t offset, char* buffEscritura, char* path);
void actualizarSize(int32_t numeroInodo,Ext2Inodo *inodo,size_t nuevoTamanio);

/**____________SOCKETS_________________**/

int32_t abrir_conexion (unsigned short puerto);
int32_t aceptar_conexion (int32_t sockfd);
int32_t conectar (char *dirIP, unsigned short puerto);
int32_t enviarNIPC (int32_t sock, NIPC paquete);
int32_t recibirNIPC (int32_t sock, NIPC *paquete);
int32_t sendall (int32_t socket, char *buf, int32_t *len);
void handshakeFSC (int32_t socket);
int32_t handshakeRFS (int32_t socketRFS,NIPC paqueteRecibido);


/**_____________POLL___________________**/

typedef struct{
	int32_t nroConexion;
	pthread_mutex_t mutex;
}__attribute__ ((__packed__)) poolMutex;

void crearPoolConexionesFSC (char *pDirIp, int16_t puerto);
void crearPoolConexionesRFS();
void setearFD(struct pollfd *descriptores);
void recibirPedidos(struct pollfd *descriptoresLectura);
void realizarOperacion(void* arg);
void buscarConexionLibre (int32_t *buscoConexion);
NIPC enviarOperacion (int32_t id,const char *path, int32_t offset, off_t size,const char *buf);
void liberarConexion (int32_t nroSocket);
void pollConexiones();
void recibirOperacion(void* arg);



/**________________UNLOCKED_____________**/

void abrirArchivoDisco(void);
int leerBloqueDisco(int bloque, char* buffLectura,int cantB);
int escribirBloqueDisco(int bloque,int offet,int size ,char* buffEscritura);
int escribirSuperBloque(char*buffEscritura);
int leerSuperBloque(int bloque, char* buffLectura,int cantB);
//int leerSuperBloque2(int bloque,Ext2SuperBloque* superBloque,int cantB);
int leerInodo(int offset,char *buffLectura);
int escribirInodo(int offset, char*buffEscritura);
int leerTdescriptora(int offset,char *buffLectura,int32_t cantGrupos);
int leerTdescriptora2(int offset,char *buffLectura);
int escribirTdescriptora(int offset, char*buffEscritura,int32_t cantGrupos);


typedef struct {
	int32_t socket;
	NIPC nipcRecibido;
	int32_t pos;
}__attribute__ ((__packed__)) parametrosHilo;

/**________________SELECT_____________**/

/* Prototipos de las funciones definidas en este fichero */
void nuevoCliente (int servidor, int *clientes, int *nClientes);
int dameMaximo (int *tabla, int n);
void compactaClaves (int *tabla, int *n);
void conexionSelect();


/**________________POLLCONEXIONES NUEVO_____________**/

void setearSock (struct pollfd *descriptores,int32_t nrosocketFS);
int32_t crearPollConexiones ();



/** ___________________ FUNCIONES EXTRA___________________**/

char* armarNuevoBloque(int32_t cantEscribir,int32_t offEscritura, int32_t bloqueDato, char* aEscribir);
int32_t eliminarDirectorio(char* path);
int32_t eliminarArchivo(char* path);
int32_t crearDirectorio(char* path, int32_t modo);
Ext2Directory *buscarDirectorio(Ext2Inodo* inodoQueLoContiene, int32_t nroInodo, int32_t *nrobloqueDato);
int32_t borrarEntradaDelDirectorio(char* nombreDir,Ext2Inodo *inodoDirectorio,int32_t tipo,int32_t nInodoDirectorio);
int32_t obtenerInodoQueLoContiene(char* path);
int32_t buscarInodoDelPathSinNombre(char* path,int32_t cantToken);
char * leerArchivo2(int32_t nInodo, size_t size, off_t offset);
char* dameElNombre(char *path);
int32_t tieneSoloElPyPP(int32_t inode, int32_t tipo);
int32_t insertarEntrada(Ext2Directory * directorio,Ext2Inodo* inodoDirectorio, char * nombreNuevo,int32_t nroInodoQueLoContiene);

void agregarlesLasEntradasPyPP(int32_t nroInodo,int32_t padre, int32_t modo);
int32_t crearArchivo(char* path, int32_t modo);
type_tipo_archivo tipoDeFicheroDirect(int32_t nrInodo);
int32_t escribirArchivo(int32_t nInodo, size_t size, off_t offset,
		char* buffEscritura, char* path);
int32_t leerYasiganrBloqueDeDato(int32_t nroBloque, Ext2Inodo *inodo);
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


int32_t subirOperacion(int32_t inodo, int32_t estado);
int32_t verSiEsIntocable( int32_t inodo, int32_t tipoOperacion);
void agregarEnLaListaIntocables( int32_t inodo, int32_t estado);
void sacarDeLaListaIntocables(int32_t inodo);
t_listaIntocables *crearListaIntocables();
void liberarMutexOperacion(int32_t inodo);
void eliminarValor(char operacion, const char *path);





struct nodoBloques{
	int32_t bloque;
	struct nodoBloques *next;
};

typedef struct nodoBloques t_nodoBloques;

typedef struct{
	t_nodoBloques *head;
}t_listaBloques;


void agregarEnLaListaBloques( int32_t bloque,t_listaBloques *listaBloques);
void sacarDeLaListaBloques(t_nodoBloques **nodoAux,t_listaBloques *listaBloques);
t_listaBloques *crearListaBloques();
t_listaPedidos *crearListaEnEspera();
int32_t sacarDeLaListaEnEspera(t_nodoPedido **nodoAux);
void agregarEnLaListaEspera( NIPC paquete, int32_t socketFD);


/**________________ Poll de Hilos_______________**/

struct pollThread{
	pthread_t id;
	int32_t enUso;
};

typedef struct pollThread t_pollThread;

void liberarRecursosHilos();
int32_t buscarHiloDisponible();
void inicializarHilos();
void liberarHilo(int32_t pos);
void responderPedidos();

void asignarBloqueEnInodo(int32_t nroBloque, int32_t bloqueDato, Ext2Inodo *inodo,int32_t nroInodo);
