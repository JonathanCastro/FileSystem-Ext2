#include "archivo_Configuracion.h"

void abrirConfig(void) {
	FILE *ptr;
	char nombre[32]; /*esto es para poder eliminar el nombre del txt*/
	memset(ip, 0, 16);
	if ((ptr = fopen("/home/utnso/2012-1c-thelasttime/fsc/config.txt", "r+"))
			== NULL)
		printf("ERROR, NO SE ABRIO");/*logear error*/

	else { /*aca con el FSCANF obtienen los datos, el printf es para corroborar, temporal*/
		printf("Datos de configuracion: \n");
		fscanf(ptr, "%s%s", nombre, ip);
		printf("Ip:%s\n", ip);
		fscanf(ptr, "%s%d", nombre, &puerto);
		printf("puerto: %d\n", puerto);
		fscanf(ptr, "%s%d", nombre, &cantConexiones);
		printf("cantidadConexiones: %d\n", cantConexiones);
		fscanf(ptr, "%s%s", nombre, ipCache);
		printf("ipCache:%s\n", ipCache);
		fscanf(ptr, "%s%d", nombre, &puertoCache);
		printf("puertoCache: %d\n", puertoCache);
		fscanf(ptr, "%s%d", nombre, &cache);
		printf("cache: %d\n", cache);
	}
}
