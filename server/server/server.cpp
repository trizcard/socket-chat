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
        exit(-1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        printServerError("Erro ao associar o socket ao endereço");
        exit(-1);
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1)
    {
        printServerError("Erro ao escutar por conexões");
        exit(-1);
    }

    // reserva espaço para o número máximo de clientes
    users.reserve(MAX_CLIENTS);
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

    User *user = getUserById(clientId);

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            clientDisconnect(*user);
            break;
        }
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);

        cout << formatMessage(buffer, ctime(&now_c), *user, blue) << endl;

        if (isAnyCommand(buffer))
        {
            ExecuteCommand(buffer, user);
        }
        else
        {
            SendTextMessageToAll(user, buffer, ctime(&now_c));
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

void Server::SendTextMessageToAll(User* hostUser, char *buffer, char *time)
{
    if (ADMINisMuted(*hostUser))
    {
        SendMessageAndPrint("Você está mutado e não pode enviar mensagens", *hostUser);
        return;
    }

    // a mensagem é [CLIENTE X]: *mensagem*
    string formattedMessage = formatMessage(buffer, time, *hostUser, blue);
    string formattedSenderMessage = formatMessage(buffer, time, *hostUser, green);

    for (User user : users)
    {
        // Se é o próprio usuário, enviar com cor diferente
        if (user.getId() == hostUser->getId()) {
            this->SendSingleMessage(formattedSenderMessage, user);
            continue;
        }

        // Não enviar a mensagem de clientes mutados para o usuário
        if (!user.isMuted(hostUser->getId()))
        {
            this->SendSingleMessage(formattedMessage, user);
        }
    }
}

void Server::SendSingleMessage(const string &message, User user)
{
    string formattedMessage = message + "\n";
    send(user.getClientSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
}

void Server::SendMessageToAll(const string &message, vector<User> exceptionUsers)
{
    for (User user : users)
    {
        bool isException = false;
        for (User exceptionUser : exceptionUsers)
        {
            if (user.getId() == exceptionUser.getId())
            {
                isException = true;
                break;
            }
        }
        if (!isException)
        {
            this->SendSingleMessage(message, user);
        }
    }
}

void Server::SendMessageAndPrint(const string &message, User user)
{
    SendSingleMessage(message, user);
    printServerMessage(message, yellow);
}