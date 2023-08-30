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

typedef struct muteData {
    int mutedById;
    int mutedId;
} MuteData;

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

    void SendSingleMessage(const string &message, User user);
    void SendMessageToAll(const string &message, vector<User> exceptionUsers);
    void SendTextMessageToAll(User *user, char *buffer, char *time);
    void SendMessageAndPrint(const string &message, User user);
    void clientDisconnect(User user);

    void ExecuteCommand(string message, User* user);

    bool isValidUser(User *searchedUser, string searchedUsername, User *clientUser);
    bool isUsernameTaken(string username);
    bool mustHaveUserInput(vector<string> users, size_t numberOfUsers, User *sender);

    bool ADMINisMuted(User user);

    void ADMINmuteUserCommand(string username, User *clientUser);
    void ADMINunmuteUserCommand(string username, User *clientUser);

    void muteUserCommand(string username, User *clientUser);
    void unmuteUserCommand(string username, User *clientUser);
    void changeNameCommand(string newName, User *clientUser);
    void helpCommand(User *clientUser);

    User *getUserById(int id);
    User *getUserByName(string name);

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
