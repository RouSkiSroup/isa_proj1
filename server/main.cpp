#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <regex>

using namespace std;

//enum pro zadaný příkaz v argumentu
enum commandType {boards, board_add, board_delete, board_list, item_add, item_delete, item_update, unknown};
enum method {POST, DELETE, GET, PUT, unknownMethod};

//struktura pro nastenku a jeji prispevky
struct boardStruct {
    string          name;           //name
    vector<string>  posts;          //posts
};

struct commandDataStruct {
    commandType command;        //typ příkazu zadaný uživatelem
    string name;           //parametr <name> v argumentu programu
    string content;        //parametr <content> v argumentu programu
    long id;             //parametr <id> v argumentu programu
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


commandDataStruct getCommand(string text){
    string line = getFirstLine(text);
    commandDataStruct command_data;

    if(line.length() < 11){
        cerr << "Bad request." << endl;
    }

    smatch match;

    //    string what("I am 5 years old.");

    //boards - GET /boards
    regex expression(R"(GET\s(\/boards)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = boards;
        return command_data;
    }

    //board add <name> - POST /boards/<name>
    expression = (R"(POST\s(\/boards\/)([0-9a-zA-Z]+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = board_add;
        command_data.name = match.str(2);
        return command_data;
    }

    //board delete <name> - DELETE /boards/<name>
    expression = (R"(DELETE\s(\/boards\/)([0-9a-zA-Z]+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = board_delete;
        command_data.name = match.str(2);
        return command_data;
    }

    //board list <name> - GET /board/<name>
    expression = (R"(GET\s(\/board\/)([0-9a-zA-Z]+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = board_list;
        command_data.name = match.str(2);
        return command_data;
    }

    //item add <name> <content> - POST /board/<name>
    expression = (R"(POST\s(\/board\/)([0-9a-zA-Z]+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = item_add;
        command_data.name = match.str(2);
        return command_data;
    }

    //item delete <name> <id> - DELETE /board/<name>/<id>
    expression = (R"(DELETE\s(\/board\/)([0-9a-zA-Z]+)\/(\d+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = item_delete;
        command_data.name = match.str(2);
        command_data.id = getLong(match.str(3));
        return command_data;
    }

    //item update <name> <id> <content> - PUT /board/<name>/<id>
    expression = (R"(PUT\s(\/board\/)([0-9a-zA-Z]+)\/(\d+)\s(HTTP\/1.1)$)");
    if (regex_search(line, match, expression))
    {
        command_data.command = item_update;
        command_data.name = match.str(2);
        command_data.id = getLong(match.str(3));
        return command_data;
    }

    command_data.command = unknown;
    return command_data;
}

bool isBoardsEmpty(vector<boardStruct> boards_list){
    return boards_list.empty();
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

void printBoardsList(vector<boardStruct> boards_list){
    for (vector<boardStruct>::iterator it = boards_list.begin(); it != boards_list.end(); ++it)
        printBoard(*it);
}

void addBoardsToString(string *response, vector<boardStruct> boards_list){
    int i = 0;
    for (vector<boardStruct>::iterator it = boards_list.begin(); it != boards_list.end(); ++it){
        if(i != 0){
            (*response).append("\n");
        }
        i++;
        (*response).append((*it).name);
    }
}

int checkBoardNameExists(vector<boardStruct> boards_list, string name){
    int i = 0;
    for (vector<boardStruct>::iterator it = boards_list.begin(); it != boards_list.end(); ++it){
        if ((*it).name == name){
            return i;
        }
        i++;
    }
    return -1;
}

boardStruct *getBoardByName(vector<boardStruct> *boards_list, string name){
    int i = 0;
    for (vector<boardStruct>::iterator it = boards_list->begin(); it != boards_list->end(); ++it){
        if ((*it).name == name){
            return &boards_list->at(i);
        }
        i++;
    }
}

void addPostsOfBoardToString(string *response, vector<boardStruct> *boards_list, string name){
    boardStruct *board = getBoardByName(boards_list, name);
    int i = 0;
    for (vector<string>::iterator it = board->posts.begin(); it != board->posts.end(); ++it){
        if(i != 0){
            (*response).append("\n");
        }
        i++;
        (*response).append((to_string(i) + ". " + *it));
    }
}

int getContentLengthValue(string request){
    string line;
    istringstream iss(request);
    smatch match;
    regex expression(R"(^Content-Length:\s(\d+))");
    while (getline(iss, line))
    {
        if (regex_search(line, match, expression)){
            return getLong(match.str(1));
        }
    }
    return -1;
}

bool getContent(string request, string *content){
    int pos = request.find("\r\n\r\n");
    if(pos != string::npos){
        cout << pos << " tuna" << endl;
        *content = request.substr(pos+4);
        return true;
    }
    return false;
}



string cmdBoards(vector<boardStruct> *boards_list){
    string header;
//    if(isBoardsEmpty(*boards_list)){
//        header = string("HTTP/1.1 404 Not Found");
//    }
//    else{
//        header=string("HTTP/1.1 200 OK \r\n\r\n");
//        addBoardsToString(&header, *boards_list);
//    };

    string content;
    addBoardsToString(&content, *boards_list);
    header=string("HTTP/1.1 200 OK \r\n");
    header.append("Content-Type: text/plain\r\n");
    header.append("Content-Length: " + to_string(content.length()));
    header.append("\r\n\r\n");
//    addBoardsToString(&header, *boards_list);
    header.append(content);
return header;
}

string cmdBoardAdd(commandDataStruct command_data, vector<boardStruct> *boards_list){
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists >= 0){
        header = string("HTTP/1.1 409 Conflict");
    }
    else{
        boardStruct board;
        board.name = command_data.name;
        boards_list->push_back(board);
        header=string("HTTP/1.1 201 OK \r\n\r\n");
    };
    return header;
}

string cmdBoardDelete(commandDataStruct command_data, vector<boardStruct> *boards_list){
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists == -1){
        header = string("HTTP/1.1 404 Not Found");
    }
    else{
        (*boards_list).erase((*boards_list).begin()+5);
        header=string("HTTP/1.1 200 OK \r\n\r\n");
    };
    return header;
}

