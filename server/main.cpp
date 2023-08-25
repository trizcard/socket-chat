#include "server/server.h"
#include <iostream>
#include <fstream>

int main() {
    
    const int PORT = 8080;
    Server server(PORT);
    server.StartListening();

    return 0;
}