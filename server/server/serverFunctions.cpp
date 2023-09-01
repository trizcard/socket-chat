#include "server.h"
#include <iostream>

/**
 * Executa um comando enviado pelo cliente
 *
 * @param message Mensagem /comando enviada pelo cliente
 * @param clientUser Usuário que enviou a mensagem
 */
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
    else if (isCommand(message, "/userlist"))
    {
        sendUserList(clientUser);
        return;
    }
    else if (isCommand(message, "/setadmin"))
    {
        setAdminCommand(usernames.at(0), clientUser);
        return;
    }

    // comandos que precisam de exatamente 1 nome de usuario
    if (!mustHaveUserInput(usernames, 1, clientUser))
    {
        return;
    }

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

    // comandos para admin
    if (!isAdmin(clientUser))
    {
        return;
    }

    {
        if (isCommand(message, "/adminmute"))
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

/**
 * Comando de mutar como administrador, muta o usuário para todos no servidor
 *
 * @param username Nome do usuário
 * @param clientUser Usuário que enviou o comando
 */
void Server::ADMINmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (!isValidUser(searchedUser, username, clientUser)) // se o usuário é válido
    {
        return;
    }

    if (ADMINisMuted(*searchedUser)) // se o usuário já está mutado
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está mutado", red), *clientUser);
        return;
    }

    if (searchedUser->getId() == clientUser->getId()) // se o usuário está tentando se mutar
    {
        this->SendSingleMessage(colorString("Você não pode se mutar", red), *clientUser);
        return;
    }

    generalMuteList.insert(searchedUser->getId()); // muta o usuário

    string message = "Usuário " + searchedUser->getName() + " mutado para todos";
    SendMessageToAll(colorString(message, gray), {*searchedUser});

    message = "Você foi mutado para todos";
    SendSingleMessage(colorString(message, red), *searchedUser);
}

/**
 * Comando de desmutar como administrador, desmuta o usuário para todos no servidor
 *
 * @param username Nome do usuário
 * @param clientUser Usuário que enviou o comando
 */
void Server::ADMINunmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (!isValidUser(searchedUser, username, clientUser)) // se o usuário é válido
    {
        return;
    }

    if (!ADMINisMuted(*searchedUser)) // se o usuário já está desmutado
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está desmutado", red), *clientUser);
        return;
    }

    generalMuteList.erase(searchedUser->getId()); // desmuta o usuário

    string message = "Usuário " + searchedUser->getName() + " desmutado para todos";
    SendMessageToAll(colorString(message, gray), {*searchedUser});

    message = "Você foi desmutado para todos";
    SendSingleMessage(colorString(message, green), *searchedUser);
}

/**
 * Comando de mutar, muta o usuário para o cliente
 *
 * @param username Nome do usuário
 * @param clientUser Usuário que enviou o comando
 */
void Server::muteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (clientUser == nullptr || searchedUser == nullptr) // verifica se os dois usuários são válidos
    {
        string err_msg = "Usuário " + username + " não encontrado";
        printServerError(err_msg);
        this->SendSingleMessage(colorString(err_msg, red), *clientUser);
        return;
    }

    if (clientUser->isMuted(*searchedUser)) // se o usuário já está mutado
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está mutado para você", red), *clientUser);
        return;
    }

    if (searchedUser->getId() == clientUser->getId()) // se o usuário está tentando se mutar
    {
        this->SendSingleMessage(colorString("Você não pode se mutar", red), *clientUser);
        return;
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->muteUser(*searchedUser); // muta o usuário
    }

    string message = "Você mutou o usuário " + searchedUser->getName();
    SendSingleMessage(colorString(message, yellow), *clientUser);
}

/**
 * Comando de desmutar, desmuta o usuário para o cliente
 *
 * @param username Nome do usuário
 * @param clientUser Usuário que enviou o comando
 */
void Server::unmuteUserCommand(string username, User *clientUser)
{
    User *searchedUser = getUserByName(username);

    if (clientUser == nullptr || searchedUser == nullptr) // verifica se os dois usuários são válidos
    {
        string err_msg = "Usuário " + username + " não encontrado";
        printServerError(err_msg);
        this->SendSingleMessage(colorString(err_msg, red), *clientUser);
        return;
    }

    if (!clientUser->isMuted(*searchedUser)) // se o usuário já está desmutado
    {
        this->SendSingleMessage(colorString("Usuário " + username + " já está desmutado para você", red), *clientUser);
        return;
    }

    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->unmuteUser(*searchedUser); // desmuta o usuário
    }

    string message = "Você desmutou o usuário " + searchedUser->getName();
    SendSingleMessage(colorString(message, yellow), *clientUser);
}

/**
 * Comando de alterar o nome de usuário
 *
 * @param newName Novo nome de usuário
 * @param clientUser Usuário que enviou o comando
 */
void Server::changeNameCommand(string newName, User *clientUser)
{
    if (newName == clientUser->getName()) // se o nome de usuário é o mesmo
    {
        this->SendSingleMessage(colorString("Nome de usuário já é " + newName, red), *clientUser);
        return;
    }

    if (isUsernameTaken(newName)) // se o nome de usuário já está em uso
    {
        this->SendSingleMessage(colorString("Nome de usuário " + newName + " já está em uso", red), *clientUser);
        return;
    }

    string oldName = clientUser->getName();
    {
        lock_guard<mutex> lock(threadPoolMutex);
        clientUser->setName(newName); // altera o nome de usuário
    }

    string message = "Cliente " + oldName + " mudou seu nome para " + clientUser->getName();
    printServerMessage(message, yellow);
    SendMessageToAll(colorString(message, yellow), {});
}

/**
 * Comando de ajuda, mostra os comandos disponíveis
 * 
 * @param clientUser Usuário que enviou o comando
 */
void Server::helpCommand(User *clientUser)
{
    this->SendSingleMessage(colorString("Comandos disponíveis:", yellow),
                            *clientUser);
    this->SendSingleMessage(colorString("/changename <new_name> - Altera o nome de usuário\n"
                                        "/mute <username> - Muta um usuário\n"
                                        "/unmute <username> - Desmuta um usuário\n"
                                        "/userlist - Mostra a lista de usuários\n"
                                        "/setadmin <password> - Torna o usuário administrador\n"
                                        "/adminmute <username> - Muta um usuário para todos\n"
                                        "/adminunmute <username> - Desmuta um usuário para todos\n"
                                        "/help - Mostra os comandos disponíveis",
                                        yellow),
                            *clientUser);
}

/**
 * Envia a lista de usuários para o cliente
 * 
 * @param clientUser Usuário que enviou o comando
 */
void Server::sendUserList(User *clientUser)
{
    string message = "Usuários conectados:";
    for (auto user : users)
    {
        message += "\n" + user.getName();
    }

    this->SendSingleMessage(colorString(message, yellow), *clientUser);
}

/**
 * Seta o usuário como admin
 *
 * @param password Senha de admin
 * @param clientUser Usuário que enviou o comando
 */
void Server::setAdminCommand(string password, User *clientUser)
{
    if (clientUser->getAdmin()) // se o usuário já é admin
    {
        this->SendSingleMessage(colorString("Você já é administrador", red), *clientUser);
        return;
    }

    if (password != ADMIN_PASSWORD) // se a senha está incorreta
    {
        this->SendSingleMessage(colorString("Senha incorreta", red), *clientUser);
        return;
    }

    clientUser->setAdmin(true); // seta o usuário como admin
    this->SendSingleMessage(colorString("Você agora é administrador", green), *clientUser);
}