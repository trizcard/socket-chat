#include <string>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>

#include "slashFunctions.h"

using namespace std;

/**
 * Verifica se a mensagem possui algum comando
*/
bool isAnyCommand(const string &message)
{
    return !message.empty() && message[0] == '/';
}

/**
 * Verifica se a mensagem é um comando
*/
bool isCommand(const string &message, const string &command)
{
    if (message.size() < command.size())
    {
        return false; // Mensagem é muito pequena para ser um comando
    }

    for (size_t i = 0; i < command.size(); ++i)
    {
        if (message[i] != command[i])
        {
            return false; // Mensagem não é um comando
        }
    }

    if (message.size() > command.size() && message[command.size()] != ' ')
    {
        return false; // Mensagem não é um comando (tem caracteres diferente de espaço após o comando)
    }

    return true;
}

/**
 * Extrai o nome do usuário da mensagem
*/
vector<string> extractUsernames(const string &message)
{
    vector<string> usernames;
    istringstream iss(message);
    string token;

    iss >> token; // ignora o comando

    while (iss >> token) // extrai os nomes de usuário
    {
        usernames.push_back(token);
    }

    return usernames; // retorna os nomes de usuário
}

/**
 * Retorna o iterador do usuário a partir do nome de usuário
*/
vector<User>::iterator getUserFromUsername(const string &username, vector<User> &users)
{
    for (auto it = users.begin(); it != users.end(); ++it)
    {
        if (it->getName() == username)
        {
            return it; // Retorna o iterador encontrado
        }
    }

    return users.end(); // Retorna iterador para o final se o usuário não for encontrado
}

/**
 * Inclui cor a uma string
*/
string colorString(const string &message, const string &color)
{
    return color + message + RESET;
}

/**
 * Formata a mensagem para ser enviada
*/
string formatMessage (const string &message, const string &time, User hostUser)
{
    string formattedName = "[" + hostUser.getName() + "]    ";
    return (
        colorString(formattedName, blue) +
        colorString(time, gray) +
        colorString(message, lightGray));
}

/**
 * Imprime uma mensagem do servidor
*/
void printServerMessage (const string &message, const string &color)
{
    cout << colorString(message, color) << endl;
}

/**
 * Imprime uma mensagem de erro do servidor
*/
void printServerError (const string &message)
{
    cerr << colorString(message, red) << endl;
}