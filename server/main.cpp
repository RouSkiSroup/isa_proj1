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

//enum for command type in request
enum commandType {boards, board_add, board_delete, board_list, item_add, item_delete, item_update, unknown};

//struktura pro nastenku a jeji prispevky
struct boardStruct {
    string          name;           //name
    vector<string>  posts;          //posts
};

//struct for command data
struct commandDataStruct {
    commandType command;        //command type
    string name;           //parameter <name> in arguments
    string content;        //parameter <content> in arguments
    long id;             //parameter <id> in arguments
};

//loads string and returns long if possible. If not, push error
//str - string number
//if fails, returns 65535, which will thow response 404 Not found.
long getLong(string str) {
    for (long i = 0; i < str.length(); i++) {
        if (!(str[i] >= '0' && str[i] <= '9')) {
            cout << "Failed to convert " << str << " to number" << endl;
            return 65535;
        }
        if (str.length() == 0) {
            cout << "Failed to convert " << str << " to number" << endl;
            return 65535;
        }
    }
    long res;
    char *end;
    res = strtol(str.c_str(), &end, 10);
    if (res > 65535) { //max port range.
        cout << "Out of range. Max 65535." << endl;
        return 65535;
    }
    return res;
}


//returns first line of text block
string getFirstLine(string text){
    return text.substr(0, text.find("\r\n"));
}

//returns command and its data
commandDataStruct getCommand(string text){
    string line = getFirstLine(text);
    commandDataStruct command_data;

    if(line.length() < 11){
        cerr << "Bad request." << endl;
    }

    smatch match;

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

//checks if board is empty
bool isBoardsEmpty(vector<boardStruct> boards_list){
    return boards_list.empty();
}

//prints board
void printBoard(boardStruct board){
    cout << "[" << board.name << "]" << endl;
    int i = 1;
    for (vector<string>::iterator it = board.posts.begin(); it != board.posts.end(); ++it){
        cout << i << ". " << *it << endl;
        i++;
    }
    cout << endl;
}

//prints board list (all boards)
void printBoardsList(vector<boardStruct> boards_list){
    for (vector<boardStruct>::iterator it = boards_list.begin(); it != boards_list.end(); ++it)
        printBoard(*it);
}

//adds boards names to string
//response - string for the names to be added
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

//returns true if the board name exists
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

//returns board by its name
boardStruct *getBoardByName(vector<boardStruct> *boards_list, string name){
    int i = 0;
    for (vector<boardStruct>::iterator it = boards_list->begin(); it != boards_list->end(); ++it){
        if ((*it).name == name){
            return &boards_list->at(i);
        }
        i++;
    }
}

//adds all posts of one single board to string
//response - string for names to be added in
//name - name of the board
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

//returns content length value. Returns -1 if the content doesnt exist
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

//returns true if content found and loads it into string
//request - string for the content
bool getContent(string request, string *content){
    int pos = request.find("\r\n\r\n");
    if(pos != string::npos){
        *content = request.substr(pos+4);
        return true;
    }
    return false;
}


//creates response for request boards
string cmdBoards(vector<boardStruct> *boards_list){
    string header;
    string content;
    addBoardsToString(&content, *boards_list);
    header=string("HTTP/1.1 200 OK \r\n");
    header.append("Content-Type: text/plain\r\n");
    header.append("Content-Length: " + to_string(content.length()));
    header.append("\r\n\r\n");
    header.append(content);
return header;
}

//creates response for request board add
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

//creates response for request board delete
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

//creates response for request board list
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

//creates response for request item add
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

//creates response for request item delete
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
    };
    return header;
}

//creates response for request item update
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

//creates response for unknown request
string unknownCommand(){
    string header;
    header = string("HTTP/1.1 404 Not Found");
    return header;
}

//function creates response for request
//from command data in command_data
//and board structure boards_list
string createResponse(commandDataStruct command_data, vector<boardStruct> *boards_list, string request){
    string response;
    switch(command_data.command){
        case boards:
            response = cmdBoards(boards_list);
            break;
        case board_add:
            response = cmdBoardAdd(command_data, boards_list);
            break;
        case board_delete:
            response = cmdBoardDelete(command_data, boards_list);
            break;
        case board_list:
            response = cmdBoardList(command_data, boards_list);
            break;
        case item_add:
            response = cmdItemAdd(command_data, boards_list, request);
            break;
        case item_delete:
            response = cmdItemDelete(command_data, boards_list);
            break;
        case item_update:
            response = cmdItemUpdate(command_data, boards_list, request);
            break;
        case unknown:
            response = unknownCommand();
            break;
    };
    return response;
}

//function to check if -h is set
bool checkHelp(int argc, char* argv[]){
    if ((argc == 2) && (strcmp(argv[1],"-h") == 0)){
        cout << "Server which is part of isa http board project.\n\
Server waits for request from client and returns response.\n\
For more information about requests refer to client.\n\
Arguments: \n\
\t -p <port to listen on>" << endl;
        return true;
    }
    return false;
}

void checkArgCount(int argc){
    if (argc != 3){
        cerr << "Bad arg count" << endl;
        exit(-1);
    }
}

int main(int argc, char* argv[]) {
    if(checkHelp(argc,argv)){
        return 0;
    }
    checkArgCount(argc);

    if (strcmp(argv[1],"-p") != 0){
        cerr << "The program accepts only parameter -p." << endl;
    }
    long port = getLong(argv[2]);
    if(port == 65535){
        cerr << "Using port 65535" << endl;
    }

    vector<boardStruct> boards_list;

    //debug - adding boards and posts
//    boardStruct nastenka_test;
//    nastenka_test.name = "Nastenka1";
//    nastenka_test.posts.push_back("prvni prispevek");
//    nastenka_test.posts.push_back("druhy prispevek");
//    boards_list.push_back(nastenka_test);
//
//    boardStruct nastenka_test1;
//    nastenka_test1.name = "Nastenka2";
//    nastenka_test1.posts.push_back("prvni prispevek");
//    nastenka_test1.posts.push_back("druhy prispevek");
//    boards_list.push_back(nastenka_test1);

//    printBoardsList(boards_list);

    int server_fd;
    int new_socket;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    string response;

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
        cout << "===================Waiting for connection=======================" << endl;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
            cerr << "accept failed" << endl;
            return -1;
        }

        char buffer[30000] = {0};
        valread = read(new_socket, buffer, 30000);

        commandDataStruct command_data = getCommand(buffer);
        response = createResponse(command_data, &boards_list, buffer);

        write(new_socket, response.c_str(), response.length());
        cout << "====================Response sent======================" << endl;

        printBoardsList(boards_list);

        close(new_socket);


    }
    return 0;
}