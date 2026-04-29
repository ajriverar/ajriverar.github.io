#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <fstream>

using namespace std;

#define ERROR(funcion) throw runtime_error(funcion + string(strerror(errno)))

// Función que, dado un puerto, configura un socket servidor TCP
int configura_servidor(const char *puerto) {
    struct addrinfo conf = {
       .ai_flags = AI_PASSIVE,
       .ai_family = AF_UNSPEC,
       .ai_socktype = SOCK_STREAM
    }, *resultado;

    if(getaddrinfo(NULL, puerto, &conf, &resultado)) ERROR("getaddrinfo()");

    int unsocket = -1;
    while(unsocket = socket(resultado->ai_family, resultado->ai_socktype, resultado->ai_protocol), unsocket == -1 && resultado != NULL) {
        if(unsocket != -1) break;

        cout << "socket() - " + string(strerror(errno)) << endl;
        resultado = resultado->ai_next;
    }

    // Importante configurar el socket para evitar el error de que ya está 
    // en uso cuando se interrumpe el servidor y se vuelve a iniciar 
    int r = 1; setsockopt(unsocket, SOL_SOCKET, SO_REUSEADDR, &r, sizeof(r)); 
    // Llamar a bind para asociar el socket a la dirección obtenida, controlando errores
    if(bind(unsocket, resultado->ai_addr, resultado->ai_addrlen)) ERROR("bind()"); 
    // Llamar a listen para poner el socket en modo escucha, controlando errores
    if(listen(unsocket, 10)) ERROR("listen()"); 

    return unsocket;
}

// Función que dado una dirección sockaddr_storage, devuelve su representación en string
string obten_direccion(struct sockaddr_storage *direccion) {
    char strdireccion[NI_MAXHOST + 1];

    inet_ntop(direccion->ss_family,
            direccion->ss_family == AF_INET ?
                (void *)&((struct sockaddr_in *) direccion)->sin_addr :
                (void *)&((struct sockaddr_in6 *) direccion)->sin6_addr,
            strdireccion, NI_MAXHOST);

    return string(strdireccion);
}

void notifica_cliente(struct sockaddr_storage *direccion) { 
char strdireccion[NI_MAXHOST + 1]; 
    inet_ntop(direccion->ss_family, 
            direccion->ss_family == AF_INET ? 
                 (void *)&((struct sockaddr_in *) direccion)->sin_addr :
                 (void *)&((struct sockaddr_in6 *) direccion)->sin6_addr, 
            strdireccion, NI_MAXHOST); 
    cout << "Conexión desde " << strdireccion << endl; 
}

void envia_respuesta(int socketcliente, string peticion_str) { 
     
    // Formato de petición: "IMC peso altura", por ejemplo: "IMC_70_1.75"

    // Extraer el peso y la altura de la petición, calcular el IMC y preparar la respuesta
    auto peso = stof(peticion_str.substr(4, peticion_str.find('_', 4) - 4));
    auto altura = stof(peticion_str.substr(peticion_str.find('_', 4) + 1));
    auto imc = peso / (altura * altura);
    auto respuesta = "IMC: " + to_string(imc);

    // Enviar la respuesta al cliente, controlando errores
    if(send(socketcliente, respuesta.c_str(), respuesta.size(), 0) == -1) ERROR("send()");

    //Notificar la categoría del IMC al cliente
    string categoria;   
    if(imc < 18.5) categoria = "Bajo peso: IMC menor a 18.5";
    else if(imc < 25) categoria = "Peso normal: IMC entre 18.5 y 24.9";
    else if(imc < 30) categoria = "Sobrepeso: IMC entre 25 y 29.9";
    else categoria = "Obesidad: IMC mayor o igual a 30 ";

    if(send(socketcliente, categoria.c_str(), categoria.size(), 0) == -1) ERROR("send()");
    

    close(socketcliente); 
}

int main(int argc, char *argv[]) {
    // Cambiar aquí el puerto del servidor según lo indicado en el guion
    const auto PUERTO = "55333";
    const auto TAM_BUFFER = 1024;
    struct sockaddr_storage dircliente; // Para almacenar la dirección del cliente
    socklen_t longdircliente;
    char peticion[TAM_BUFFER], respuesta[TAM_BUFFER];
    string peticion_str;

    // Declarar otras variables que se consideren necesarias

    int misocket = configura_servidor(PUERTO), socketcliente;

    while(true) {

        longdircliente = sizeof dircliente; 

        auto socketcliente = accept(misocket,(struct sockaddr *)&dircliente, &longdircliente); 

        auto bytesrecibidos = recv(socketcliente, peticion, sizeof peticion, 0);
        peticion_str = string(peticion, bytesrecibidos);

        if(socketcliente != -1) { 
            if(peticion_str.substr(0, 3) == "IMC") {
            notifica_cliente(&dircliente); 
            envia_respuesta(socketcliente, peticion_str); 
        } 
        else {
            cout << "Petición no reconocida: " << peticion_str << endl; 
            close(socketcliente); 
        }
        // Esperar la petición de conexión de un cliente con accept, controlando errores

        // Indicar por consola la dirección del cliente que se ha conectado

        // Extraer la petición del cliente y procesarla

        // Enviar la respuesta apropiada al cliente, controlando posibles errores

        close(socketcliente);
        }
    }
    close(misocket);
    return 0;
}