string cmdBoardList(commandDataStruct command_data, vector<boardStruct> *boards_list){
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists == -1){
        header = string("HTTP/1.1 404 Not Found");
    }
    else{
        string content;
        addPostsOfBoardToString(&content, boards_list, command_data.name);
        header=string("HTTP/1.1 200 OK");
        header.append("Content-Type: text/plain\r\n");
        header.append("Content-Length: " + to_string(content.length()));
        header.append("\r\n\r\n");
        header.append(content);

    };
    return header;
}

string cmdItemAdd(commandDataStruct command_data, vector<boardStruct> *boards_list, string request) {
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists == -1){
        header = string("HTTP/1.1 404 Not Found");
    }
    else{
        int content_lenght = getContentLengthValue(request);
        if (content_lenght < 1){
            header = string("HTTP/1.1 400 Bad Request");
        }
        else{
            boardStruct *board = getBoardByName(boards_list, command_data.name);
            string content;
            if(getContent(request, &content)){
              board->posts.push_back(content);
              header=string("HTTP/1.1 201 OK");
              printBoardsList(*boards_list);
            }
            else{
                header = string("HTTP/1.1 400 Bad Request");
            }
//
        }
    };
    return header;
}

string cmdItemDelete(commandDataStruct command_data, vector<boardStruct> *boards_list) {
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists == -1){
        header = string("HTTP/1.1 404 Not Found");
    }
    else{
        boardStruct *board = getBoardByName(boards_list, command_data.name);
        int size = board->posts.size();
        if (command_data.id > size){
            header = string("HTTP/1.1 400 Bad Request");
        }
        else{
            board->posts.erase(board->posts.begin()+command_data.id);
            header=string("HTTP/1.1 201 OK");
        }
//
    };
    return header;
}

string cmdItemUpdate(commandDataStruct command_data, vector<boardStruct> *boards_list, string request) {
    string header;
    int exists = checkBoardNameExists(*boards_list, command_data.name);
    if(exists == -1){
        header = string("HTTP/1.1 404 Not Found");
    }
    else{
        int content_lenght = getContentLengthValue(request);
        if (content_lenght < 1){
            header = string("HTTP/1.1 400 Bad Request");
        }
        else {
            boardStruct *board = getBoardByName(boards_list, command_data.name);
            string content;
            if(getContent(request, &content)){
                int size = board->posts.size();
                if (command_data.id > size) {
                    header = string("HTTP/1.1 400 Bad Request");
                } else {
                    board->posts.at(command_data.id-1) = content;
                    header = string("HTTP/1.1 201 OK");
                }
            }
        }
//
    };
    return header;
}



string unknownCommand(){
    string header;
    header = string("HTTP/1.1 404 Not Found");
    return header;
}



string createResponse(commandDataStruct command_data, vector<boardStruct> *boards_list, string request){
    string hello;
    switch(command_data.command){
        case boards:
            hello = cmdBoards(boards_list);
            break;
        case board_add:
            hello = cmdBoardAdd(command_data, boards_list);
            break;
        case board_delete:
            hello = cmdBoardDelete(command_data, boards_list);
            break;
        case board_list:
            hello = cmdBoardList(command_data, boards_list);
            break;
        case item_add:
            hello = cmdItemAdd(command_data, boards_list, request);
            break;
        case item_delete:
            hello = cmdItemDelete(command_data, boards_list);
            break;
        case item_update:
            hello = cmdItemUpdate(command_data, boards_list, request);
            break;
        case unknown:
            hello = unknownCommand();
            break;
    };
    return hello;
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
    nastenka_test.name = "Nastenka1";
    nastenka_test.posts.push_back("prvni prispevek");
    nastenka_test.posts.push_back("druhy prispevek");
    boards_list.push_back(nastenka_test);

    boardStruct nastenka_test1;
    nastenka_test1.name = "Nastenka2";
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

        commandDataStruct command_data = getCommand(buffer);

        cout << endl << endl << "type " << command_data.command << endl;
        cout << "name " << command_data.name << endl;
        cout << "id " << to_string(command_data.id) << endl;
        cout << "content " << command_data.content << endl;

        hello = createResponse(command_data, &boards_list, buffer);

        write(new_socket, hello.c_str(), hello.length());
        cout << "======================================Hello sent======================" << endl;

        printBoardsList(boards_list);

        close(new_socket);


    }
    return 0;
}