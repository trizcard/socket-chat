#include "server.h"
#include <iostream>

/**
 * Verifica se o usuário é válido
 * 
 * @param searchedUser Usuário buscado
 * @param searchedUsername Nome do usuário buscado
 * @param clientUser Usuário que está fazendo a busca
 * @return true Se o usuário é válido
*/
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


/**
 * Verifica se houve uma entrada
 * 
 * @param users Usuários buscados
 * @param expectedUsers Quantidade de usuários esperados
 * @param sender Usuário que está fazendo a busca
 * @return true Se houve uma entrada
*/
bool Server::mustHaveUserInput(vector<string> users, size_t expectedUsers, User* sender)
{
    if (users.size() != expectedUsers)
    {
        this->SendSingleMessage(colorString("Necessário digitar o username do usuário alvo, use o comando /help para saber mais", red), *sender);
        return false;
    }

    return true;
}

/**
 * Verifica se o username já está em uso
 * 
 * @param username Nome do usuário
 * @return true Se o username já está em uso
*/
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

/**
 * Pega o usuário pelo id
 * 
 * @param id Id do usuário
 * @return Usuário
*/
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

/**
 * Pega o usuário pelo nome
 * 
 * @param name Nome do usuário
 * @return Usuário
*/
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

/**
 * Verifica se o admin está mutado
 * 
 * @param user Usuário
 * @return true Se o admin está mutado
*/
bool Server::ADMINisMuted(User user)
{
    return (generalMuteList.find(user.getId()) != generalMuteList.end());
}