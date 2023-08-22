#ifndef __SERVER_H__
#define __SERVER_H__

#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "../user/user.h"
#include "../slashFunctions/slashFunctions.h"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

typedef struct muteData {
    int mutedById;
    int mutedId;
} MuteData;

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

    void SendMessagesToAllClients(User user, char *buffer);
    void clientDisconnect(User user);

    void ExecuteCommand(std::string message, User user);

    void ADMINmuteUser(int id);
    void ADMINunmuteUser (int id);
private:
    int serverSocket;
    int port;

    // Pool de threads e mutex
    std::vector<std::thread> threadPool;
    std::mutex threadPoolMutex;
    std::condition_variable cv;

    std::set<int> generalMuteList;

    // Variável para manter o número do próximo cliente
    std::atomic<int> nextClientId;
    std::vector<User> users;

    // Função para a thread do cliente
    void HandleClient(int clientSocket, int clientId);
};

#endif // SERVER_H
