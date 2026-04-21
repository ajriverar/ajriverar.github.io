#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

#define ERROR(funcion) throw runtime_error(funcion + string(strerror(errno)))

int main(int argc, char *argv[]) {
    const auto PUERTO = 32323; // Cambiar por el puerto indicado en el guion

    // Crear el socket UDP para enviar un segmento al servidor
    int unsocket = socket(AF_INET, SOCK_DGRAM, 0);
	
	if (unsocket == -1) ERROR("socket()");


    // Configurar la dirección y puerto al que se enviará el segmento
    struct sockaddr_in destino {};
    destino.sin_family = AF_INET;
    destino.sin_port = htons(PUERTO);
    inet_aton(argv[1], &destino.sin_addr); // El primer argumento es la dirección IP del servidor
    
    if (sendto(unsocket, argv[2], strlen(argv[2]), 0, (struct sockaddr *) &destino, sizeof(destino)) == -1) ERROR("sendto()"); // El segundo argumento es el mensaje a enviar
    // Enviar la solicitud al servidor y controlar errores

    // Esperar la respuesta del servidor e interpretarla para indicar por consola
    // la dirección asignada, la dirección liberada o que no hay direcciones disponibles
    char respuesta[IP_MAXPACKET];
    socklen_t longdirserver = sizeof(destino);
    auto bytesrecibidos = recvfrom(unsocket, respuesta, IP_MAXPACKET, 0, (struct sockaddr *) &destino, &longdirserver);
    if (bytesrecibidos == -1) ERROR("recvfrom()");
    respuesta[bytesrecibidos] = '\0';
    cout << "Respuesta del servidor: " << respuesta << endl;


    // Cerrar el socket UDP
    close(unsocket);

    return 0;
}