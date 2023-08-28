#define __SLASHFUNCTIONS_H__
#ifdef __SLASHFUNCTIONS_H__

#include <string>
#include <sstream>
#include <vector>

#include "../user/user.h"

#define red "\033[1;31m"
#define green "\033[1;92m"
#define gray "\033[1;90m"
#define lightGray "\033[0;37m"
#define blue "\033[1;94m"
#define yellow "\033[1;93m"

#define RESET "\033[1;97m"

using namespace std;

bool isAnyCommand(const string &message);
bool isCommand(const string &message, const string &command);

vector<string> extractUsernames(const string &message);
vector<User>::iterator getUserFromUsername(const string &username, vector<User> &users);

string formatMessage(const string &message, const string &time, User hostUser);
string colorString(const string &message, const string &color);
void printServerMessage(const string &message, const string &color);
void printServerError(const string &message);

// mute user by id, add to mute list

#endif // __SLASHFUNCTIONS_H__