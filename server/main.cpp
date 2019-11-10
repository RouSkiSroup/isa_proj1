#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

using namespace std;

//enum pro zadaný příkaz v argumentu
enum commandType {boards, board_add, board_delete, board_list, item_add, item_delete, item_update, unknown};
enum method {POST, DELETE, GET, PUT, unknownMethod};

//struktura pro nastenku a jeji prispevky
struct boardStruct {
    string          name;           //name
    vector<string>  posts;          //posts
};

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



string getFirstLine(string text){
    return text.substr(0, text.find("\r\n"));
}

method getMethod(string line){
    string str_method = line.substr(0, line.find(" "));
    cout << "str " << str_method << endl;
    if (str_method == "POST"){
        cout << "POST" << endl;
        return POST;
    }
    if (str_method  == "DELETE"){
        cout << "DELETE" << endl;
        return DELETE;
    }
    if (str_method == "GET"){
        cout << "GET" << endl;
        return GET;
    }
    if (str_method == "PUT"){
        cout << "PUT" << endl;
        return PUT;
    }
   return unknownMethod;
}

commandType getCommand(string text){
    string line = getFirstLine(text);
    if(line.length() < 11){
        cerr << "Bad request." << endl;
    }
    method command_method = getMethod(line);
    if (command_method == PUT){
        return item_update;
    }
    cout << "get command unknown" << endl;
    return unknown;
}

string unknownCommand(){
    string header;
    header = string("HTTP/1.1 404 Not Found");
    return header;
}

string cmdItemUpdate(){
    string header;
    header=string("PUT /board/");
    return header;
}

string createResponse(commandType command){
    string hello;
    switch(command){
        case boards:
//            hello = cmdBoards(arguments);
            break;
        case board_add:
//            hello = cmdBoardAdd(arguments);
            break;
        case board_delete:
//            hello = cmdBoardDelete(arguments);
            break;
        case board_list:
//            hello = cmdBoardList(arguments);
            break;
        case item_add:
//            hello = cmdItemAdd(arguments);
            break;
        case item_delete:
//            hello = cmdItemDelete(arguments);
            break;
        case item_update:
            hello = cmdItemUpdate();
            break;
        case unknown:
            hello = unknownCommand();
            break;
    };
    return hello;
}

void printBoard(boardStruct board){
    cout << "[" << board.name << "]" << endl;
    int i = 1;
    for (vector<string>::iterator it = board.posts.begin(); it != board.posts.end(); ++it){
        cout << i << ". " << *it << endl;
        i++;
    }
    cout << endl;
}

void printBoardsList(vector<boardStruct> board_list){
    for (vector<boardStruct>::iterator it = board_list.begin(); it != board_list.end(); ++it)
        printBoard(*it);
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

    vector<boardStruct> boards_list;

    boardStruct nastenka_test;
    nastenka_test.name = "Nastenka 1";
    nastenka_test.posts.push_back("prvni prispevek");
    nastenka_test.posts.push_back("druhy prispevek");
    boards_list.push_back(nastenka_test);

    boardStruct nastenka_test1;
    nastenka_test1.name = "Nastenka 2";
    nastenka_test1.posts.push_back("prvni prispevek");
    nastenka_test1.posts.push_back("druhy prispevek");
    boards_list.push_back(nastenka_test1);

    printBoardsList(boards_list);

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

        commandType command = getCommand(buffer);
        hello = createResponse(command);

        write(new_socket, hello.c_str(), hello.length());
        cout << "======================================Hello sent======================" << endl;
        close(new_socket);


    }
    return 0;
}