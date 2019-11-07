#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

using namespace std;


//fce prevadejici retezcovou reprezentaci cisla na long
//str - retezcova reprezentace cisla
long getLong(string str) {
    for (long i = 0; i < str.length(); i++) {
        if (!(str[i] >= '0' && str[i] <= '9')) {
            cout << "zadano neplatne cislo." << endl;
            exit(1);
        }
        if (str.length() == 0) {
            cout << "zadano neplatne cislo." << endl;
            exit(1);
        }
    }
    long res;
    char *end;
    res = strtol(str.c_str(), &end, 10);
    if (res > 65535) { //nechavam pro maximalni rozsah intu.
        cout << "Zadan port mimo rozsah." << endl;
        exit(1);
    }
    return res;
}

int main(int argc, char* argv[]) {
    if ((argc == 2) && (strcmp(argv[1],"-h") == 0)){
        cout << "vypisuju help" << endl;
        return 0;
    }
    if (argc != 3){
        cerr << "Spatny pocet argumentu" << endl;
    }
    if (strcmp(argv[1],"-p") != 0){
        cerr << "argument -p je povinny" << endl;
    }
    long port = getLong(argv[2]);
    cout << "Jedu na portu: " << port << endl;

    int server_fd;
    int new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    string hello = "Hello from server";

    //vytvoreni socket file descriptoru
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        cerr << "cannot create socket" << endl;
        return -1;
    }

    /* htonl converts a long integer (e.g. address) to a network representation */
    /* htons converts a short integer (e.g. port) to a network representation */

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (bind(server_fd,(struct sockaddr *)&address, sizeof(address)) < 0){
        cerr << "bind failed" << endl;
        return -1;
    }
    if (listen(server_fd, 10) < 0) {
        cerr << "listen failed" << endl;
        return -1;
    }
    while(true){
        cout << "===================Cekam na pripojeni=======================" << endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            cerr << "accept failed" << endl;
            return -1;
        }

        char buffer[30000] = {0};
        valread = read(new_socket, buffer, 30000);
        cout << buffer << endl;
        write(new_socket, hello.c_str(), hello.length());
        cout << "======================================Hello sent======================" << endl;
        close(new_socket);


    }
    return 0;
}