#ifndef __USER_H__
#define __USER_H__

#include <string>
#include <set>

// deve ser mudado para um arquivo .h separado que irá ficar oculto. Senha para demonstração
#define ADMIN_PASSWORD "admin"

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
    void setAdmin(bool isAdmin);
    bool getAdmin();
    void setClientSocket(int clientSocket);

    void muteUser(User user);
    void unmuteUser(User user);
    bool isMuted(User user);
private:
    int id;;
    std::string name;
    int clientSocket;
    std::set<int> muteList;
    bool isAdmin;
};

#endif // USER_H