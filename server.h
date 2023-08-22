#ifndef SERVER_H
#define SERVER_H

class Server {
public:
    Server(int port);
    ~Server();

    void StartListening();

private:
    int serverSocket;
    int newSocket;
    int port;
};

#endif // SERVER_H
