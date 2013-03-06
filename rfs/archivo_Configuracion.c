#include "archivo_Configuracion.h"
#include <stdio.h>
char buffer[BUF_LEN];
struct inotify_event *event;

void abrirConfig(void) {
	FILE *ptr;
	char nombre[32]; /*esto es para poder eliminar el nombre del txt*/
	memset(ip, 0, 16);
	if ((ptr = fopen("/home/utnso/2012-1c-thelasttime/rfs/config.txt", "r+")) == NULL)
		printf("ERROR, NO SE ABRIO");/*logear error*/

	else { /*aca con el FSCANF obtienen los datos, el printf es para corroborar, temporal*/

		fscanf(ptr, "%s%s", nombre, ip);
		printf("-IP:%s\n", ip);

		fscanf(ptr, "%s%d", nombre, &puerto);
		printf("-Puerto:%d\n", puerto);

		fscanf(ptr, "%s%d", nombre, &cantOperaciones);
		printf("-cantConexiones:%d\n", cantOperaciones);

		fscanf(ptr, "%s%d", nombre, &cache);
		printf("-Cache habilitada:%d\n", cache);

		fscanf(ptr, "%s%s", nombre, pathDisco);
		printf("%s\n", pathDisco);

		fscanf(ptr, "%s%s", nombre, ipCache);
		printf("-ipCache:%s\n", ipCache);

		fscanf(ptr, "%s%d", nombre, &puertoCache);
		printf("-PuertoCache:%d\n", puertoCache);

		fscanf(ptr, "%s%d", nombre, &maxEnEspera);
		printf("-cantConexiones:%d\n", maxEnEspera);

		fscanf(ptr, "%s%d", nombre, &retardo);
		printf("-retardo:%d\n", retardo);

		fscanf(ptr, "%s%s", nombre, unidadRetardo);
		printf("-unidadRetardo:%s\n", unidadRetardo);

	}
}

int generarInotify() {

	int file_descriptor = inotify_init();
	if (file_descriptor < 0) {
		perror("inotify_init");
	}
	int watch_descriptor = inotify_add_watch(file_descriptor,
			"/home/utnso/2012-1c-thelasttime/rfs", IN_MODIFY);
	//
	printf("%d", watch_descriptor);
	return file_descriptor;
}

void readEvento(int file_descriptor) {

	int length = read(file_descriptor, buffer, BUF_LEN);
	if (length < 0) {
		perror("read");
	}

	int offset = 0;

	while (offset < length) {

		event = (struct inotify_event *) &buffer[offset];

		if (event->len) {

			if (event->mask & IN_MODIFY) {

				if (!(event->mask & IN_ISDIR)) {
					if (strcmp(event->name, "config.txt")==0) {

						abrirConfig();

						if (strcmp(unidadRetardo, "us")==0) {
							retardo = retardo * 1000;
						}

					}

				}
			}
		}
		offset += sizeof(struct inotify_event) + event->len;
	}

}
