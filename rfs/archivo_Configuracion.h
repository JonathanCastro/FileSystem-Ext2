#ifndef ARCHIVO_CONFIGURACION_H_
#define ARCHIVO_CONFIGURACION_H_

#include<string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

/*ACA ABAJO PONEMOS LAS VARIABLES A OBTENER*/
char ip[16];
int puerto,cantOperaciones,cache,puertoCache,maxEnEspera;
char chs[11+1];
char pathDisco [200+1];
char ipCache[16];
int retardo;
char unidadRetardo[3];
/**__________retardo____________**/
#define EVENT_SIZE  ( sizeof (struct inotify_event) + 40)
#define BUF_LEN (EVENT_SIZE)

/*FIN de variables a poner*/

void abrirConfig(void);
int generarInotify();
void readEvento(int file_descriptor);

#endif
