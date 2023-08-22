#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

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

string Server::GetClientIP(int clientSocket)
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    if (getpeername(clientSocket, (struct sockaddr *)&clientAddr, &addrLen) == 0)
    {
        return inet_ntoa(clientAddr.sin_addr);
    }

    return "Unknown"; // Return "Unknown" if IP retrieval fails
}

void Server::HandleClient(int clientSocket, int clientId) {
    char buffer[BUFFER_SIZE] = {0};

    string clientName = "Cliente " + to_string(clientId);
    User newUser = User(clientId, clientSocket, GetClientIP(clientId), clientName);

    {
        lock_guard<mutex> lock(threadPoolMutex);
        users.push_back(newUser);
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cerr << clientName << " desconectou" << endl;

            if (clientId == 1) {
                nextClientId.store(1);
            }

            {
                std::lock_guard<std::mutex> lock(threadPoolMutex);
                for (auto it = users.begin(); it != users.end(); ++it)
                {
                    if (it->getId() == clientId)
                    {
                        users.erase(it);
                        break; // Assuming each client has a unique ID, you can exit the loop once found
                    }
                }
            }

            break;
        }

        cout << clientName << ": " << buffer << endl;

        for (User user : users) {
            // Não enviar a mensagem para o próprio cliente
            if (user.getId() == clientId) {
                continue;
            }

            // Não enviar a mensagem para clientes mutados
            if (user.isMuted(clientId) || generalMuteList.find(user.getId()) != generalMuteList.end()) {
                continue;
            }

            send(user.getClientSocket(), buffer, bytesReceived, 0);
        }

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

    cout << "Servidor escutando na porta " << port << endl;

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

void Server::ADMINmuteUser (int id) {
    generalMuteList.insert(id);

    std::string adminMessage = "User " + std::to_string(id) + " has been muted by the admin.";
    for (User user : users)
    {
        send(user.getClientSocket(), adminMessage.c_str(), adminMessage.length(), 0);
    }
}

void Server::ADMINunmuteUser (int id) {
    generalMuteList.erase(id);

    std::string adminMessage = "User " + std::to_string(id) + " has been unmuted by the admin.";
    for (User user : users)
    {
        send(user.getClientSocket(), adminMessage.c_str(), adminMessage.length(), 0);
    }
}