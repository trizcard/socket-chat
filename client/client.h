#ifndef CLIENT_H
#define CLIENT_H

class Client {
public:
    Client(const char* serverIP, int port);
    ~Client();

    void ConnectAndCommunicate();

private:
    int clientSocket;
    const char* serverIP;
    int port;
};

#endif // CLIENT_H
