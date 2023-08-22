#include "client.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread> // Include for threads

using namespace std;

const int BUFFER_SIZE = 1024;

Client::Client(const char *serverIP, int port) : serverIP(serverIP), port(port)
{
    struct sockaddr_in serverAddr;
    isConnected.store(false);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << "Erro ao criar o socket" << endl;
        // Pode lançar uma exceção aqui se preferir
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << "Erro ao conectar ao servidor" << endl;
    }

    isConnected.store(true);
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

    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(client->getClientSocket(), buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0)
        {
            client->setConnected(false);
            cerr << "Conexão encerrada pelo servidor" << endl;
            break;
        }

        cout << buffer << endl;
    }
}

void SendThread(Client *client)
{
    while (client->getConnected())
    {
        cout << "Digite uma mensagem para o servidor: ";
        string message;
        getline(cin, message);

        send(client->getClientSocket(), message.c_str(), message.length(), 0);
    }
}

int main()
{
    const char *SERVER_IP = "192.168.0.23";
    const int PORT = 8080;
    Client client(SERVER_IP, PORT);

    thread listenThread(ListenThread, &client);
    thread sendThread(SendThread, &client);

    listenThread.join();
    sendThread.join();

    return 0;
}