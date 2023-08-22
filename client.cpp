#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int BUFFER_SIZE = 1024;

Client::Client(const char* serverIP, int port) : serverIP(serverIP), port(port) {
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Erro ao criar o socket" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Erro ao conectar ao servidor" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }
}

Client::~Client() {
    close(clientSocket);
}

void Client::ConnectAndCommunicate() {
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Conexão estabelecida com o servidor" << std::endl;

    while (true) {
        std::cout << "Digite uma mensagem para o servidor: ";
        std::string message;
        std::getline(std::cin, message);

        send(clientSocket, message.c_str(), message.length(), 0);

        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Conexão encerrada pelo servidor" << std::endl;
            break;
        }

        std::cout << "Servidor: " << buffer << std::endl;
    }
}

int main() {
    const char* SERVER_IP = "172.20.11.39";
    const int PORT = 8080;
    Client client(SERVER_IP, PORT);
    client.ConnectAndCommunicate();
    return 0;
}