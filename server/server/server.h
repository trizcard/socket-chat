#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <vector>
#include <set>
#include <mutex>
#include <atomic>
#include <condition_variable>

#define BUFFER_SIZE 1024;
#define MAX_CLIENTS 5;

typedef struct muteData {
    int mutedById;
    int mutedId;
} MuteData;

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

    void ADMINmuteUser (int id);
    void ADMINunmuteUser (int id);

    void muteUser (muteData muteData);
    void unmuteUser (muteData muteData);

private:
    int serverSocket;
    int port;

    // Pool de threads e mutex
    std::vector<std::thread> threadPool;
    std::mutex threadPoolMutex;
    std::condition_variable cv;

    std::set<int> generalMuteList;
    std::set<MuteData> userMuteList;

    // Variável para manter o número do próximo cliente
    std::atomic<int> nextClientId;

    // Função para a thread do cliente
    void HandleClient(int clientSocket, int clientId);
};

#endif // SERVER_H
