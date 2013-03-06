#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext2.h"
#include "libRFS.h"

#define EXT2_FT_UNKNOWN 0
#define EXT2_FT_REG_FILE 1
#define EXT2_FT_DIR	2
#define EXT2_FT_CHRDEV 3
#define EXT2_FT_BLKDEV 4
#define EXT2_FT_FIFO 5
#define EXT2_FT_SOCK 6
#define EXT2_FT_SYMLINK 7

long indirecciones[4];

/** ____________________Super Bloque______________________________________ **/

int leerSuper_Bloque() {
	extern Ext2SuperBloque *superBloque;
	char *buffLectura = NULL;
	buffLectura = (char*) malloc(1024);
	int resultado = leerSuperBloque(0, buffLectura, 1);
	if (resultado != 1) {
		superBloque = (Ext2SuperBloque*) buffLectura;

//		printf("\n--------- Super Bloque ----------\n");
//		printf("cant inodos: %d\n", superBloque->s_inodes_count);
//		printf("cant bloques: %d\n", superBloque->s_blocks_count);
//		printf("cant bloques reservados: %d\n", superBloque->s_r_blocks_count);
//		printf("bloques libres: %d\n", superBloque->s_free_blocks_count);
//		printf("inodos libres: %d\n", superBloque->s_free_inodes_count);
//		printf("primer bloque%d\n", superBloque->s_first_data_block);
//		printf("tama�o de bloque: %d\n", superBloque->s_log_block_size);
//		printf("fragmentacion de bloque: %d\n", superBloque->s_log_frag_size);
//		printf("bloques por grupo %d\n", superBloque->s_blocks_per_group);
//		printf("fragmentacion por grupo: %d\n", superBloque->s_frags_per_group);
//		printf("inodo por grupo: %d\n", superBloque->s_inodes_per_group);
//		printf("inodo size: %d\n", superBloque->s_inode_size);
//		printf("inodo rev_level: %d\n", superBloque->s_rev_level);

		return 0;
	} else {
		printf("error al leer el SuperBloque\n");
		//free(buffLectura);
		return 1;
	}
}

/** ________________Tabla Descriptora__________________________ **/

//Ext2TablaDeDescriptores * leerEntradaTablaDescriptora(int32_t nBlockGroup) {
//	extern Ext2SuperBloque *superBloque;
//	extern int32_t TAM_BLOCK;
//	char *buffLectura = NULL;
//	buffLectura = (char*) calloc(32, sizeof(char));
//	Ext2TablaDeDescriptores * tablaDescriptora = NULL;
//	//tablaDescriptora = (Ext2TablaDeDescriptores*) malloc(
//	//	sizeof(Ext2TablaDeDescriptores));
//	int32_t SIZETABLA = 32;
//	int32_t offsetTablaCorrspondiente = (SIZETABLA * nBlockGroup);
//
//	int cantBloques = CANT_BLOCK;
//	//int val = 0;
//	int cantGrupos = (superBloque->s_blocks_count
//			/ superBloque->s_blocks_per_group);
//	if ((superBloque->s_blocks_count % superBloque->s_blocks_per_group) != 0)
//		cantGrupos++;
//	if (TAM_BLOCK < (cantGrupos * 32))
//		cantBloques++;
//
//	if (TAM_BLOCK == 1024)
//		offsetTablaCorrspondiente += (TAM_BLOCK * 2);
//	else
//		offsetTablaCorrspondiente += (TAM_BLOCK * 1);
//
//	leerTdescriptora2(offsetTablaCorrspondiente, buffLectura);
//
//	tablaDescriptora = (Ext2TablaDeDescriptores *) buffLectura;
//
//	return tablaDescriptora;
//}
int leerTablaDeDescriptores() {
	extern int32_t cantGrupos;
	extern Ext2TablaDeDescriptores *tablaDescriptora;
	extern int32_t TAM_BLOCK;
	char* buffer = (char*) calloc(32 * cantGrupos,sizeof(char));
	int32_t offset;
	if (TAM_BLOCK == 1024)
		offset = (TAM_BLOCK * 2);
	else
		offset = (TAM_BLOCK * 1);

	leerTdescriptora(offset, buffer, cantGrupos);

	tablaDescriptora = (Ext2TablaDeDescriptores*) buffer;

	//printf("\n----- Tabla de Descriptores ------\n");

//	int i;
//	for (i = 0; i < cantGrupos; i++) {
//
//		printf(
//				"***** Entrada De Tabla Descriptora del Grupo De Bloque: %d  *****\n",
//				i);
//		printf("id del bitmap de bloque: %d\n",
//				tablaDescriptora[i].bg_block_bitmap);
//		printf("id del bitmap de inodo: %d\n",
//				tablaDescriptora[i].bg_inode_bitmap);
//		printf("id de la tabla de inodo: %d\n",
//				tablaDescriptora[i].bg_inode_table);
//		printf("bloques libres: %d\n", tablaDescriptora[i].bg_free_blocks_count);
//		printf("inodos libres: %d\n", tablaDescriptora[i].bg_free_inodes_count);
//		printf("numero de inodos para direc: %d\n",
//				tablaDescriptora[i].bg_used_dirs_count);
//	}

	return 1;

}

