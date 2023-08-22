CC = g++
CFLAGS = -std=c++11 -Wall -pthread

SERVER_SOURCES = server.cpp
CLIENT_SOURCES = client.cpp

SERVER_OBJECTS = $(SERVER_SOURCES:.cpp=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.cpp=.o)

SERVER_EXECUTABLE = server
CLIENT_EXECUTABLE = client

all: $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE)

$(SERVER_EXECUTABLE): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) $(SERVER_OBJECTS) -o $(SERVER_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) $(CLIENT_OBJECTS) -o $(CLIENT_EXECUTABLE)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SERVER_EXECUTABLE) $(CLIENT_EXECUTABLE) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)
