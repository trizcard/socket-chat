#include "user.h"
#include <string>

User::User(int id, int socket, std::string name)
{
    this->id = id;
    this->name = name;
    this->clientSocket = socket;
    this->muteList = std::set<int>();
}

User::User(int id, std::string name)
{
    this->id = id;
    this->name = name;
    this->muteList = std::set<int>();
}

User::User(int id)
{
    this->id = id;
    this->name = "";
    this->muteList = std::set<int>();
}

User::~User()
{
    this->muteList = std::set<int>();
}

int User::getId()
{
    return this->id;
}

std::string User::getName()
{
    return this->name;
}

void User::setId(int id)
{
    this->id = id;
}

void User::setName(std::string name)
{
    this->name = name;
}

int User::getClientSocket()
{
    return this->clientSocket;
}

void User::setClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

// ==============================

void User::muteUser(User user)
{
    this->muteList.insert(user.getId());
}

void User::unmuteUser(User user)
{
    this->muteList.erase(user.getId());
}

bool User::isMuted(User user)
{
    return this->muteList.find(user.getId()) != this->muteList.end();
}