/** ___________________________Inodo__________________________________________ **/

int32_t blockGroup(int32_t numeroInodo) {
	extern Ext2SuperBloque *superBloque;
	int32_t nBlockGroup;
	nBlockGroup = ((numeroInodo - 1) / superBloque->s_inodes_per_group);
	return nBlockGroup;
}

int32_t inode_index(int32_t numeroInodo) {
	extern Ext2SuperBloque *superBloque;

	int32_t inode_index;
	inode_index = ((numeroInodo - 1) % superBloque->s_inodes_per_group);

	return inode_index;
}

Ext2Inodo* leerInodoDeTabla(int32_t nroInodo) {
	extern int32_t TAM_BLOCK;
	int32_t nBlockGroup;
	int32_t indexInode;
	extern Ext2TablaDeDescriptores *tablaDescriptora;

	Ext2Inodo* inodoLeido = NULL;
	nBlockGroup = blockGroup(nroInodo);
	indexInode = inode_index(nroInodo);


	int32_t offTabla =
			(tablaDescriptora[nBlockGroup].bg_inode_table * TAM_BLOCK);


	int32_t offInodoEnTabla = (128 * indexInode);
	char *buffLectura = NULL;
	buffLectura = (char*) calloc(128, sizeof(char*));
	leerInodo(offTabla + offInodoEnTabla, buffLectura);
	inodoLeido = (Ext2Inodo*) buffLectura;
	return inodoLeido;
}

void escribirInodoTabla(int32_t nroInodo, Ext2Inodo* inodo) {
	extern int32_t TAM_BLOCK;
	int32_t nBlockGroup;
	int32_t indexInode;
	extern Ext2TablaDeDescriptores *tablaDescriptora;

	nBlockGroup = blockGroup(nroInodo);
	indexInode = inode_index(nroInodo);

	int32_t offTabla =
			(tablaDescriptora[nBlockGroup].bg_inode_table * TAM_BLOCK);

	int32_t offInodoEnTabla = (128 * indexInode);
	char *buffLectura = NULL;
	//buffLectura = (char*) calloc(128, sizeof(char));
	buffLectura = (char*) inodo;
	escribirInodo(offTabla + offInodoEnTabla, buffLectura);
	free(buffLectura);
}

void leerTablaDeInodos() {
	extern Ext2SuperBloque *superBloque;
	int32_t i;
	int j;
	Ext2Inodo *inodo = NULL;
	for (i = 0; i < superBloque->s_inodes_per_group; i++) {
		inodo = leerInodoDeTabla(i);
		printf("***** Inodo nro: %d  *****\n", i + 1);
		printf("blocks %d: ", inodo->blocks);
		for (j = 0; j < 15; j++) {
			printf("block %d: ", inodo->block[j]);
		}
		printf("Mode %d: ", inodo->mode);
		printf("Flags %d: ", inodo->flags);
		printf("Reserved %d: ", inodo->reserved);
		printf("Size %d: ", inodo->size);
		printf("Fragment %d: ", inodo->fragmentAddr);
		printf("Ctime %d: \n", inodo->ctime);
	}

	free(inodo);
}
/** __________________________BitsMaps de Bloques e Inodos________________________________________ **/

t_bitarray *leerBitMapBloques(int32_t nroGroupo) {
	extern int32_t TAM_BLOCK;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	extern Ext2TablaDeDescriptores *tablaDescriptora;

	t_bitarray *bitMapBloque = NULL;

	leerBloqueDisco(tablaDescriptora[nroGroupo].bg_block_bitmap, buffLectura,
			1);

	bitMapBloque = bitarray_create((char*) buffLectura, TAM_BLOCK);
	//free(buffLectura);
	return bitMapBloque;
}

