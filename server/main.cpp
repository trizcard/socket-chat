#include "server/server.h"

int main() {
    const int PORT = 8080;
    Server server(PORT);
    server.StartListening();

    return 0;
}