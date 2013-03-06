#ifndef EXT2_DEF_H_
#define EXT2_DEF_H_

#include <stdint.h>
#include <stdbool.h>
#include "bitarray.h"

#define TAM_PUNTERO 4

#define  CANT_BLOCK 1

#define EXT2_SUPER_MAGIC	0xEF53

#define EXT2_ROOT_INODE_INDEX	2

#define TAMSECTOR 512
#define FINARCHIVO 0xfffffff
#define LISTARDIRECTORIO 0x58
#define NOMBRELARGO 0x0F
#define ENTRADADIR 0x10
#define ENTRADAARC 0x20
#define NOHAYMASENTRADAS 0x00
#define RETORNADIRECTORIO 0x2E
#define ENTRADABORRADA 0xE5
#define HANDSHAKE 0
#define Retorno_Error 1

// t_ext2_superblock -> state
#define EXT2_VALID_FS	1	//Unmounted cleanly
#define EXT2_ERROR_FS	2	//Errors detected
// t_ext2_superblock -> errors
#define EXT2_ERRORS_CONTINUE	1	//continue as if nothing happened
#define EXT2_ERRORS_RO			2	//remount read-only
#define EXT2_ERRORS_PANIC		3	//cause a kernel panic
// t_ext2_superblock -> creator_os
#define EXT2_OS_LINUX	0	//Linux
#define EXT2_OS_HURD	1	//GNU HURD
#define EXT2_OS_MASIX	2	//MASIX
#define EXT2_OS_FREEBSD	3	//FreeBSD
#define EXT2_OS_LITES	4	//Lites
// t_ext2_superblock -> rev_level
#define EXT2_GOOD_OLD_REV	0	//Revision 0
#define EXT2_DYNAMIC_REV	1	//Revision 1 with variable inode sizes, extended attributes, etc.
// t_ext2_inode -> rev_level
#define EXT2_INODE_HAS_MODE_FLAG(inode, flag)	((inode->mode & 0xF000) == flag)

/*const uint8_t EXT2_INODES_INDIRECTION_LEVEL[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 1, 2, 3 };*/

// -- file format --
#define EXT2_IFSOCK	0xC000	//socket
#define EXT2_IFLNK	0xA000	//symbolic link
#define EXT2_IFREG	0x8000	//regular file
#define EXT2_IFBLK	0x6000	//block device
#define EXT2_IFDIR	0x4000	//directory
#define EXT2_IFCHR	0x2000	//character device
#define EXT2_IFIFO	0x1000	//fifo
// -- process execution user/group override --
#define EXT2_ISUID	0x0800	//Set process User ID
#define EXT2_ISGID	0x0400	//Set process Group ID
#define EXT2_ISVTX	0x0200	//sticky bit
// -- access rights --
#define EXT2_IRUSR	0x0100	//user read
#define EXT2_IWUSR	0x0080	//user write
#define EXT2_IXUSR	0x0040	//user execute
#define EXT2_IRGRP	0x0020	//group read
#define EXT2_IWGRP	0x0010	//group write
#define EXT2_IXGRP	0x0008	//group execute
#define EXT2_IROTH	0x0004	//others read
#define EXT2_IWOTH	0x0002	//others write
#define EXT2_IXOTH	0x0001	//others execute
/**
 * @name Flags Inodo
 * @{
 */
/** Secure deletion. */
#define EXT2_SECRM_FL           0x00000001
/** Undelete. */
#define EXT2_UNRM_FL            0x00000002
/** Compress file. */
#define EXT2_COMPR_FL           0x00000004
/** Synchronous updates. */
#define EXT2_SYNC_FL            0x00000008
/** Immutable file. */
#define EXT2_IMMUTABLE_FL       0x00000010
/** Writes to file may only append. */
#define EXT2_APPEND_FL          0x00000020
/** Do not dump file. */
#define EXT2_NODUMP_FL          0x00000040
/** Do not update atime. */
#define EXT2_NOATIME_FL         0x00000080
/** Btree format dir. */
#define EXT2_BTREE_FL           0x00001000
/** Hash-indexed directory. */
#define EXT2_INDEX_FL           0x00001000
/** AFS directory. */
#define EXT2_IMAGIC_FL          0x00002000
/** File tail should not be merged. */
#define EXT2_NOTAIL_FL          0x00008000
/** Dirsync behaviour (directories only). */
#define EXT2_DIRSYNC_FL         0x00010000
/** Top of directory hierarchies. */
#define EXT2_TOPDIR_FL          0x00020000
/** Reserved for ext2 lib. */
#define EXT2_RESERVED_FL        0x80000000
/** User visible flags. */
#define EXT2_FL_USER_VISIBLE    0x0003DFFF
/** User modifiable flags. */
#define EXT2_FL_USER_MODIFIABLE 0x000380FF
/**
 * @} 
 */

