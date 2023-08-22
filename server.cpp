#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = 5;

Server::Server(int port) : port(port), nextClientId(1) {
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

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        std::cerr << "Erro ao escutar por conexões" << std::endl;
        // Pode lançar uma exceção aqui se preferir
    }
}

Server::~Server() {
    close(serverSocket);
}

void Server::HandleClient(int clientSocket, int clientId) {
    char buffer[BUFFER_SIZE] = {0};

    std::string clientName = "Cliente " + std::to_string(clientId);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            std::cerr << clientName << " desconectou" << std::endl;

            if (clientId == 1) {
                nextClientId.store(1);
            }

            break;
        }

        std::cout << clientName << ": " << buffer << std::endl;

        const char* response = "Mensagem recebida pelo servidor";
        send(clientSocket, response, strlen(response), 0);
    }

    close(clientSocket);
    {
        std::unique_lock<std::mutex> lock(threadPoolMutex);
        cv.notify_one();
    }
}


void Server::StartListening() {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while (true) {
        int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (newSocket == -1) {
            std::cerr << "Erro ao aceitar a conexão" << std::endl;
            continue;
        }

        std::cout << "Conexão estabelecida com o cliente" << std::endl;

        int clientId = nextClientId.fetch_add(1);

        std::unique_lock<std::mutex> lock(threadPoolMutex);
        cv.wait(lock, [this] { return threadPool.size() < MAX_CLIENTS; });

        threadPool.emplace_back(&Server::HandleClient, this, newSocket, clientId);
    }
}

int main() {
    const int PORT = 8080;
    Server server(PORT);
    server.StartListening();

    return 0;
}
