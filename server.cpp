#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int BUFFER_SIZE = 1024;

Server::Server(int port) : port(port) {
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Erro ao criar o socket" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Erro ao associar o socket ao endereço" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Erro ao escutar por conexões" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }
}

Server::~Server() {
    close(newSocket);
    close(serverSocket);
}

void Server::StartListening() {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    char buffer[BUFFER_SIZE] = {0};

    std::cout << "Aguardando conexões na porta " << port << std::endl;

    newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
    if (newSocket == -1) {
        std::cerr << "Erro ao aceitar a conexão" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }

    std::cout << "Conexão estabelecida com o cliente" << std::endl;

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(newSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << "Conexão encerrada pelo cliente" << std::endl;
            break;
        }

        std::cout << "Cliente: " << buffer << std::endl;

        const char* response = "Mensagem recebida pelo servidor";
        send(newSocket, response, strlen(response), 0);
    }
}

int main() {
    const int PORT = 8080;
    Server server(PORT);
    server.StartListening();
    return 0;
}