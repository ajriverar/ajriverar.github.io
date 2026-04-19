#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string>

using namespace std;

#define ERROR(funcion) throw runtime_error(funcion + string(strerror(errno)))

// Función que, dado un puerto, configura un socket servidor UDP
int configura_servidor(const char *puerto) {
    struct addrinfo conf {
       .ai_flags = AI_PASSIVE,
       .ai_family = AF_UNSPEC,
       .ai_socktype = SOCK_DGRAM
    }, *resultado;

    if(getaddrinfo(NULL, puerto, &conf, &resultado)) ERROR("getaddrinfo()");

    int unsocket = -1;
    while(unsocket = socket(resultado->ai_family, resultado->ai_socktype, resultado->ai_protocol)) {
        if(unsocket != -1) break;

        cout << "socket() - " + string(strerror(errno)) << endl;
        resultado = resultado->ai_next;
    }

    if(bind(unsocket, resultado->ai_addr, resultado->ai_addrlen)) ERROR("bind()");

    cout << "Servidor UDP configurado en el puerto " << puerto << endl;

    return unsocket;
}

void informa_paquete(char paquete[], struct sockaddr_storage& dircliente) {
    char strdireccion[NI_MAXHOST + 1] = "\0";

    inet_ntop(dircliente.ss_family,
            dircliente.ss_family == AF_INET ?
                (void *)&((struct sockaddr_in *) &dircliente)->sin_addr :
                (void *)&((struct sockaddr_in6 *) &dircliente)->sin6_addr,
            strdireccion, NI_MAXHOST);

    in_port_t puerto = ntohs(((struct sockaddr_in*)&dircliente)->sin_port);

    cout << "El cliente " << strdireccion << " por el puerto " <<  puerto;
}


int main(int argc, char *argv[]) {
    // Cambiar aquí el puerto del servidor según lo indicado en el guion
    const auto PUERTO = "54321";

    struct sockaddr_storage dircliente; // Para almacenar la dirección del cliente
    socklen_t longdircliente;    // Para almacenar la longitud de la dirección del cliente
    char paquete[IP_MAXPACKET];  // Para almacenar el paquete recibido

    // Declarar otras variables que se necesiten
    bool direcciones[10] = {};

    // Configurar el socket servidor UDP
    int misocket = configura_servidor(PUERTO), socketcliente;

    while(true) {
        // Preparar las variables para recibir un paquete
        longdircliente = sizeof dircliente;

        // Esperar y recibir un paquete UDP de un cliente, controlando errores
        cout << "Esperando peticiones..." << endl;
        auto bytes = recvfrom(misocket, paquete, IP_MAXPACKET, 0, (struct sockaddr *) &dircliente, &longdircliente);
        if(bytes == -1) ERROR("recvfrom()"); else cout << "Paquete recibido --> ";

        // Informar de la petición recibida indicando la dirección IP de origen
        paquete[bytes] = '\0';
        informa_paquete(paquete, dircliente);

        // Preparar la respuesta para el cliente

        // Enviar la respuesta al cliente
        auto comando = string(paquete).substr(0,7);

        if(comando == "REQUEST") {
            short direccion =  32767;
            cout << " pide dirección --> ";
            for(auto idx = 0; idx < 10; idx++) {
                if(!direcciones[idx]) {
                    direccion = idx;
                    direcciones[idx] = true;
                    break;
                }
            }
            string strdireccion;
            if(direccion == 32767) {
                cout << " no hay direcciones disponibles" << endl;
                strdireccion = "0.0.0.0";
            } else {
                strdireccion = "192.168.25." + to_string(direccion+1);
                cout << "Asignada la dirección " << strdireccion << endl;
            }
            sendto(misocket, strdireccion.c_str(), strdireccion.length(), 
                MSG_WAITALL, (struct sockaddr *) &dircliente, longdircliente);
        }

        if(comando == "RELEASE") { // RELEASE 192.168.25.X
            unsigned short direccion = atoi(&paquete[19]) - 1;
            direcciones[direccion] = false;
            string strdireccion = "192.168.25." + to_string(direccion + 1);
            cout << " libera la direccion " << strdireccion << endl;
            strdireccion = "Liberada";
                sendto(misocket, strdireccion.c_str(), strdireccion.length(), 
                MSG_WAITALL, (struct sockaddr *) &dircliente, longdircliente);
        }
    }
    
    // Cerrar el socket servidor UDP
    return 0;
}