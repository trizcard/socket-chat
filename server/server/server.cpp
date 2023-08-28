#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <chrono>
#include <fstream>

using namespace std;

Server::Server(int port) : port(port), nextClientId(1)
{
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        printServerError("Erro ao criar o socket");
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printServerError("Erro ao associar o socket ao endereço");
        return;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1)
    {
        printServerError("Erro ao escutar por conexões");
        return;
    }
}

Server::~Server()
{
    close(serverSocket);
}

void Server::HandleClient(int clientSocket, int clientId)
{
    char buffer[BUFFER_SIZE] = {0};

    string clientName = "CLIENTE_" + to_string(clientId);
    User newUser = User(clientId, clientSocket, clientName);

    {
        lock_guard<mutex> lock(threadPoolMutex);
        users.push_back(newUser);
    }

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            clientDisconnect(newUser);
            break;
        }
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);

        cout << formatMessage(buffer, ctime(&now_c), newUser) << endl;

        if (isAnyCommand(buffer))
        {
            ExecuteCommand(buffer, newUser);
        }
        else
        {
            SendMessagesToAllClients(newUser, buffer, ctime(&now_c));
        }
    }

    close(clientSocket);
    {
        unique_lock<mutex> lock(threadPoolMutex);
        cv.notify_one();
    }
}

void Server::clientDisconnect(User user)
{
    printServerMessage("Cliente " + user.getName() + " desconectou", yellow);

    if (user.getId() == 1)
    {
        nextClientId.store(1);
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        for (auto it = users.begin(); it != users.end(); ++it)
        {
            if (it->getId() == user.getId())
            {
                users.erase(it);
                break; // Assuming each client has a unique ID, you can exit the loop once found
            }
        }
    }
}

void Server::StartListening()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    printServerMessage("Servidor escutando na porta " + to_string(port), green);

    while (true)
    {
        int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (newSocket == -1)
        {
            printServerError("Erro ao aceitar a conexão");
        }

        int clientId = nextClientId.fetch_add(1);

        printServerMessage("Nova conexão de " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string(ntohs(clientAddr.sin_port)) + " (CLIENTE_" + to_string(clientId) + ")", green);

        unique_lock<mutex> lock(threadPoolMutex);
        cv.wait(lock, [this]
                { return threadPool.size() < MAX_CLIENTS; });

        threadPool.emplace_back(&Server::HandleClient, this, newSocket, clientId);
    }
}

void Server::SendMessagesToAllClients(User hostUser, char *buffer, char *time)
{
    // a mensagem é [CLIENTE X]: *mensagem*
    string formattedMessage = formatMessage(buffer, time, hostUser);

    for (User user : users)
    {
        // Não enviar a mensagem de clientes mutados
        // TODO: arrumar /mute, o /muteall funciona
        if (/*user.isMuted(hostUser.getId()) ||*/ ADMINisMuted(user))
        {
            continue;
        }

        this->SendSingleMessage(formattedMessage, user);
    }
}

void Server::SendSingleMessage(const string &message, User user)
{
    string formattedMessage = message + "\n";
    send(user.getClientSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
}