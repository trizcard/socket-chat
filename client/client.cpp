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

/**
 * Construtor da classe Client
 * 
 * @param serverIP IP do servidor
 * @param port porta do servidor
*/
Client::Client(const char *serverIP, int port) : serverIP(serverIP), port(port)
{

    struct sockaddr_in serverAddr; // estrutura que armazena informações do servidor
    isConnected.store(false);     // inicializa a variável de controle de conexão

    // cria o socket e verifica se ocorreu algum erro
    clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (clientSocket == -1)
    {
        cerr << red << "Erro ao criar o socket" << RESET << endl;
    }

    // configura a estrutura do servidor
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    // conecta ao servidor e verifica se ocorreu algum erro
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        cerr << red << "Erro ao conectar ao servidor" << RESET << endl;
        return;
    }

    isConnected.store(true);
    cout << green << "Conectado ao servidor" << RESET << endl;
}

/**
 * Destrutor da classe Client
*/
Client::~Client()
{
    close(clientSocket); 
}


/**
 * Retorna o socket do cliente
 * 
 * @return socket do cliente
*/
int Client::getClientSocket()
{
    return clientSocket; // retorna o socket
}


/**
 * Altera o valor da variável de controle de conexão
 * 
 * @param connected novo valor da variável de controle de conexão
*/
void Client::setConnected(bool connected)
{
    isConnected.store(connected);
}


/**
 * Retorna o valor da variável de controle de conexão
 * 
 * @return valor da variável de controle de conexão
*/
bool Client::getConnected()
{
    return isConnected.load();
}

/**
 * Thread que fica escutando o servidor
 * 
 * @param client ponteiro para o objeto Client
*/
void ListenThread(Client *client)
{
    char buffer[BUFFER_SIZE] = {0}; // buffer para armazenar as mensagens recebidas

    while (client->getConnected())
    {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(client->getClientSocket(), buffer, sizeof(buffer), 0);

        // verifica se ocorreu algum erro ou se a conexão foi encerrada
        if (bytesReceived <= 0)
        {
            client->setConnected(false);
            cerr << red << "Conexão encerrada pelo servidor" << RESET << endl;
            break;
        }

        cout << "\n" << buffer << endl;
    }
}

/**
 * Thread que fica enviando mensagens para o servidor
 * 
 * @param client ponteiro para o objeto Client
*/
void SendThread(Client *client)
{
    while (client->getConnected())
    {
        string message;
        getline(cin, message); // lê a mensagem digitada pelo usuário

        // apaga a linha recem digitada
        cout << "\x1b[1F"
             << "\x1b[2K";
        
        send(client->getClientSocket(), message.c_str(), message.length(), 0); // envia a mensagem
    }
}

/**
 * Função principal
*/
int main()
{
    char IP[16];
    cout << "Digite o IP do servidor: ";
    cin >> IP; // lê o IP do servidor
    const char *SERVER_IP = IP;
    const int PORT = 8080;

    Client client(SERVER_IP, PORT); // cria o objeto Client

    // declara e inicia as threads
    thread listenThread(ListenThread, &client);
    thread sendThread(SendThread, &client);

    listenThread.join();
    sendThread.join();

    return 0;
}