/**
 ** DIRECCIONES AL BLOQUE DE DATO
 **/

#define EXT2_NDIR_BLOCKS 12  // Directos  
#define EXT2_IND_BLOCK EXT2_NDIR_BLOCKS  // Indirectos
#define EXT2_DIND_BLOCK (EXT2_IND_BLOCK + 1) // DoblementeIndirectos
#define EXT2_TIND_BLOCK (EXT2_DIND_BLOCK + 1) // TriplementeDirectos
#define EXT2_N_BLOCKS (EXT2_TIND_BLOCK + 1)  // Tama�o de las direcciones de un inodo.
/**
 * Estructura del Super Bloque.
 */

//Errores
#define NODIRECTORIO 0
#define NOARCHIVO 1
#define PATHERROR -1

typedef struct Ext2SuperBloque {
	int32_t s_inodes_count;
	int32_t s_blocks_count;
	int32_t s_r_blocks_count;
	int32_t s_free_blocks_count;
	int32_t s_free_inodes_count;
	int32_t s_first_data_block;
	int32_t s_log_block_size;
	int32_t s_log_frag_size;
	int32_t s_blocks_per_group;
	int32_t s_frags_per_group;
	int32_t s_inodes_per_group;
	int32_t s_mtime;
	int32_t s_wtime;
	uint16_t s_mnt_count;
	uint16_t s_max_mnt_count;
	uint16_t s_magic;
	uint16_t s_state;
	uint16_t s_errors;
	uint16_t s_minor_rev_level;
	int32_t s_lastcheck;
	int32_t s_checkinterval;
	int32_t s_creator_os;
	int32_t s_rev_level;
	uint16_t s_def_resuid;
	uint16_t s_def_resgid;
	int32_t s_first_ino;
	uint16_t s_inode_size;
	uint16_t s_block_group_nr;
	int32_t s_feature_compat;
	int32_t s_feature_incompat;
	int32_t s_feature_ro_compat;
	uint8_t s_uuid[16];
	uint8_t s_volume_name[16];
	uint8_t s_last_mounted[64];
	int32_t s_algo_bitmap;
	uint8_t s_prealloc_blocks;
	uint8_t s_prealloc_dir_blocks;
	uint16_t padding1; //(alignment)
	uint8_t s_journal_uuid[16];
	int32_t s_journal_inum;
	int32_t s_journal_dev;
	int32_t s_last_orphan;
	int32_t s_hash_seed[4];
	uint8_t s_def_hash_version;
	uint8_t reservedCharPad;
	uint16_t reservedWordPad; //padding - reserved for future expansion
	int32_t s_default_mount_options;
	int32_t s_first_meta_bg;
	int32_t reserved[190]; //Unused - reserved for future revisions
} Ext2SuperBloque;

/**
 * Estructura de La Tabla De Descriptores.
 */

typedef struct Ext2TablaDeDescriptores {
	int32_t bg_block_bitmap;
	int32_t bg_inode_bitmap;
	int32_t bg_inode_table;
	int16_t bg_free_blocks_count;
	int16_t bg_free_inodes_count;
	int16_t bg_used_dirs_count;
	int16_t bg_pad;
	int8_t bg_reserved[12];
} Ext2TablaDeDescriptores;

/**
 * Estructura de un Inodo.
 */
