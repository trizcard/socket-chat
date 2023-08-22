#define __SLASHFUNCTIONS_H__
#ifdef __SLASHFUNCTIONS_H__

#include <string>
#include <sstream>
#include <vector>

bool isCommand(const std::string &message, const std::string &command);

std::vector<std::string> extractUsernames(const std::string &message);

// mute user by id, add to mute list

#endif // __SLASHFUNCTIONS_H__