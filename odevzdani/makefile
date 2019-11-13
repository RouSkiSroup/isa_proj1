CLIENT = isaclient
SERVER = isaserver

all: $(CLIENT) $(SERVER)

$(CLIENT): $(CLIENT).cpp
	g++ $^ -o $@

$(SERVER): $(SERVER).cpp
	g++ $^ -o $@

clean:
	rm $(CLIENT) $(SERVER)

tar:
	tar cf xjerab24.tar *.cpp manual.* README makefile