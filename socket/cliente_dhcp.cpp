#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

using namespace std;

#define ERROR(funcion) throw runtime_error(funcion + string(strerror(errno)))

int main(int argc, char *argv[]) {
    const auto PUERTO = 54321;

    int socket_servidor = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_servidor == -1) ERROR("socket()");

    struct sockaddr_in destino {
        .sin_family = AF_INET,
        .sin_port = htons(PUERTO),
    };
    inet_aton(argv[1], &destino.sin_addr);
    socklen_t longdestino = sizeof destino;

    if(sendto(socket_servidor, argv[2], strlen(argv[2]), 0, (struct sockaddr *) &destino, sizeof destino) == -1)
        ERROR("sendto()");
    else cout << "Solicitud enviada" << endl;

    char direccion[IP_MAXPACKET];
    auto bytes = recvfrom(socket_servidor, direccion, IP_MAXPACKET, 0, (struct sockaddr *) &destino, &longdestino);
    if(bytes == -1) ERROR("recvfrom()");
    string strdireccion(direccion, bytes);
    if(strdireccion == "0.0.0.0") {
        cout << "No hay direcciones disponibles en este momento" << endl;
    } else if(strdireccion == "Liberada") {
        cout << "Dirección liberada correctamente" << endl;
    } else {
        cout << "Recibida la dirección " << strdireccion << endl;
    }
}