t_bitarray * leerBitMapInodos(int32_t nroGroupo) {
	extern int32_t TAM_BLOCK;
	char *buffLectura = NULL;

	buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	extern Ext2TablaDeDescriptores *tablaDescriptora;
	t_bitarray *bitMapInodo = NULL;

	leerBloqueDisco(tablaDescriptora[nroGroupo].bg_inode_bitmap, buffLectura,
			1);


	bitMapInodo = bitarray_create((char*) buffLectura, sizeof(buffLectura));
	//free(buffLectura);
	return bitMapInodo;
}
/*retorna el numero de grupo segun el numero de bloque */

int32_t numeroGrupo(int32_t numeroBloque) {
	extern Ext2SuperBloque *superBloque;


	int cantGrupos = (superBloque->s_blocks_count
			/ superBloque->s_blocks_per_group);
	if ((superBloque->s_blocks_count % superBloque->s_blocks_per_group) != 0)
		cantGrupos++;
	int32_t bloquesPorGrupo = superBloque->s_blocks_per_group;
	int32_t cantidadDeBloques = superBloque->s_blocks_count;
	int32_t i = bloquesPorGrupo;
	int32_t grupo = 0;

	while (i < cantidadDeBloques && grupo < cantGrupos) {

		if (numeroBloque < i) {
			break;
		}
		i = i + bloquesPorGrupo;
		grupo++;
	}
	return grupo;
}

void liberarBit(t_bitarray *bitMap, int32_t index, int32_t bloqueBitMap) {
	extern int32_t TAM_BLOCK;
	bitarray_clean_bit(bitMap, index);
	escribirBloqueDisco(bloqueBitMap, 0, TAM_BLOCK, bitMap->bitarray);
}
void ocuparBit(t_bitarray *bitMap, int32_t index, int32_t bloqueBitMap) {
	extern int32_t TAM_BLOCK;
	bitarray_set_bit(bitMap, index);
	escribirBloqueDisco(bloqueBitMap, 0, TAM_BLOCK, bitMap->bitarray);
}

/*Bit Map Bloque -- Liberar y Ocupar*/

int32_t indiceBloque(int32_t bloque, int32_t grupo) {
	extern int32_t TAM_BLOCK;
	extern Ext2SuperBloque *superBloque;


	int32_t indice = (-(superBloque->s_blocks_per_group * (grupo + 1))
			+ (bloque - 1) + superBloque->s_blocks_per_group);

	if(TAM_BLOCK== 2048 || TAM_BLOCK == 4096){
			indice++;
		}
	return indice;
}

int32_t numeroDeBloque(int32_t grupo, int32_t indice) {
	extern int32_t TAM_BLOCK;
	extern Ext2SuperBloque *superBloque;

	int32_t bloque = ((superBloque->s_blocks_per_group * (grupo + 1))
			- ((superBloque->s_blocks_per_group-1) - indice ));


	if(TAM_BLOCK== 2048 || TAM_BLOCK == 4096){
		bloque--;
	}
	return bloque;
}

void liberarBloque(int32_t bloqueALiberar) {
	extern pthread_mutex_t BitMapBloque;
	pthread_mutex_lock(&BitMapBloque);

	extern Ext2TablaDeDescriptores *tablaDescriptora;
	int32_t ngrupo = numeroGrupo(bloqueALiberar);
	t_bitarray * bitMapBloque = NULL;
	bitMapBloque = leerBitMapBloques(ngrupo);
	int32_t indice = indiceBloque(bloqueALiberar, ngrupo);


	liberarBit(bitMapBloque, indice, tablaDescriptora[ngrupo].bg_block_bitmap);

	actualizarTablaDescriptora(ngrupo, 1, 0, 0, 0);
	bitarray_destroy(bitMapBloque);
	pthread_mutex_unlock(&BitMapBloque);

}

void liberarTodosLosBloques(Ext2Inodo * inodo) {
	extern int32_t TAM_BLOCK;
	int32_t cantidadBloques = inodo->size / TAM_BLOCK;
	if ((inodo->size % TAM_BLOCK) != 0) {
		cantidadBloques++;
	}
	int32_t i;
	for (i = 1; i <= cantidadBloques; i++) {
		int32_t bloqueDato = leerBloqueDeDato(i, inodo);
		liberarBloque(bloqueDato);
	}
	actualizarSuperBloque(cantidadBloques, 0, 0, 0);

}

