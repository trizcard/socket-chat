#include <string>
#include <set>
#include <sstream>
#include <vector>
#include <iostream>

#include "slashFunctions.h"

using namespace std;

bool isAnyCommand(const string &message)
{
    return !message.empty() && message[0] == '/';
}

bool isCommand(const string &message, const string &command)
{
    if (message.size() < command.size())
    {
        return false; // Message is too short to match the command
    }

    for (size_t i = 0; i < command.size(); ++i)
    {
        if (message[i] != command[i])
        {
            return false; // Characters don't match, not a command
        }
    }

    // Check if the command is followed by a space or the end of the string
    if (message.size() > command.size() && message[command.size()] != ' ')
    {
        return false;
    }

    return true;
}

vector<string> extractUsernames(const string &message)
{
    vector<string> usernames;
    istringstream iss(message);
    string token;

    // Skip the command itself
    iss >> token;

    // Extract usernames
    while (iss >> token)
    {
        usernames.push_back(token);
    }

    return usernames;
}

vector<User>::iterator getUserFromUsername(const string &username, vector<User> &users)
{
    for (auto it = users.begin(); it != users.end(); ++it)
    {
        if (it->getName() == username)
        {
            return it; // Return iterator to the found User object
        }
    }

    return users.end(); // Return iterator to the end if user not found
}

string colorString(const string &message, const string &color)
{
    return color + message + RESET;
}

string formatMessage (const string &message, const string &time, User hostUser)
{
    string formattedName = "[" + hostUser.getName() + "]    ";
    return (
        colorString(formattedName, blue) +
        colorString(time, gray) +
        colorString(message, lightGray));
}

void printServerMessage (const string &message, const string &color)
{
    cout << colorString(message, color) << endl;
}

void printServerError (const string &message)
{
    cerr << colorString(message, red) << endl;
}