#include "server.h"
#include <iostream>

void Server::ExecuteCommand(string message, User *clientUser)
{
    vector<string> usernames = extractUsernames(message);

    // comandos que não usam nome de usuario
    if (isCommand(message, "/changename"))
    {
        changeNameCommand(usernames.at(0), clientUser);
        return;
    }
    else if (isCommand(message, "/help"))
    {
        helpCommand(clientUser);
        return;
    }

    // comandos que precisam de exatamente 1 nome de usuario
    if (!mustHaveUserInput(usernames, 1, clientUser))
    {
        return;
    }
    {
        if (isCommand(message, "/mute"))
        {
            muteUserCommand(usernames.at(0), clientUser);
            return;
        }

        else if (isCommand(message, "/unmute"))
        {
            unmuteUserCommand(usernames.at(0), clientUser);
            return;
        }
        else if (isCommand(message, "/adminmute"))
        {
            ADMINmuteUserCommand(usernames.at(0), clientUser);
            return;
        }
        else if (isCommand(message, "/adminunmute"))
        {
            ADMINunmuteUserCommand(usernames.at(0), clientUser);
            return;
        }
    }

    this->SendSingleMessage(colorString("Comando não encontrado, use o comando /help para saber mais", red), *clientUser);
}

void Server::ADMINmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (!isValidUser(searchedUser, username, clientUser))
    {
        return;
    }

    if (ADMINisMuted(*searchedUser))
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está mutado", red), *clientUser);
        return;
    }

    // nao pode se mutar
    if (searchedUser->getId() == clientUser->getId())
    {
        this->SendSingleMessage(colorString("Você não pode se mutar", red), *clientUser);
        return;
    }

    generalMuteList.insert(searchedUser->getId());

    string message = "Usuário " + searchedUser->getName() + " mutado para todos";
    SendMessageToAll(colorString(message, gray), {*searchedUser});

    message = "Você foi mutado para todos";
    SendSingleMessage(colorString(message, red), *searchedUser);
}

void Server::ADMINunmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (!isValidUser(searchedUser, username, clientUser))
    {
        return;
    }

    if (!ADMINisMuted(*searchedUser))
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está desmutado", red), *clientUser);
        return;
    }

    generalMuteList.erase(searchedUser->getId());

    string message = "Usuário " + searchedUser->getName() + " desmutado para todos";
    SendMessageToAll(colorString(message, gray), {*searchedUser});

    message = "Você foi desmutado para todos";
    SendSingleMessage(colorString(message, green), *searchedUser);
}

void Server::muteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (clientUser == nullptr || searchedUser == nullptr)
    {
        string err_msg = "Usuário " + username + " não encontrado";
        printServerError(err_msg);
        this->SendSingleMessage(colorString(err_msg, red), *clientUser);
        return;
    }

    if (clientUser->isMuted(*searchedUser))
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está mutado para você", red), *clientUser);
        return;
    }

    // nao pode se mutar
    if (searchedUser->getId() == clientUser->getId())
    {
        this->SendSingleMessage(colorString("Você não pode se mutar", red), *clientUser);
        return;
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->muteUser(*searchedUser);
    }

    string message = "Você mutou o usuário " + searchedUser->getName();
    SendSingleMessage(colorString(message, yellow), *clientUser);
}

void Server::unmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (clientUser == nullptr || searchedUser == nullptr)
    {
        string err_msg = "Usuário " + username + " não encontrado";
        printServerError(err_msg);
        this->SendSingleMessage(colorString(err_msg, red), *clientUser);
        return;
    }

    if (!clientUser->isMuted(*searchedUser))
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está desmutado para você", red), *clientUser);
        return;
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->unmuteUser(*searchedUser);
    }

    string message = "Você desmutou o usuário " + searchedUser->getName();
    SendSingleMessage(colorString(message, yellow), *clientUser);
}

void Server::changeNameCommand(string newName, User *clientUser)
{
    if (newName == clientUser->getName())
    {
        this->SendSingleMessage(colorString("Nome de usuário já é " + newName, red), *clientUser);
        return;
    }

    if (isUsernameTaken(newName))
    {
        this->SendSingleMessage(colorString("Nome de usuário " + newName + " já está em uso", red), *clientUser);
        return;
    }

    string oldName = clientUser->getName();
    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->setName(newName);
    }

    string message = "Cliente " + oldName + " mudou seu nome para " + clientUser->getName();
    printServerMessage(message, yellow);
    SendMessageToAll(colorString(message, yellow), {});
}

void Server::helpCommand(User *clientUser)
{
    this->SendSingleMessage(colorString("Comandos disponíveis:", yellow),
                            *clientUser);
    this->SendSingleMessage(colorString("/changename <new_name> - Altera o nome de usuário\n"
                                        "/mute <username> - Muta um usuário\n"
                                        "/unmute <username> - Desmuta um usuário\n"
                                        "/adminmute <username> - Muta um usuário para todos\n"
                                        "/adminunmute <username> - Desmuta um usuário para todos\n"
                                        "/help - Mostra os comandos disponíveis",
                                        yellow),
                            *clientUser);
}