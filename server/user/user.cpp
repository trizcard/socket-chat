#include "user.h"
#include <string>

/**
 * Construtor da classe User
 *
 * @param id id do usuário
 * @param socket socket do usuário
 * @param name nome do usuário
*/
User::User(int id, int socket, std::string name)
{
    this->id = id;
    this->name = name;
    this->clientSocket = socket;
    this->muteList = std::set<int>();
    this->isAdmin = false;
}

/**
 * Construtor da classe User
 *
 * @param id id do usuário
 * @param name nome do usuário
*/
User::User(int id, std::string name)
{
    this->id = id;
    this->name = name;
    this->muteList = std::set<int>();
    this->isAdmin = false;
}

/**
 * Construtor da classe User
 *
 * @param id id do usuário
*/
User::User(int id)
{
    this->id = id;
    this->name = "";
    this->muteList = std::set<int>();
    this->isAdmin = false;
}

/**
 * Destrutor da classe User
*/
User::~User()
{
    this->muteList = std::set<int>();
}

/**
 * Retorna o id do usuário
 *
 * @return id do usuário
*/
int User::getId()
{
    return this->id;
}

/**
 * Retorna o nome do usuário
 *
 * @return nome do usuário
*/
std::string User::getName()
{
    return this->name;
}

/**
 * Altera o id do usuário
 *
 * @param id novo id do usuário
*/
void User::setId(int id)
{
    this->id = id;
}

/**
 * Altera o nome do usuário
 *
 * @param name novo nome do usuário
*/
void User::setName(std::string name)
{
    this->name = name;
}

/**
 * Retorna o socket do usuário
 *
 * @return socket do usuário
*/
int User::getClientSocket()
{
    return this->clientSocket;
}

/**
 * Altera o socket do usuário
 *
 * @param clientSocket novo socket do usuário
*/
void User::setClientSocket(int clientSocket)
{
    this->clientSocket = clientSocket;
}

// ==============================

/**
 * Adiciona um usuário à lista de usuários mutados
 *
 * @param user usuário a ser mutado
*/
void User::muteUser(User user)
{
    this->muteList.insert(user.getId());
}

/**
 * Remove um usuário da lista de usuários mutados
 *
 * @param user usuário a ser desmutado
*/
void User::unmuteUser(User user)
{
    this->muteList.erase(user.getId());
}

/**
 * Verifica se um usuário está mutado
 *
 * @param user usuário a ser verificado
 * @return true se o usuário estiver mutado, false caso contrário
*/
bool User::isMuted(User user)
{
    return this->muteList.find(user.getId()) != this->muteList.end();
}