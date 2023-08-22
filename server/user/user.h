#define __USER_H__
#ifndef __USER_H__

#include <string>

class User {
public:
    User(int id, std::string ip, std::string name);
    User(int id, std::string ip);
    ~User();

    void HandleClient();

private:
    int id;
    std::string ip;
    std::string name;

    void changeName(std::string newName);
    void muteUser(int id);
};

#endif // USER_H