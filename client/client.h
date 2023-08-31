#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <atomic>

#define red "\033[1;31m"
#define green "\033[1;92m"
#define gray "\033[1;90m"
#define lightGray "\033[0;37m"
#define blue "\033[1;94m"

#define RESET "\033[1;97m"

class Client {
public:
    Client(const char* serverIP, int port);
    ~Client();

    void ConnectAndCommunicate();
    int getClientSocket();
    void setConnected(bool connected);
    bool getConnected();

    void makeConnection();
    void tryToReconnect();
private:
    int clientSocket;
    const char* serverIP;
    int port;
    std::atomic<bool> isConnected;
};

#endif // CLIENT_H
