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

// ==============================N

void User::muteUser(int id)
{
    this->muteList.insert(id);
}

void User::unmuteUser(int id)
{
    this->muteList.erase(id);
}

bool User::isMuted(int id)
{
    return this->muteList.find(id) != this->muteList.end();
}