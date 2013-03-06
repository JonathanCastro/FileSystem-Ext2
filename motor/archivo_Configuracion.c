#include "archivo_Configuracion.h"

int abrirConfig(void)
{
	FILE *ptr;
	char nombre[32]; /*esto es para poder eliminar el nombre del txt*/

	if((ptr=fopen("/home/utnso/2012-1c-thelasttime/motor/config.txt","r+"))== NULL){
		printf("ERROR, NO SE ABRIO");/*loguear error*/
//		esquema= 0;
//		algEleccionLibre= 0;
//		algEleccionVictima= 0;
//		cantBusqFallidas= 2;

	}else{ /*aca con el FSCANF obtienen los datos, el printf es para corroborar, temporal*/

		fscanf(ptr,"%s%d",nombre,&esquema);

        if (esquema < 0 || esquema > 1){
        	printf("-El numero de Esquema debe ser 0 o 1, usted ingreso:%d\n",esquema);
        	return -1;
        }

		fscanf(ptr,"%s%d",nombre,&algEleccionLibre);

		if (algEleccionLibre < 0 || algEleccionLibre > 1){
			printf("-El numero de Algoritmo de eleccion debe ser 0 o 1, usted ingreso: %d\n",algEleccionLibre);
			return -1;
		}

		fscanf(ptr,"%s%d",nombre,&algEleccionVictima);

		if (algEleccionVictima < 0 || algEleccionVictima > 1){
			printf("-El numero de Algoritmo de seleccion de victima debe ser 0 o 1, usted ingreso: %d\n",algEleccionVictima);
			return -1;
		}

		fscanf(ptr,"%s%d",nombre,&cantBusqFallidas);


	}
	return 0;
}
