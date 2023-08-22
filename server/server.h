#ifndef SERVER_H
#define SERVER_H

#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

private:
    int serverSocket;
    int port;

    // Pool de threads e mutex
    std::vector<std::thread> threadPool;
    std::mutex threadPoolMutex;
    std::condition_variable cv;

    // Variável para manter o número do próximo cliente
    std::atomic<int> nextClientId;

    // Função para a thread do cliente
    void HandleClient(int clientSocket, int clientId);
};

#endif // SERVER_H
