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

        {
            lock_guard<mutex> lock(threadPoolMutex);
            for (User& user : users)
            {
                if (user.getId() == clientUser.getId())
                {
                    user.setName(newName);
                    break;
                }
            }
        }

        clientUser.setName(newName);
        cout << "CLIENTE_" << clientUser.getId() << " changed name to " << clientUser.getName() << endl;
        return;
    }
    else if (isCommand(message, "/help")){
        this->SendSingleMessage(colorString("Comandos disponíveis:\n"
                                            "/changename <new_name> - Altera o nome de usuário\n"
                                            "/mute <username> - Muta um usuário\n"
                                            "/unmute <username> - Desmuta um usuário\n"
                                            "/adminmute <username> - Muta um usuário para todos\n"
                                            "/adminunmute <username> - Desmuta um usuário para todos\n"
                                            "/help - Mostra os comandos disponíveis", yellow), clientUser);
        return;
    }

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
        bool found = false;
        string username = usernames.at(0);

        {
            lock_guard<mutex> lock(threadPoolMutex);

            for (User& user : users)
            {
                if (user.getName() == username)
                {
                    User blockedUser = user;

                    for (User& user : users)
                    {
                        if (user.getId() == clientUser.getId())
                        {
                            user.muteUser(blockedUser);
                            found = true;
                            break;
                        }
                    }
                    break;
                }
            }
        }

        if (!found)
        {
            // TODO: send error message
            cout << red << "User " << username << " not found" << endl;
        }
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
        return;
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
        return;
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
        return;
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
    if (user.getId() == NULL || user.getId() == -1)
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