typedef struct Ext2Inodo {
	int16_t mode; /**< File mode. */
	uint16_t uid; /**< Low 16 bits of Owner Uid. */
	int32_t size; /**< Size in bytes. */
	int32_t atime; /**< Access time. */
	int32_t ctime; /**< Creation time. */
	int32_t mtime; /**< Modification time. */
	int32_t dtime; /**< Deletion Time. */
	uint16_t gid; /**< Low 16 bits of Group Id. */
	uint16_t linksCount; /**< Links count. */
	int32_t blocks; /**< Number of 512-byte blocks. */
	int32_t flags; /**< File flags. */
	int32_t reserved; /**< Reserved bits. */
	int32_t block[15]; /**< Pointers to blocks. */
	int32_t generation; /**< File version (for NFS). */
	int32_t fileACL; /**< File ACL. */
	int32_t directoryACL; /**< Directory ACL. */
	int32_t fragmentAddr; /**< Fragment address. */
	uint8_t fragmentNumber; /**< Fragment number. */
	uint8_t fragmentSize; /**< Fragment size. */
	uint16_t padding; /**< Padding bytes. */
	uint16_t uidHigh; /**< High 16 bits of Owner Uid. */
	uint16_t gidHigh; /**< High 16 bits of Group Id. */
	int32_t reserved2; /**< Reserved. */
} Ext2Inodo;

typedef struct Ext2Directory {
	int32_t inode;
	int16_t rec_len;
	int8_t name_len;
	int8_t file_type;
	char name[];
} Ext2Directory;

typedef enum {

	IFREG = 1, //regular file
	IFDIR = 2, //directory
	IFCHR = 3, //character device
	IFBLK = 4, //block device
	IFIFO = 5, //fifo
	IFSOCK = 6, //socket
	IFLNK = 7, //symbolic link

} type_tipo_archivo;

typedef struct {
	int32_t bg_block_bitmap;
	int32_t bg_inode_bitmap;
} __attribute__ ((__packed__))bitmapsIndices;

/**
 * Funciones de Ext2
 */
int leerSuper_Bloque();
int leerTablaDeDescriptores();
void leerBitsMapsDeInodos();
void leerBitsMapsDeBloques();
void leerIndireccionamientos(Ext2Inodo inodo);
void leerIndos(char** buff);
void leerTablaDeInodos();

void inicializarEstructuras();
void leerIndireccionamiento(Ext2Inodo *inodo);

int buscarBloqueLibre(int32_t numeroinodo);
void bloqueDePunteros(int32_t **bloqueDePunteros, int32_t bloque);
void escribirInodoTabla(int nroInodo, Ext2Inodo* inodo);
int32_t numeroGrupo(int32_t numeroBloque);
void liberarBit(t_bitarray *bitMapBloque, int32_t bloqueIndex,
		int32_t bloqueBitMap);
Ext2TablaDeDescriptores * leerEntradaTablaDescriptora(int32_t nBlockGroup);
t_bitarray *leerBitMapBloques(int nroGroupo);
void liberarBloque(int32_t bloqueALiberar);
int32_t buscarInodolibre();
void liberarInodo(int32_t numeroInodo);
//Truncar

int32_t truncarArchivo(int32_t numeroInodo, size_t nuevoTamanio);
void asignarBloqueDato(int32_t nroBloque, int32_t bloqueDato, Ext2Inodo *inodo);
void actualizarTablaDescriptora(int32_t grupo, int32_t cantBloquL,int32_t cantBloqueO, int32_t cantInodoL, int32_t cantInodoO);
void actualizarSuperBloque(int32_t cantBloquL, int32_t cantBloqueO,
		int32_t cantInodoL, int32_t cantInodoO);

//_________Listar Directorio_______________//

int32_t leerSubDirectorios(char * path);
int32_t buscarEntrada(int32_t nInodoDirectorio, char* palabra);
int32_t buscarEnBloqueDeDato(int32_t bloqueDato, char* palabra);
char* listarDirectorio(int32_t nInodo);

char * leerArchivo(int32_t nInodo, size_t size, off_t offset, char* path,
		int32_t *sizeNuevo);
int32_t leerBloqueDeDato(int32_t nroBloque, Ext2Inodo *inodo);
Ext2Inodo* leerInodoDeTabla(int nroInodo);
type_tipo_archivo tipoDeFichero(Ext2Inodo *inodo);

/** ___ NO VAN ACA___*/
void liberarTodosLosBloques(Ext2Inodo * inodo);

#endif /* EXT2_DEF_H_ */