int32_t buscarBloqueLibre(int32_t numeroinodo) {
	extern pthread_mutex_t BitMapBloque;
	pthread_mutex_lock(&BitMapBloque);
	extern int32_t cantGrupos;
	extern Ext2TablaDeDescriptores *tablaDescriptora;

	extern Ext2SuperBloque *superBloque;
	int32_t grupo=0;
	t_bitarray *bitMapBloque = NULL;

	while(grupo < cantGrupos){
		if(tablaDescriptora[grupo].bg_free_blocks_count != 0){
			break;
		}
		grupo++;
	}
	if (grupo < cantGrupos) {

		bitMapBloque = leerBitMapBloques(grupo);

		int bit = 1;
		int32_t i = 0;

		while (bit != 0 && i <= (superBloque->s_blocks_per_group - 1)) {
			bit = bitarray_test_bit(bitMapBloque, i);
			i++;
		}

		ocuparBit(bitMapBloque, i - 1, tablaDescriptora[grupo].bg_block_bitmap);

		actualizarTablaDescriptora(grupo, 0, 1, 0, 0);
		bitarray_destroy(bitMapBloque);
		pthread_mutex_unlock(&BitMapBloque);

		return numeroDeBloque(grupo, i - 1);
	} else {

		pthread_mutex_unlock(&BitMapBloque);
		return -1;
	}

}

/*Bit Map Inodo -- Liberar y Ocupar*/

int32_t numeroDeInodo(int32_t grupo, int32_t indice) {
	extern Ext2SuperBloque *superBloque;
	int32_t ninodo = ((superBloque->s_inodes_per_group * (grupo + 1))
			- (superBloque->s_inodes_per_group - (indice + 1)));

	return ninodo;
}
int32_t indiceInodo(int32_t numeroInodo, int32_t grupo) {
	extern Ext2SuperBloque *superBloque;



	int32_t indice = (-(superBloque->s_inodes_per_group * (grupo + 1))
			+ (numeroInodo - 1) + superBloque->s_inodes_per_group);

	return indice;
}
int32_t buscarInodolibre() {
	extern pthread_mutex_t BitMapInodo;
	pthread_mutex_lock(&BitMapInodo);
	extern int32_t cantGrupos;
	extern Ext2TablaDeDescriptores *tablaDescriptora;
	extern Ext2SuperBloque *superBloque;
	int32_t grupo=0;
	t_bitarray *bitMapInodo = NULL;



	while(grupo < cantGrupos){
		if(tablaDescriptora[grupo].bg_free_inodes_count != 0){
			break;
		}
		grupo++;
	}
	if (grupo < cantGrupos) {

		bitMapInodo = leerBitMapInodos(grupo);
		int bit = 1;
		int32_t i = 0;

		while (bit != 0 && i <= (superBloque->s_inodes_per_group - 1)) {

			bit = bitarray_test_bit(bitMapInodo, i);
			i++;
		}

		ocuparBit(bitMapInodo, i - 1, tablaDescriptora[grupo].bg_inode_bitmap);

		actualizarTablaDescriptora(grupo, 0, 0, 0, 1);
		bitarray_destroy(bitMapInodo);


		pthread_mutex_unlock(&BitMapInodo);
		return numeroDeInodo(grupo, i - 1);
	} else {

		pthread_mutex_unlock(&BitMapInodo);
		return -1;
	}
}

void liberarInodo(int32_t numeroInodo) {

	extern pthread_mutex_t BitMapInodo;
	pthread_mutex_lock(&BitMapInodo);
	extern Ext2TablaDeDescriptores *tablaDescriptora;
	int32_t ngrupo = blockGroup(numeroInodo);
	t_bitarray * bitMapInodo = NULL;
	bitMapInodo = leerBitMapInodos(ngrupo);
	int32_t indice = indiceInodo(numeroInodo, ngrupo);
	liberarBit(bitMapInodo, indice, tablaDescriptora[ngrupo].bg_inode_bitmap);
	actualizarTablaDescriptora(ngrupo, 0, 0, 1, 0);
	free(bitMapInodo);
	pthread_mutex_unlock(&BitMapInodo);
}

//-----------------Lectura de Los Bitmap--------------//

void leerBitsMapsDeBloques() {
	extern Ext2SuperBloque *superBloque;
	extern int32_t TAM_BLOCK;
	int i;
	int32_t j;
	bool resultado;
	t_bitarray *bitMapBloque = NULL;


	int cantGrupos = (superBloque->s_blocks_count
			/ superBloque->s_blocks_per_group);
	int cantl = 0;

	if ((superBloque->s_blocks_count % superBloque->s_blocks_per_group) != 0)
		cantGrupos++;


	for (j = 0; j < cantGrupos; j++) {
		printf("----- BitMap Bloques Del grupo n° %d\n", j);
		bitMapBloque = leerBitMapBloques(j);
		for (i = 0; i < TAM_BLOCK * 8; i++) {
			resultado = bitarray_test_bit(bitMapBloque, i);
			if (resultado == 0)
				cantl++;
			//printf("%d ,%d\n", i, resultado);
		}

		printf("cant libresssssssss,%d\n", cantl);
		bitMapBloque = NULL;
		cantl = 0;
	}
	bitarray_destroy(bitMapBloque);
}

