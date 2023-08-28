#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

using namespace std;

const int BUFFER_SIZE = 1024;

Client::Client(const char *serverIP, int port) : serverIP(serverIP), port(port)
{

    struct sockaddr_in serverAddr;
    isConnected.store(false);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << red << "Erro ao criar o socket" << RESET << endl;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << red << "Erro ao conectar ao servidor" << RESET << endl;
        return;
    }

    isConnected.store(true);
    cout << green << "Conectado ao servidor" << RESET << endl;
}

Client::~Client()
{
    close(clientSocket);
}

int Client::getClientSocket()
{
    return clientSocket;
}

void Client::setConnected(bool connected)
{
    isConnected.store(connected);
}

bool Client::getConnected()
{
    return isConnected.load();
}

void ListenThread(Client *client)
{
    char buffer[BUFFER_SIZE] = {0};

    while (client->getConnected())
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(client->getClientSocket(), buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            client->setConnected(false);
            cerr << red << "Conexão encerrada pelo servidor" << RESET << endl;
            break;
        }

        cout << "\n" << buffer << endl;
    }
}

void SendThread(Client *client)
{
    while (client->getConnected())
    {
        string message;
        getline(cin, message);

        // apaga a linha recem digitada
        cout << "\x1b[1F"
             << "\x1b[2K";

        send(client->getClientSocket(), message.c_str(), message.length(), 0);
    }
}

int main()
{
    char IP[16];
    cout << "Digite o IP do servidor: ";
    cin >> IP;
    const char *SERVER_IP = IP;
    const int PORT = 8080;

    Client client(SERVER_IP, PORT);

    thread listenThread(ListenThread, &client);
    thread sendThread(SendThread, &client);

    listenThread.join();
    sendThread.join();

    return 0;
}