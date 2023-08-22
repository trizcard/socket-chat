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

void Server::HandleClient(int clientSocket, int clientId) {
    char buffer[BUFFER_SIZE] = {0};

    string clientName = "CLIENTE_" + to_string(clientId);
    User newUser = User(clientId, clientSocket, clientName);

    {
        lock_guard<mutex> lock(threadPoolMutex);
        users.push_back(newUser);
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            clientDisconnect(newUser);
            break;
        }

        // TODO: mudar a mensagem, criar uma função bonitinha pra isso que envia o user e o buffer
        cout << "[" << clientName << "] " << buffer << endl;

        if (isAnyCommand(buffer))
        {
            ExecuteCommand(buffer, newUser);
        } else {
            SendMessagesToAllClients(newUser, buffer);
        }

        // const char* response = "Mensagem recebida pelo servidor";
        // send(clientSocket, response, strlen(response), 0);
    }

    close(clientSocket);
    {
        unique_lock<mutex> lock(threadPoolMutex);
        cv.notify_one();
    }
}

void Server::clientDisconnect(User user) {
    cerr << user.getName() << " desconectou" << endl;

    if (user.getId() == 1)
    {
        nextClientId.store(1);
    }

    {
        std::lock_guard<std::mutex> lock(threadPoolMutex);
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

void Server::SendMessagesToAllClients(User hostUser, char *buffer)
{
    // a mensagem é [CLIENTE X]: *mensagem*
    string formattedMessage = "[" + hostUser.getName() + "]: " + buffer;

    for (User user : users)
    {
        // Não enviar a mensagem para o próprio cliente
        if (user.getId() == hostUser.getId())
        {
            continue;
        }

        // Não enviar a mensagem para clientes mutados
        if (user.isMuted(hostUser.getId()) || generalMuteList.find(user.getId()) != generalMuteList.end())
        {
            continue;
        }

        send(user.getClientSocket(), formattedMessage.c_str(), formattedMessage.length(), 0);
    }
}

void Server::ExecuteCommand(string message, User clientUser)
{
    if (isCommand(message, "/mute"))
    {
        cout << "Mute command" << endl;
        vector<string> usernames = extractUsernames(message);
        cout << "Usernames extracted" << usernames.at(0) << endl;

        bool found = false;
        string username = usernames.at(0);

        for (User user : users)
        {
            if (user.getName() == username)
            {
                clientUser.muteUser(user.getId());
                found = true;
                break;
            }
        }

        if (!found)
        {
            // TODO: send error message
            cout << "User " << username << " not found" << endl;
        }
    }
    else if (isCommand(message, "/unmute"))
    {
        std::vector<std::string> usernames = extractUsernames(message);
        for (std::string username : usernames)
        {
            for (User user : users)
            {
                if (user.getName() == username)
                {
                    clientUser.unmuteUser(user.getId());
                    break;
                }
            }
        }
    }
    else if (isCommand(message, "/muteall"))
    {
        std::vector<std::string> usernames = extractUsernames(message);
        for (std::string username : usernames)
        {
            for (User user : users)
            {
                if (user.getName() == username)
                {
                    ADMINmuteUser(user.getId());
                    break;
                }
            }
        }
    }
    else if (isCommand(message, "/unmuteall"))
    {
        std::vector<std::string> usernames = extractUsernames(message);
        for (std::string username : usernames)
        {
            for (User user : users)
            {
                if (user.getName() == username)
                {
                    ADMINunmuteUser(user.getId());
                    break;
                }
            }
        }
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