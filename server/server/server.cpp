#include "server.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <chrono>

using namespace std;


Server::Server(int port) : port(port), nextClientId(1) {
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << red << "Erro ao criar o socket" << RESET << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << red << "Erro ao associar o socket ao endereço" << RESET << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        cerr << red << "Erro ao escutar por conexões" << RESET << endl;
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
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        cout << gray << std::ctime(&now_c);

        // TODO: mudar a mensagem, criar uma função bonitinha pra isso que envia o user e o buffer
        cout << "[" << clientName << "] " << buffer << endl;

        if (isAnyCommand(buffer))
        {
            ExecuteCommand(buffer, newUser);
        } else {
            SendMessagesToAllClients(newUser, buffer, std::ctime(&now_c));
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
            cerr << red << "Erro ao aceitar a conexão" << RESET << endl;
            continue;
        }

        cout << green << "Conexão estabelecida com o cliente" << RESET << endl;

        int clientId = nextClientId.fetch_add(1);

        unique_lock<mutex> lock(threadPoolMutex);
        cv.wait(lock, [this] { return threadPool.size() < MAX_CLIENTS; });

        threadPool.emplace_back(&Server::HandleClient, this, newSocket, clientId);
    }
}

void Server::SendMessagesToAllClients(User hostUser, char *buffer, char *time)
{
    // a mensagem é [CLIENTE X]: *mensagem*
    string formattedMessage = string(blue) + "[" + hostUser.getName() + "]    " + gray + time + lightGray + buffer + "\n" + RESET;

    for (User user : users)
    {
        // Não enviar a mensagem para o próprio cliente
        if (user.getId() == hostUser.getId())
        {
            continue;
        }

        // Não enviar a mensagem de clientes mutados
        // TODO: arrumar /mute, o /muteall funciona
        if (/*user.isMuted(hostUser.getId()) ||*/ generalMuteList.find(hostUser.getId()) != generalMuteList.end())
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
        vector<string> usernames = extractUsernames(message);

        bool found = false;
        string username = usernames.at(0);

        for (User user : users)
        {
            if (user.getName() == username)
            {
                clientUser.muteUser(user);
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
                    clientUser.unmuteUser(user);
                    break;
                }
            }
        }
    }
    else if (isCommand(message, "/adminmute"))
    {
        std::vector<std::string> usernames = extractUsernames(message);
        for (std::string username : usernames)
        {
            for (User user : users)
            {
                if (user.getName() == username)
                {
                    ADMINmuteUser(user);
                    break;
                }
            }
        }
    }
    else if (isCommand(message, "/adminunmute"))
    {
        std::vector<std::string> usernames = extractUsernames(message);
        for (std::string username : usernames)
        {
            for (User user : users)
            {
                if (user.getName() == username)
                {
                    ADMINunmuteUser(user);
                    break;
                }
            }
        }
    }
}

void Server::ADMINmuteUser(User userToMute) {
    generalMuteList.insert(userToMute.getId());

    std::string adminMessage = "User " + userToMute.getName() + " has been muted by the admin.";
    for (User user : users)
    {
        send(user.getClientSocket(), adminMessage.c_str(), adminMessage.length(), 0);
    }
}

void Server::ADMINunmuteUser(User userToUnmute)
{
    generalMuteList.erase(userToUnmute.getId());

    std::string adminMessage = "User " + userToUnmute.getName() + " has been unmuted by the admin.";
    for (User user : users)
    {
        send(user.getClientSocket(), adminMessage.c_str(), adminMessage.length(), 0);
    }
}