void leerBitsMapsDeInodos() {
	extern Ext2SuperBloque *superBloque;
	extern int32_t TAM_BLOCK;
	int i;
	int32_t j;
	bool resultado;
	t_bitarray *bitMapInode = NULL;

	int cantGrupos = (superBloque->s_blocks_count
			/ superBloque->s_blocks_per_group);
	int cantl = 0;

	if ((superBloque->s_blocks_count % superBloque->s_blocks_per_group) != 0)
		cantGrupos++;


	for (j = 0; j < cantGrupos; j++) {
		printf("----- BitMap Inode Del grupo n° %d\n", j);
		bitMapInode = leerBitMapInodos(j);
		for (i = 0; i < TAM_BLOCK * 8; i++) {
			resultado = bitarray_test_bit(bitMapInode, i);
			if (resultado == 0)
				cantl++;
			//printf("%d ,%d\n", i, resultado);
		}
		printf("cant libresssssssss,%d\n", cantl);
		cantl = 0;
	}
	bitarray_destroy(bitMapInode);
}
/** ____________________Imprime los indireccionamiento que posee_____________________________**/

void bloqueDePunteros(int32_t **bloqueDePunteros, int32_t bloque) {
	extern int32_t TAM_BLOCK;
	char* buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	leerBloqueDisco(bloque, buffLectura, 1);
	*bloqueDePunteros = (int32_t*) buffLectura;
}

void leerPunterosDirectos(int32_t nrobloque) {

	printf("Nro de Bloque de Datos: %d \n", nrobloque);
//agregarElemento(listaDatos,nrobloque);

}

void leerPunterosIndirectos(int32_t bloque) {
	extern int32_t BLOQXPUNT;
	int32_t* bloqueDeP;
	bloqueDePunteros(&bloqueDeP, bloque);
	int32_t i, indirectos;
	indirectos = 0;
	for (i = 0; i < (BLOQXPUNT) && (indirectos != -1); i++) {
		if (bloqueDeP[i] != 0)
			leerPunterosDirectos(bloqueDeP[i]); /** Quiero que aca me ponga en la lista un nuevo BLOQUE, pero ademas que me la
			 Retorne por referencia para poder usarla en LeerIndireccionamiento():  **/
		else
			indirectos = -1;

	}
}

void leerPunterosIndirectosDobles(int32_t bloque) {
	int32_t* bloqueDeP;
	int32_t i, dobles;
	dobles = 0;
	extern int32_t BLOQXPUNT;
	bloqueDePunteros(&bloqueDeP, bloque);

	for (i = 0; i < (BLOQXPUNT) && (dobles != -1); i++) {
		if (bloqueDeP[i] != 0)
			leerPunterosIndirectos(bloqueDeP[i]);
		else
			dobles = -1;
	}
}

void leerPunterosIndirectosTriples(int32_t bloque) {
	int32_t *bloqueDeP;
	int32_t i, triples;
	triples = 0;
	extern int32_t BLOQXPUNT;
	bloqueDePunteros(&bloqueDeP, bloque);
	for (i = 0; i < (BLOQXPUNT) && (triples != -1); i++) {
		if (bloqueDeP[i] != 0)
			leerPunterosIndirectosDobles(bloqueDeP[i]);
		else
			triples = -1;
	}
}

void leerIndireccionamiento(Ext2Inodo *inodo) {

	int32_t i;

	for (i = 0; i < 12; i++) {
		if (inodo->block[i] != 0) {
			leerPunterosDirectos(inodo->block[i]);
		} else {
			i = 15;
		}
	}
	if (inodo->block[12] != 0) {
		leerPunterosIndirectos(inodo->block[12]);
		if (inodo->block[13] != 0) {
			leerPunterosIndirectosDobles(inodo->block[13]);
			if (inodo->block[14] != 0) {
				leerPunterosIndirectosTriples(inodo->block[14]);
			}
		}
	}
}

/** ___________________________________________________________________________________________ **/
/*************************** Leer Indireccionamiento *********************************************/

int32_t buscarEnIndirectos(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroBloque -= bloquesAnt;
	bloque = bloquesDatos[nroBloque - 1]; // menos 1 porque el array arranca desde 0  :$
	free(buffLectura);
	return bloque;
}

