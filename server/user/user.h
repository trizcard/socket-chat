#ifndef __USER_H__
#define __USER_H__

#include <string>
#include <set>

class User {
public:
    User(int id, int socket, std::string name);
    User(int id, std::string name);
    User(int id);
    ~User();

    int getId();
    std::string getName();
    int getClientSocket();

    void setId(int id);
    void setName(std::string name);
    void setClientSocket(int clientSocket);

    void muteUser(int id);
    void unmuteUser(int id);
    bool isMuted(int id);
private:
    int id;;
    std::string name;
    int clientSocket;
    std::set<int> muteList;
};

#endif // USER_H