#include "libRFS.h"

void setearSock(struct pollfd *descriptores, int32_t nrosocketFS) {
	int32_t i = 0;
	while (descriptores[i].fd != 0) {
		i++;
	}
	descriptores[i].fd = nrosocketFS;
	descriptores[i].events = POLLIN;
	descriptores[i].revents = 0;
}

int32_t crearPollConexiones() {

	printf("\n ________Conexiones___________\n");

	extern sem_t semaforoLista;
	extern t_log *loguear;
	int32_t cantSockets = 2;
	struct pollfd *pollfds;
	pollfds = (struct pollfd *) calloc(cantSockets, sizeof(struct pollfd));
	int32_t socketFS;

	socketFS = abrir_conexion(puerto);

	setearSock(pollfds, socketFS);

	/*______________Inotify_________*/

	int fdRetardo = generarInotify();

	setearSock(pollfds, fdRetardo);

	/*__________________________*/

	int32_t cantRespuestas = 0;
	int32_t cantAceptados = 0;
	int32_t timeout_msecs = -1;
	int32_t nvoSocketFS;
	int32_t valorDeRetorno = 0;
	NIPC nipcRecibido;
	int32_t i, j;
	while (1) {

		cantRespuestas = poll(pollfds, cantAceptados + cantSockets,
				timeout_msecs);
		i = 0;
		j = 0;
		while (i < cantSockets + cantAceptados && j < cantRespuestas) {

			if ((pollfds[i].revents & POLLIN) && pollfds[i].fd == socketFS) {

				nvoSocketFS = aceptar_conexion(socketFS);
				if (nvoSocketFS != -1) {
					log_info(loguear, "Se conecto nuevo Cliente : %d \n",
							nvoSocketFS);
					cantAceptados++;
					pollfds = realloc(pollfds,
							(cantAceptados + cantSockets)
									* sizeof(struct pollfd));
					memset(&pollfds[cantAceptados + cantSockets - 1], '\0',
							sizeof(struct pollfd));
					setearSock(pollfds, nvoSocketFS);

				} else if (nvoSocketFS == -1) {
					log_info(loguear, "No acepte la conexion del sock : %d \n",
							nvoSocketFS);
					break;
				}

				j++;
			} else if ((pollfds[i].revents & POLLIN)
					&& pollfds[i].fd == fdRetardo) {

				readEvento(fdRetardo);
				j++;

			} else if (pollfds[i].revents & POLLIN) {

				valorDeRetorno = recibirNIPC(pollfds[i].fd, &nipcRecibido);

				if (valorDeRetorno == 0) {
					log_info(loguear,
							"Se a desconectado el File system Client :- %d...\n",
							pollfds[i].fd);
					cantAceptados--;

				} else if (valorDeRetorno != 0) {

					agregarEnLaLista(nipcRecibido, pollfds[i].fd);
					sem_post(&semaforoLista);
					j++;
				}
			}
			i++;

		}

	}
	return 0;
}
