#ifndef ARCHIVO_CONFIGURACION_H_
#define ARCHIVO_CONFIGURACION_H_

#include<stdio.h>
#include<string.h>

/*ACA ABAJO PONEMOS LAS VARIABLES A OBTENER*/
char ip[16];
int puerto,cantConexiones,tiempoLec,puertoCache;
char chs[11+1];
char pathDisco [200+1];
char ipCache[16];
int cache;
/*FIN de variables a poner*/

void abrirConfig(void);

#endif