int32_t buscarEnIndirectosDobles(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	extern int32_t BLOQXPUNT;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	int32_t nroPunt = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;

	nroPunt = ((nroBloque - 1 - bloquesAnt) / BLOQXPUNT);
	bloque = buscarEnIndirectos(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * BLOQXPUNT) + bloquesAnt));

	free(buffLectura);
	return bloque;
}

int32_t buscarEnIndirectosTriples(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	extern int32_t BLOQXPUNT;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	int32_t nroPunt = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroPunt = ((nroBloque - 1 - bloquesAnt) / (BLOQXPUNT * BLOQXPUNT));
	bloque = buscarEnIndirectosDobles(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * BLOQXPUNT * BLOQXPUNT) + bloquesAnt));

	free(buffLectura);
	return bloque;

}

int32_t buscarBloqueDatos(int32_t nroBloque, int32_t block[]) {
	extern int32_t BLOQXPUNT;
	int32_t inicio = 13;
	int32_t fin = BLOQXPUNT;
	int32_t bloqueDatos = 0;

	if (nroBloque >= inicio && nroBloque <= (fin + inicio - 1)) {
		bloqueDatos = buscarEnIndirectos(nroBloque, block[12], 12);
	} else {
		if (nroBloque >= (inicio + fin)
				&& nroBloque <= ((fin * fin) + (inicio + fin - 1)))
			bloqueDatos = buscarEnIndirectosDobles(nroBloque, block[13],
					(fin + inicio - 1));
		else
			bloqueDatos = buscarEnIndirectosTriples(nroBloque, block[14],
					((fin * fin) + (inicio + fin - 1)));
	}
	return bloqueDatos;
}

int32_t leerBloqueDeDato(int32_t nroBloque, Ext2Inodo *inodo) {

	if (nroBloque <= 12) // Es directo
		return inodo->block[nroBloque - 1];

	int32_t bloqueDatos = buscarBloqueDatos(nroBloque, inodo->block);

	return bloqueDatos;
}

/*________________________________________*/

type_tipo_archivo tipoDeFichero(Ext2Inodo *inodo) {

	type_tipo_archivo tipoDeArchivo;

	if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFREG))
		tipoDeArchivo = IFREG;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFDIR))
		tipoDeArchivo = IFDIR;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFCHR))
		tipoDeArchivo = IFCHR;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFBLK))
		tipoDeArchivo = IFBLK;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFIFO))
		tipoDeArchivo = IFIFO;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFSOCK))
		tipoDeArchivo = IFSOCK;
	else
		tipoDeArchivo = IFLNK;

//free(inodo);
	return tipoDeArchivo;

}

type_tipo_archivo tipoDeFicheroDirect(int32_t nrInodo) {

	Ext2Inodo *inodo = (Ext2Inodo*) malloc(sizeof(Ext2Inodo));
	inodo = leerInodoDeTabla(nrInodo);
	type_tipo_archivo tipoDeArchivo;

	if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFREG))
		tipoDeArchivo = IFREG;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFDIR))
		tipoDeArchivo = IFDIR;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFCHR))
		tipoDeArchivo = IFCHR;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFBLK))
		tipoDeArchivo = IFBLK;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFIFO))
		tipoDeArchivo = IFIFO;
	else if (EXT2_INODE_HAS_MODE_FLAG(inodo,EXT2_IFSOCK))
		tipoDeArchivo = IFSOCK;
	else
		tipoDeArchivo = IFLNK;

//free(inodo);
	return tipoDeArchivo;

}

/**_________________________ASIGNAR BLOQUE DE DATO______________________**/

void asignarEnIndirectos(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt, int32_t bloqueDato) {
	extern int32_t TAM_BLOCK;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroBloque -= bloquesAnt;
	bloquesDatos[nroBloque - 1] = bloqueDato;

	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos);

	free(buffLectura);
}

void asignarEnIndirectosDobles(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt, int32_t bloqueDato, int32_t inodo) {
	extern int32_t TAM_BLOCK;
	extern int32_t cantPunteros;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t nroPunt;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroPunt = (nroBloque - 1 - bloquesAnt) / cantPunteros;

	if (bloquesDatos[nroPunt] == 0) {
		bloquesDatos[nroPunt] = buscarBloqueLibre(inodo);
		actualizarSuperBloque(0, 1, 0, 0);
	}

	asignarEnIndirectos(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * cantPunteros) + bloquesAnt), bloqueDato);

	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos); // nose si va!

	free(buffLectura);

}

