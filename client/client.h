#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <atomic>

class Client {
public:
    Client(const char* serverIP, int port);
    ~Client();

    void ConnectAndCommunicate();
    int getClientSocket();
    void setConnected(bool connected);
    bool getConnected();
private:
    int clientSocket;
    const char* serverIP;
    int port;
    std::atomic<bool> isConnected;
};

#endif // CLIENT_H
