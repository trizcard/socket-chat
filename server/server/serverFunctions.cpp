#include "server.h"
#include <iostream>

void Server::ExecuteCommand(string message, User& clientUser)
{
    vector<string> usernames = extractUsernames(message);

    // comandos que não usam nome de usuario
    if (isCommand(message, "/changename"))
    {
        if (usernames.size() != 1)
        {
            this->SendSingleMessage(colorString("Necessário digitar o novo nome de usuário, use o comando /help para saber mais", red), clientUser);
            return;
        }

        string newName = usernames.at(0);
        if (newName == clientUser.getName())
        {
            this->SendSingleMessage(colorString("Nome de usuário já é " + newName, red), clientUser);
            return;
        }

        if (isUsernameTaken(newName))
        {
            this->SendSingleMessage(colorString("Nome de usuário " + newName + " já está em uso", red), clientUser);
            return;
        }

        lock_guard<mutex> lock(threadPoolMutex);

        clientUser.setName(newName);
        this->SendSingleMessage(colorString("Nome de usuário alterado para " + newName, green), clientUser);
    }
    else if (isCommand(message, "/help"))

    vector<User> users;

    // checa cada username pra ver se ele existe mesmo
    if (!usernames.empty())
    {
        for (string username : usernames)
        {
            User& searchedUser = *getUserFromUsername(username, users);
            if (!isValidUser(searchedUser, username))
            {
                return;
            }

            users.push_back(searchedUser);
        }
    }

    if (isCommand(message, "/mute") && mustHaveUserInput(users, 1, clientUser))
    {
        User searchedUser = users.at(0);

        if (clientUser.isMuted(searchedUser))
        {
            this->SendSingleMessage(colorString("Usuário " + usernames.at(0) + " já está mutado para você", red), clientUser);
            return;
        }

        clientUser.muteUser(searchedUser);
    }
    else if (isCommand(message, "/unmute") && mustHaveUserInput(users, 1, clientUser))
    {
        User searchedUser = users.at(0);

        if (!clientUser.isMuted(searchedUser))
        {
            this->SendSingleMessage(colorString("Usuário " + usernames.at(0) + " já está desmutado para você", red), clientUser);
            return;
        }

        clientUser.unmuteUser(searchedUser);
    }
    else if (isCommand(message, "/adminmute") && mustHaveUserInput(users, 1, clientUser))
    {
        User searchedUser = users.at(0);

        if (ADMINisMuted(searchedUser))
        {
            this->SendSingleMessage(colorString("Usuário " + usernames.at(0) + " já está mutado", red), clientUser);
            return;
        }

        ADMINmuteUser(searchedUser);
    }
    else if (isCommand(message, "/adminunmute") && mustHaveUserInput(users, 1, clientUser))
    {
        User searchedUser = users.at(0);

        if (!ADMINisMuted(searchedUser))
        {
            this->SendSingleMessage(colorString("Usuário " + usernames.at(0) + " já está desmutado", red), clientUser);
            return;
        }

        ADMINunmuteUser(searchedUser);
    }
    else
    {
        this->SendSingleMessage(colorString("Comando não encontrado, use o comando /help para saber mais", red), clientUser);
    }
}

void Server::ADMINmuteUser(User userToMute)
{
    generalMuteList.insert(userToMute.getId());

    string adminMessage = string(yellow) + "User " + userToMute.getName() + " has been muted by the admin.";
    for (User user : users)
    {
        this->SendSingleMessage(adminMessage, user);
    }

    printServerMessage(adminMessage, yellow);
}

void Server::ADMINunmuteUser(User userToUnmute)
{
    generalMuteList.erase(userToUnmute.getId());

    string adminMessage = string(yellow) + "User " + userToUnmute.getName() + " has been unmuted by the admin.";
    for (User user : users)
    {
        this->SendSingleMessage(adminMessage, user);
    }

    printServerMessage(adminMessage, yellow);
}

bool Server::ADMINisMuted(User user)
{
    return (generalMuteList.find(user.getId()) != generalMuteList.end());
}

bool Server::isValidUser(User user, string username)
{
    if (user.getId() == -1)
    {
        printServerError("Usuário " + username + " não encontrado");
        this->SendSingleMessage(colorString("Usuário " + username + " não encontrado", red), user);
        return false;
    }

    return true;
}

bool Server::mustHaveUserInput(vector<User> users, size_t expectedUsers, User sender)
{
    if (users.size() < expectedUsers)
    {
        this->SendSingleMessage(colorString("Necessário digitar o username do usuário alvo, use o comando /help para saber mais", red), sender);
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