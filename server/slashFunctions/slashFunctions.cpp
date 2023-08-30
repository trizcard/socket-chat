#include <string>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>

#include "slashFunctions.h"

using namespace std;

/**
 * Verifica se a mensagem possui algum comando
 * 
 * @param message Mensagem a ser verificada
 * @return true Se a mensagem possui algum comando
*/
bool isAnyCommand(const string &message)
{
    return !message.empty() && message[0] == '/';
}

/**
 * Verifica se a mensagem é um comando
 * 
 * @param message Mensagem a ser verificada
 * @param command Comando a ser verificado
 * @return true Se a mensagem é um comando
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
 * 
 * @param message Mensagem a ser verificada
 * @return Nomes do usuário
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
 * 
 * @param username Nome do usuário
 * @param users Vetor de usuários
 * @return Iterador do usuário
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
 * 
 * @param message Mensagem a ser colorida
 * @param color Cor a ser adicionada
 * @return Mensagem colorida
*/
string colorString(const string &message, const string &color)
{
    return color + message + RESET;
}

/**
 * Formata a mensagem para ser enviada
 * 
 * @param message Mensagem a ser formatada
 * @param time Hora da mensagem
 * @param hostUser Usuário que enviou a mensagem
 * @param color Cor da mensagem
 * @return Mensagem formatada
*/
string formatMessage (const string &message, const string &time, User hostUser, const string &color)
{
    string formattedName = "[" + hostUser.getName() + "]    ";
    return (
        colorString(formattedName, color) +
        colorString(time, gray) +
        colorString(message, lightGray));
}

/**
 * Imprime uma mensagem do servidor
 * 
 * @param message Mensagem a ser impressa
 * @param color Cor da mensagem
*/
void printServerMessage (const string &message, const string &color)
{
    cout << colorString(message, color) << endl;
}

/**
 * Imprime uma mensagem de erro do servidor
 * 
 * @param message Mensagem de erro
*/
void printServerError (const string &message)
{
    cerr << colorString(message, red) << endl;
}