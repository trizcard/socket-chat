#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctime>
#include <chrono>

using namespace std;
#define grey "\033[0;37m"
#define green "\033[0;32m"
#define red "\033[0;31m"
#define reset "\033[0m"

Server::Server(int port) : port(port), nextClientId(1) {
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << red << "Erro ao criar o socket" << reset << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << red << "Erro ao associar o socket ao endereço" << reset << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        cerr << red << "Erro ao escutar por conexões" << reset << endl;
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
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        cout << grey << std::ctime(&now_c) << reset;

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
            cerr << red << "Erro ao aceitar a conexão" << reset << endl;
            continue;
        }

        cout << green << "Conexão estabelecida com o cliente" << reset << endl;

        int clientId = nextClientId.fetch_add(1);

        unique_lock<mutex> lock(threadPoolMutex);
        cv.wait(lock, [this] { return threadPool.size() < MAX_CLIENTS; });

        threadPool.emplace_back(&Server::HandleClient, this, newSocket, clientId);
    }
}

void Server::ADMINmuteUser (int id) {
    generalMuteList.insert(id);
}

void Server::ADMINunmuteUser (int id) {
    generalMuteList.erase(id);
}

void Server::muteUser (MuteData muteData) {
    userMuteList.insert(muteData);
}

void Server::unmuteUser (MuteData muteData) {
    userMuteList.erase(muteData);
}