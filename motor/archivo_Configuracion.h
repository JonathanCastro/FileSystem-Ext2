#ifndef ARCHIVO_CONFIGURACION_H_
#define ARCHIVO_CONFIGURACION_H_

#include<stdio.h>
#include<string.h>

/*ACA ABAJO PONEMOS LAS VARIABLES A OBTENER*/

int tamMaxCache, tamMinBloque;
int esquema;
int algEleccionLibre, algEleccionVictima;
int cantBusqFallidas;
/*FIN de variables a poner*/

int abrirConfig(void);

#endif
