#include "server.h"
#include <iostream>

bool Server::isValidUser(User* searchedUser, string searchedUsername, User* clientUser)
{
    if (searchedUser == nullptr || searchedUser->getId() <= 0)
    {
        string message = "Usuário " + searchedUsername + " não encontrado";
        printServerError(message);
        this->SendSingleMessage(colorString(message, red), *clientUser);
        return false;
    }

    return true;
}

bool Server::mustHaveUserInput(vector<string> users, size_t expectedUsers, User* sender)
{
    if (users.size() != expectedUsers)
    {
        this->SendSingleMessage(colorString("Necessário digitar o username do usuário alvo, use o comando /help para saber mais", red), *sender);
        return false;
    }

    return true;
}

bool Server::isUsernameTaken(string username)
{
    for (User user : users)
    {
        if (user.getName() == username)
        {
            return true;
        }
    }

    return false;
}

User *Server::getUserById(int id)
{
    for (User &user : users)
    {
        if (user.getId() == id)
        {
            return &user;
        }
    }

    return nullptr;
}

User *Server::getUserByName(string name)
{
    for (User &user : users)
    {
        if (user.getName() == name)
        {
            return &user;
        }
    }

    return nullptr;
}

bool Server::ADMINisMuted(User user)
{
    return (generalMuteList.find(user.getId()) != generalMuteList.end());
}