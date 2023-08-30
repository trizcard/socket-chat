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

/**
 * Cria um servidor
 * 
 * @param port Porta do servidor
 * @param nextClientId Próximo ID de cliente
 * @return Servidor criado
*/
Server::Server(int port) : port(port), nextClientId(1)
{
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) // erro ao criar o socket
    {
        printServerError("Erro ao criar o socket");
        exit(-1);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) // erro ao associar o socket ao endereço
    {
        printServerError("Erro ao associar o socket ao endereço");
        exit(-1);
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) // erro ao escutar por conexões
    {
        printServerError("Erro ao escutar por conexões");
        exit(-1);
    }

    // reserva espaço para o número máximo de clientes
    users.reserve(MAX_CLIENTS);
}

/**
 * Destrói o servidor
*/
Server::~Server()
{
    close(serverSocket);
}

/**
 * Lida com um cliente
 * 
 * @param clientSocket Socket do cliente
 * @param clientId ID do cliente
*/
void Server::HandleClient(int clientSocket, int clientId)
{
    char buffer[BUFFER_SIZE] = {0};

    string clientName = "CLIENTE_" + to_string(clientId);
    User newUser = User(clientId, clientSocket, clientName);

    {
        lock_guard<mutex> lock(threadPoolMutex);
        users.push_back(newUser); // adiciona o usuário na lista de usuários
    }

    User *user = getUserById(clientId);

    while (true)
    {
        memset(buffer, 0, sizeof(buffer)); // limpa o buffer
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); // recebe a mensagem do cliente
        if (bytesReceived <= 0)
        {
            clientDisconnect(*user);
            break;
        }
        
        // Pega o tempo atual
        chrono::system_clock::time_point now = chrono::system_clock::now();
        time_t now_c = chrono::system_clock::to_time_t(now);

        // Imprime a mensagem no servidor
        cout << formatMessage(buffer, ctime(&now_c), *user, blue) << endl;

        if (isAnyCommand(buffer)) // se a mensagem é um comando
        {
            ExecuteCommand(buffer, user);
        }
        else
        {
            SendTextMessageToAll(user, buffer, ctime(&now_c)); // envia a mensagem para todos os clientes
        }
    }

    close(clientSocket); // fecha o socket do cliente
    {
        unique_lock<mutex> lock(threadPoolMutex);
        cv.notify_one();
    }
}

/**
 * Desconexa de um cliente
 * 
 * @param user Usuário que desconectou
*/
void Server::clientDisconnect(User user)
{
    printServerMessage("Cliente " + user.getName() + " desconectou", yellow);

    if (user.getId() == 1)
    {
        nextClientId.store(1); // reseta o ID do próximo cliente
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        for (auto it = users.begin(); it != users.end(); ++it) // percorre a lista de usuários
        {
            if (it->getId() == user.getId())
            {
                users.erase(it); // remove o usuário da lista de usuários
                break; 
            }
        }
    }
}

/**
 * Inicia o servidor e escuta por conexões
*/
void Server::StartListening()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    printServerMessage("Servidor escutando na porta " + to_string(port), green); // imprime a mensagem no servidor

    while (true)
    {
        int newSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
        if (newSocket == -1) // erro ao aceitar a conexão
        {
            printServerError("Erro ao aceitar a conexão");
        }

        int clientId = nextClientId.fetch_add(1);

        printServerMessage("Nova conexão de " + string(inet_ntoa(clientAddr.sin_addr)) + ":" + to_string(ntohs(clientAddr.sin_port)) + " (CLIENTE_" + to_string(clientId) + ")", green);

        // espera até que haja espaço na thread pool
        unique_lock<mutex> lock(threadPoolMutex);
        cv.wait(lock, [this]
                { return threadPool.size() < MAX_CLIENTS; });

        threadPool.emplace_back(&Server::HandleClient, this, newSocket, clientId);
    }
}

/**
 * Envia uma mensagem para todos os clientes
 * 
 * @param hostUser Usuário que enviou a mensagem
 * @param buffer Mensagem a ser enviada
 * @param time Tempo que a mensagem foi enviada
*/
void Server::SendTextMessageToAll(User* hostUser, char *buffer, char *time)
{
    if (ADMINisMuted(*hostUser)) // se o usuário está mutado
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

/**
 * Envia uma mensagem para um único cliente
 * 
 * @param message Mensagem a ser enviada
 * @param user Usuário que receberá a mensagem
*/
void Server::SendSingleMessage(const string &message, User user)
{
    string formattedMessage = message + "\n";
    send(user.getClientSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
}

/**
 * Envia uma mensagem para todos os clientes, exceto os usuários passados
 * 
 * @param message Mensagem a ser enviada
 * @param exceptionUsers Usuários que não receberão a mensagem
*/
void Server::SendMessageToAll(const string &message, vector<User> exceptionUsers)
{
    for (User user : users)
    {
        bool isException = false;
        for (User exceptionUser : exceptionUsers) // verifica se o usuário é uma exceção
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

    else if (isCommand(message, "/changename"))
    {
        vector<string> usernames = extractUsernames(message);
        string newName = usernames.at(0);

        {
            lock_guard<mutex> lock(threadPoolMutex);
            for (User& user : users)
            {
                if (user.getId() == clientUser.getId())
                {
                    user.setName(newName);
                    break;
                }
            }
        }

        clientUser.setName(newName);
        cout << "CLIENTE_" << clientUser.getId() << " changed name to " << clientUser.getName() << endl;
    }

}

/**
 * Envia uma mensagem e a imprime no servidor
 * 
 * @param message Mensagem a ser enviada
 * @param user Usuário que receberá a mensagem
*/
void Server::SendMessageAndPrint(const string &message, User user)
{
    SendSingleMessage(message, user);
    printServerMessage(message, yellow);
}