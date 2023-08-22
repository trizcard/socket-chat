#define __SLASHFUNCTIONS_H__
#ifdef __SLASHFUNCTIONS_H__

#include <string>
#include <set>

bool isCommand(const std::string &message, const std::string &command);

// mute user by id, add to mute list
int ADMINmuteUser (int id, std::set<int> &muteList);

#endif // __SLASHFUNCTIONS_H__