void asignarEnIndirectosTriples(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt, int32_t bloqueDato, int32_t inodo) {
	extern int32_t cantPunteros;
	extern int32_t TAM_BLOCK;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t nroPunt;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroPunt = ((nroBloque - 1 - bloquesAnt) / (cantPunteros * cantPunteros));

	if (bloquesDatos[nroPunt] == 0) {
		bloquesDatos[nroPunt] = buscarBloqueLibre(inodo);
		actualizarSuperBloque(0, 1, 0, 0);
	}

	asignarEnIndirectosDobles(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * cantPunteros * cantPunteros) + bloquesAnt), bloqueDato,
			inodo);

	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos);

	free(buffLectura);

}

void asignarEnIndirecciones(int32_t nroBloque, int32_t bloqueDato,
		int32_t block[], int32_t inodo) {
	extern int32_t cantPunteros;

	int32_t inicio = 13;
	int32_t fin = cantPunteros;
	if (nroBloque >= inicio && nroBloque <= (fin + inicio - 1)) {
		if (block[12] == 0) {
			int32_t bloquelibre = buscarBloqueLibre(inodo);
			actualizarSuperBloque(0, 1, 0, 0);
			block[12] = bloquelibre;
		}

		asignarEnIndirectos(nroBloque, block[12], 12, bloqueDato);

	} else {

		if (nroBloque >= (inicio + fin)
				&& nroBloque <= ((fin * fin) + (inicio + fin - 1))) {
			if (block[13] == 0) {
				block[13] = buscarBloqueLibre(inodo);
				actualizarSuperBloque(0, 1, 0, 0);
			}

			asignarEnIndirectosDobles(nroBloque, block[13], (fin + inicio - 1),
					bloqueDato, inodo);
		} else {
			if (block[14] == 0) {
				block[14] = buscarBloqueLibre(inodo);
				actualizarSuperBloque(0, 1, 0, 0);
			}

			asignarEnIndirectosTriples(nroBloque, block[14],
					((fin * fin) + (inicio + fin - 1)), bloqueDato, inodo);
		}
	}

}

void asignarBloqueEnInodo(int32_t nroBloque, int32_t bloqueDato,
		Ext2Inodo *inodo, int32_t nroInodo) {
	if (nroBloque <= 12) // Es directo
		inodo->block[nroBloque - 1] = bloqueDato;
	else
		asignarEnIndirecciones(nroBloque, bloqueDato, inodo->block, nroInodo);

}

/**_________________________________________________________**/

//Actualizar el SuperBloque//
void actualizarSuperBloque(int32_t cantBloquL, int32_t cantBloqueO,
		int32_t cantInodoL, int32_t cantInodoO) {

	extern Ext2SuperBloque *superBloque;


	if (cantBloquL != 0)
		superBloque->s_free_blocks_count = superBloque->s_free_blocks_count
				+ cantBloquL;
	else
		superBloque->s_free_blocks_count = superBloque->s_free_blocks_count
				- cantBloqueO;
	if (cantInodoL != 0)
		superBloque->s_free_inodes_count = superBloque->s_free_inodes_count
				+ cantInodoL;
	else
		superBloque->s_free_inodes_count = superBloque->s_free_inodes_count
				- cantInodoO;

	escribirSuperBloque((char*) superBloque);

}

void actualizarTablaDescriptora(int32_t grupo, int32_t cantBloquL,
		int32_t cantBloqueO, int32_t cantInodoL, int32_t cantInodoO) {
	extern int32_t TAM_BLOCK;
	extern int32_t cantGrupos;
	extern Ext2TablaDeDescriptores *tablaDescriptora;


	if (cantBloquL != 0)
		tablaDescriptora[grupo].bg_free_blocks_count =
				tablaDescriptora[grupo].bg_free_blocks_count + cantBloquL;
	else
		tablaDescriptora[grupo].bg_free_blocks_count =
				tablaDescriptora[grupo].bg_free_blocks_count - cantBloqueO;
	if (cantInodoL != 0)
		tablaDescriptora[grupo].bg_free_inodes_count =
				tablaDescriptora[grupo].bg_free_inodes_count + cantInodoL;
	else
		tablaDescriptora[grupo].bg_free_inodes_count =
				tablaDescriptora[grupo].bg_free_inodes_count - cantInodoO;

	int32_t offsetTablaCorrspondiente;

	if (TAM_BLOCK == 1024)
		offsetTablaCorrspondiente = (TAM_BLOCK * 2);
	else
		offsetTablaCorrspondiente = (TAM_BLOCK * 1);

	escribirTdescriptora(offsetTablaCorrspondiente, (char*) tablaDescriptora,
			cantGrupos);

}

