#include "libFSC.h"

#define DEFAULT_FILE_CONTENT "Hello World!\n"
#define DEFAULT_FILE_NAME "hello"
#define DEFAULT_FILE_PATH "/" DEFAULT_FILE_NAME

struct t_runtime_options {
	char* welcome_msg;
} runtime_options;

/*
 * Esta Macro sirve para definir nuestros propios parametros que queremos que
 * FUSE interprete. Esta va a ser utilizada mas abajo para completar el campos
 * welcome_msg de la variable runtime_options
 */
#define CUSTOM_FUSE_OPT_KEY(t, p, v) { t, offsetof(struct t_runtime_options, p), v }

static struct fuse_operations fsc_oper = {
		.create = fsc_create,
		.open = fsc_open,
		.read = fsc_read,
		.write = fsc_write,
		.release = fsc_release,
		.truncate = fsc_truncate,
		.unlink = fsc_unlink,
		.mkdir  = fsc_mkdir,
		.readdir = fsc_readdir,
		.rmdir = fsc_rmdir,
		.getattr = fsc_getattr,	
};

/** keys for FUSE_OPT_ options */
enum {
	KEY_VERSION,
	KEY_HELP,
};
/*
 * Esta estructura es utilizada para decirle a la biblioteca de FUSE que
 * parametro puede recibir y donde tiene que guardar el valor de estos
 */
static struct fuse_opt fuse_options[] = {
		// Este es un parametro definido por nosotros
	//	CUSTOM_FUSE_OPT_KEY("--welcome-msg %s", welcome_msg, 0),

		// Estos son parametros por defecto que ya tiene FUSE
		FUSE_OPT_KEY("-V", KEY_VERSION),
		FUSE_OPT_KEY("--version", KEY_VERSION),
		FUSE_OPT_KEY("-h", KEY_HELP),
		FUSE_OPT_KEY("--help", KEY_HELP),
		FUSE_OPT_END,
};



/**_________________ INICIALIZACION FSC _______________**/

sem_t semaforo;
pthread_mutex_t bloquearCache;

/*____________Log_________*/
t_log * loguear;


void inicializarFsc(){
	pthread_mutex_init(&mutexConexiones,NULL);
	pthread_mutex_init(&bloquearCache,NULL);

	loguear = log_create("log.txt", "fsc", 1, LOG_LEVEL_DEBUG);
	sem_init(&semaforo,0,cantConexiones);
	inicializarMemcached();
	crearPoolConexionesFSC(ip,puerto);
}




int main(int argc, char *argv[]) {

	printf(" _____ _          _              _  _____ _                \n");
	printf("|_   _| |__   ___| |    __ _ ___| ||_   _(_)_ __ ___   ___  \n");
	printf("  | | | '_ \\ / _ \\ |   / _` / __| __|| | | | '_ ` _ \\ / _ \\  \n");
	printf("  | | | | | |  __/ |__| (_| \\__ \\ |_ | | | | | | | | |  __/   \n");
	printf("  |_| |_| |_|\\___|_____\\__,_|___/\\__||_| |_|_| |_| |_|\\___|   \n");

	printf("                                      _____   \n");
	printf("                                     |  ___|__  ___      \n");
	printf("                                     | |_ / __|/ __|     \n");
	printf("                                  _  |  _|\\__ \\ (__   _  \n");
	printf("                                 (_) |_|  |___/\\___| (_) \n\n");

	abrirConfig();
	inicializarFsc();
	
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	memset(&runtime_options, 0, sizeof(struct t_runtime_options));


	// Esta funcion de FUSE lee los parametros recibidos y los intepreta
	if (fuse_opt_parse(&args, &runtime_options, fuse_options, NULL) == -1){
		/** error parsing options */
		perror("Invalid arguments!");
		return EXIT_FAILURE;
	}


	if( runtime_options.welcome_msg != NULL ){
		printf("%s\n", runtime_options.welcome_msg);
	}
	return fuse_main(argc, argv, &fsc_oper, NULL);

	log_destroy(loguear);
}
