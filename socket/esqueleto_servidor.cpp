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

    // Obtener con getaddrinfo la información de la dirección del servidor,
    // controlando errores
    if(getaddrinfo(NULL, puerto, &conf, &resultado)) ERROR("getaddrinfo()");

    int unsocket = -1;
    while(unsocket = socket(resultado->ai_family, resultado->ai_socktype, resultado->ai_protocol)) {
        if(unsocket != -1) break;

        cout << "socket() - " + string(strerror(errno)) << endl;
        resultado = resultado->ai_next;
    }

   bind(unsocket, resultado->ai_addr, resultado->ai_addrlen);

    return unsocket;
}

// Función que dado un paquete y la dirección obtenida de recvfrom, informa por consola
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
    const auto PUERTO = "32323";
    
    struct sockaddr_storage dircliente; // Para almacenar la dirección del cliente
    socklen_t longdircliente;    // Para almacenar la longitud de la dirección del cliente
    char paquete[IP_MAXPACKET];  // Para almacenar el paquete recibido
    string respuesta;           // Para almacenar la respuesta a enviar al cliente
    float IBEX35 = 0.0;              // Para almacenar el valor del IBEX35, si se necesita

    int unsocket = configura_servidor(PUERTO);
    // Declarar otras variables que se necesiten

    // Configurar el socket servidor UDP
    cout << "Servidor UDP en ejecución en el puerto " << PUERTO << endl;
    while(true) {
        // Preparar las variables para recibir un paquete
        longdircliente = sizeof(dircliente);
        cout << "Esperando un paquete UDP..." << endl;
        // Esperar y recibir un paquete UDP de un cliente, controlando errores
        auto bytesrecibidos = recvfrom(unsocket, paquete, IP_MAXPACKET, 0, (struct sockaddr *) &dircliente, &longdircliente);
        cout << "hola" << endl;
        if(bytesrecibidos == -1) ERROR("recvfrom()");

        paquete[bytesrecibidos] = '\0'; // Asegurar que el paquete es un string
        informa_paquete(paquete, dircliente); // Informar de la petición recibida indicando la dirección IP de origen
        respuesta = paquete; // Para procesar la respuesta, inicialmente es el mismo mensaje recibido


        if (respuesta.substr(0, 5) == "SUBE ") {
            IBEX35 += stof(respuesta.substr(5));
            respuesta = "El valor del IBEX35 es: " + to_string(IBEX35);
        } else if (respuesta.substr(0, 5) == "BAJA ") {
            IBEX35 -= stof(respuesta.substr(5));
            respuesta = "El valor del IBEX35 es: " + to_string(IBEX35);
        } else if (respuesta == "VALOR") {
            respuesta = "El valor del IBEX35 es: " + to_string(IBEX35);
        } else {
            respuesta = "ERR Comando no reconocido";
        }


        // Informar de la petición recibida indicando la dirección IP de origen
        informa_paquete(paquete, dircliente);

        // Preparar la respuesta para el cliente
        
        sendto(unsocket, paquete, bytesrecibidos, 0, (struct sockaddr *) &dircliente, longdircliente);

        // Enviar la respuesta al cliente
    }
    
    // Cerrar el socket servidor UDP
    close(unsocket);
    return 0;
}