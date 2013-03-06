#ifndef MANEJARDISCO_H_
#define MANEJARDISCO_H_

#include "ext2.h"

void abrirArchivoDisco(void);
int leerBloqueDisco(int bloque, char* buffLectura,int cantB);
int escribirBloqueDisco(int bloque,int offet,int size ,char* buffEscritura)
int leerSuperBloque(int bloque, char* buffLectura,int cantB);
//int leerSuperBloque2(int bloque,Ext2SuperBloque* superBloque,int cantB);
int leerInodo(int offset,char *buffLectura);
int escribirInodo(int offset, char*buffEscritura);
int leerTdescriptora(int offset,char *buffLectura);
#endif /* MANEJARDISCO_H_ */