/** ___________________________________________________________________________________________ **/
/*************************** Leer y Asignar *********************************************/

int32_t buscarYasignarEnIndirectos(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroBloque -= bloquesAnt;
	bloque = bloquesDatos[nroBloque - 1];

//liberarBloque(bloquesDatos[nroBloque - 1]);
	bloquesDatos[nroBloque - 1] = 0;
	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos);

// menos 1 porque el array arranca desde 0  :$
	free(buffLectura);
	return bloque;
}

int32_t buscarYasignarEnIndirectosDobles(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	extern int32_t BLOQXPUNT;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	int32_t nroPunt = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;

	nroPunt = ((nroBloque - 1 - bloquesAnt) / BLOQXPUNT);
	bloque = buscarYasignarEnIndirectos(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * BLOQXPUNT) + bloquesAnt));

	if (nroBloque == ((nroPunt * BLOQXPUNT) + bloquesAnt + 1)) {
		liberarBloque(bloquesDatos[nroPunt]);
		actualizarSuperBloque(1, 0, 0, 0);
		bloquesDatos[nroPunt] = 0;
	}
	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos);

	free(buffLectura);
	return bloque;
}

int32_t buscarYasignarEnIndirectosTriples(int32_t nroBloque, int32_t bloquePunt,
		int32_t bloquesAnt) {
	extern int32_t TAM_BLOCK;
	extern int32_t BLOQXPUNT;
	char *buffLectura = (char*) calloc(TAM_BLOCK, sizeof(char));
	int32_t bloque = 0;
	int32_t nroPunt = 0;
	leerBloqueDisco(bloquePunt, buffLectura, 1);
	int32_t *bloquesDatos = (int32_t*) buffLectura;
	nroPunt = ((nroBloque - 1 - bloquesAnt) / (BLOQXPUNT * BLOQXPUNT));
	bloque = buscarYasignarEnIndirectosDobles(nroBloque, bloquesDatos[nroPunt],
			((nroPunt * BLOQXPUNT * BLOQXPUNT) + bloquesAnt));
	if (nroBloque == ((nroPunt * BLOQXPUNT * BLOQXPUNT) + bloquesAnt + 1)) {
		liberarBloque(bloquesDatos[nroPunt]);
		actualizarSuperBloque(1, 0, 0, 0);
		bloquesDatos[nroPunt] = 0;
	}
	escribirBloqueDisco(bloquePunt, 0, TAM_BLOCK, (char*) bloquesDatos);

	free(buffLectura);
	return bloque;

}

int32_t buscarBloqueDatosYasignar(int32_t nroBloque, int32_t block[]) {
	extern int32_t BLOQXPUNT;
	int32_t inicio = 13;
	int32_t fin = BLOQXPUNT;
	int32_t bloqueDatos = 0;

	if (nroBloque >= inicio && nroBloque <= (fin + inicio - 1)) {
		bloqueDatos = buscarYasignarEnIndirectos(nroBloque, block[12], 12);
		if (nroBloque == inicio) {
			liberarBloque(block[12]);
			actualizarSuperBloque(1, 0, 0, 0);
			block[12] = 0;
		}
	} else {
		if (nroBloque >= (inicio + fin)
				&& nroBloque <= ((fin * fin) + (inicio + fin - 1))) {
			bloqueDatos = buscarYasignarEnIndirectosDobles(nroBloque, block[13],
					(fin + inicio - 1));
			if (nroBloque == (inicio + fin)) {
				liberarBloque(block[13]);
				actualizarSuperBloque(1, 0, 0, 0);
				block[13] = 0;
			}

		} else {
			bloqueDatos = buscarYasignarEnIndirectosTriples(nroBloque,
					block[14], ((fin * fin) + (inicio + fin - 1)));

			if (nroBloque == ((fin * fin) + (inicio + fin))) {
				liberarBloque(block[14]);
				actualizarSuperBloque(1, 0, 0, 0);
				block[14] = 0;
			}
		}

	}
	return bloqueDatos;
}

int32_t leerYasiganrBloqueDeDato(int32_t nroBloque, Ext2Inodo *inodo) {
	int32_t bloqueDatos;
	if (nroBloque <= 12) { // Es directo
		bloqueDatos = inodo->block[nroBloque - 1];
		inodo->block[nroBloque - 1] = 0;
	} else {
		bloqueDatos = buscarBloqueDatosYasignar(nroBloque, inodo->block);
	}

	return bloqueDatos;
}

