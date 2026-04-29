#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;

#define ERROR(funcion) throw runtime_error(funcion + string(strerror(errno)))


// Función que dada la dirección IP del servidor, establece la conexión TCP
int conecta(char * servidor) {
    int misocket;
    const auto PUERTO_TIME = 37;

    misocket = socket(AF_INET, SOCK_STREAM, 0);
    // Convierte la dirección al formato adecuado y llama a socket() para configurar el socket

    // Llama a connect() para establecer la conexión con el servidor, controlando errores
    struct sockaddr_in destino = {}; 
    destino.sin_family = AF_INET; 
    destino.sin_port = htons(PUERTO_TIME); 
    inet_aton(servidor, &destino.sin_addr); 
    if(connect(misocket, (struct sockaddr *)&destino, sizeof destino) == -1) { 
        throw runtime_error(strerror(errno)); 
   } 
    
    return misocket;
}

int main(int argc, char *argv[]) {
    // Cambiar por el puerto indicado en el guion
    
    const auto TAM_BUFFER = 1024;

    // Controlar los argumentos de entrada para evitar errores
    

    int socket = conecta(argv[1]);

    // Formato de paquete IMC_67_1.73, leyendo de teclado y enviar la petición al servidor, controlando errores
    string peticion_str, peso_str, altura_str;
    cout << "Introduce tu peso (kg): ";
    cin >> peso_str;
    cout << "Introduce tu altura (m): ";
    cin >> altura_str;
    peticion_str = "IMC_" + peso_str + "_" + altura_str;
    
    if(send(socket, peticion_str.c_str(), peticion_str.size(), 0) == -1) ERROR("send()");
    
    // Esperar y recibir la respuesta del servidor, controlando errores
    char respuesta[TAM_BUFFER];
    if(recv(socket, respuesta, sizeof respuesta, 0) == -1) ERROR("recv()");
    
        // Mostrar la respuesta por consola
    cout << respuesta << endl;
    
        // Esperar y recibir la categoría del IMC, controlando errores
    char categoria[TAM_BUFFER];
    if(recv(socket, categoria, sizeof categoria, 0) == -1) ERROR("recv()");
    
        // Mostrar la categoría por consola
    cout << "Categoría del IMC: " << categoria << endl;
        
    close(socket);
    
    

    // Mostrar la respuesta por consola
    
    

    return 0;
}
