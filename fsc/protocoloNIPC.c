#include "libFSC.h"

char *serializarGetAttr(getAttr paquete) {

	int32_t sizeP = sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t)
			+ paquete.tamanio;
	char*buffer;

	buffer = calloc(sizeP,sizeof(char));

	int32_t offset = 0;
	memcpy(buffer, &(paquete.size), sizeof(int32_t));
	offset = sizeof(int32_t);
	memcpy(buffer + offset, &(paquete.links), sizeof(int32_t));
	offset += sizeof(int32_t);
	memcpy(buffer + offset, &(paquete.tamanio), sizeof(int32_t));
	offset += sizeof(int32_t);
	if (paquete.tamanio > 0)
		memcpy(buffer + offset, (paquete.tipo), strlen(paquete.tipo) + 1);

	return buffer;
}

void deserializarGetAttr(char *buffer, getAttr* paquete) {
	int32_t offset = 0;

	memcpy(&paquete->size, buffer,
			sizeof(paquete->size));
	offset = sizeof(paquete->size);

	memcpy(&paquete->links, buffer + offset, sizeof(paquete->links));
	offset += sizeof(paquete->links);

	memcpy(&paquete->tamanio, buffer + offset,
			sizeof(paquete->tamanio));
	offset += sizeof(paquete->tamanio);

	if (paquete->tamanio > 0) {
		paquete->tipo = (char *) malloc(paquete->tamanio);
		memcpy(paquete->tipo, buffer + offset, paquete->tamanio);
	} else
		paquete->tipo = NULL;

}


char* serializar(NIPC paquete) {

	int32_t sizeP = sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t)
			+ sizeof(int32_t) + sizeof(int32_t)+ paquete.PayloadLength;
	char *buffer = NULL;
	buffer = malloc(sizeP);

	int32_t offset = 0;
	memcpy(buffer, &(paquete.PayloadDescriptor), sizeof(int32_t));
	offset = sizeof(int32_t);

	memcpy(buffer + offset, &(paquete.offset), sizeof(int32_t));
	offset += sizeof(int32_t);

	memcpy(buffer + offset, &(paquete.size), sizeof(int32_t));
	offset += sizeof(int32_t);

	memcpy(buffer + offset, &(paquete.modo), sizeof(int32_t));
	offset += sizeof(int32_t);

	memcpy(buffer + offset, &(paquete.PayloadLength), sizeof(int32_t));
	offset += sizeof(int32_t);

	if (paquete.PayloadLength > 0)
		memcpy(buffer + offset, (paquete.payload), paquete.PayloadLength);

	return buffer;
}

void deserializar(char *buffer, NIPC* paquete) {
	int32_t offset = 0;

	memcpy(&paquete->PayloadDescriptor, buffer,
			sizeof(paquete->PayloadDescriptor));
	offset = sizeof(paquete->PayloadDescriptor);

	memcpy(&paquete->offset, buffer + offset, sizeof(paquete->offset));
	offset += sizeof(paquete->offset);

	memcpy(&paquete->size, buffer + offset, sizeof(paquete->size));
	offset += sizeof(paquete->size);

	memcpy(&paquete->modo, buffer + offset, sizeof(paquete->modo));
	offset += sizeof(paquete->modo);

	memcpy(&paquete->PayloadLength, buffer + offset,
			sizeof(paquete->PayloadLength));
	offset += sizeof(paquete->PayloadLength);

	if (paquete->PayloadLength > 0) {
		paquete->payload = (char *) calloc(paquete->PayloadLength,sizeof(char));
		memcpy(paquete->payload, buffer + offset, paquete->PayloadLength);
	} else
		paquete->payload = NULL;

}

void crearNIPC(int32_t PayloadDescriptor, int32_t offset, int32_t size,
		int32_t PayloadLength, char *Payload, NIPC *ret ,int32_t modo) {
	ret->PayloadDescriptor = PayloadDescriptor;
	ret->offset = offset;
	ret->size = size;
	ret->modo=modo;
	ret->PayloadLength = PayloadLength;
	if (PayloadLength > 0) {
		ret->payload = (char *) calloc(ret->PayloadLength, sizeof(char));
		memcpy(ret->payload, Payload, ret->PayloadLength);

	} else
		ret->payload = NULL;
}

void chauNIPC(NIPC *paquete) {
	if (paquete->PayloadLength > 0) {
		free(paquete->payload);
		paquete->payload = NULL;
	}
}

