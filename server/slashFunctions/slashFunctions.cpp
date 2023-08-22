#include <string>
#include <set>
#include <sstream>
#include <vector>
#include "slashFunctions.h"

bool isCommand(const std::string &message, const std::string &command)
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

std::vector<std::string> extractUsernames(const std::string &message)
{
    std::vector<std::string> usernames;
    std::istringstream iss(message);
    std::string token;

    // Skip the command itself
    iss >> token;

    // Extract usernames
    while (iss >> token)
    {
        usernames.push_back(token);
    }

    return usernames;
}
