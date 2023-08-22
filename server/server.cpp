#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

const int BUFFER_SIZE = 1024;
const int MAX_CLIENTS = 5;

Server::Server(int port) : port(port), nextClientId(1) {
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Erro ao criar o socket" << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Erro ao associar o socket ao endereço" << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        cerr << "Erro ao escutar por conexões" << endl;
        // Pode lançar uma exceção aqui se preferir
    }
}

Server::~Server() {
    close(serverSocket);
}

void Server::HandleClient(int clientSocket, int clientId) {
    char buffer[BUFFER_SIZE] = {0};

    string clientName = "Cliente " + to_string(clientId);

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << clientName << " desconectou" << endl;

            if (clientId == 1) {
                nextClientId.store(1);
            }

            break;
        }

        cout << clientName << ": " << buffer << endl;

        const char* response = "Mensagem recebida pelo servidor";
        send(clientSocket, response, strlen(response), 0);
    }

    close(clientSocket);
    {
        unique_lock<mutex> lock(threadPoolMutex);
        cv.notify_one();
    }
}


void Server::StartListening() {
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    while (true) {
        int newSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrLen);
        if (newSocket == -1) {
            cerr << "Erro ao aceitar a conexão" << endl;
            continue;
        }

        cout << "Conexão estabelecida com o cliente" << endl;

        int clientId = nextClientId.fetch_add(1);

        unique_lock<mutex> lock(threadPoolMutex);
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
