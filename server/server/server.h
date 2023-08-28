#ifndef __SERVER_H__
#define __SERVER_H__

#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <fstream>

#include "../user/user.h"
#include "../slashFunctions/slashFunctions.h"

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

#define red "\033[1;31m"
#define green "\033[1;92m"
#define gray "\033[1;90m"
#define lightGray "\033[0;37m"
#define blue "\033[1;94m"
#define yellow "\033[1;93m"

#define RESET "\033[1;97m"

typedef struct muteData {
    int mutedById;
    int mutedId;
} MuteData;

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

    void SendMessagesToAllClients(User user, char *buffer, char *time);
    void clientDisconnect(User user);

    void ExecuteCommand(std::string message, User& user);

    void ADMINmuteUser(User user);
    void ADMINunmuteUser (User user);
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
