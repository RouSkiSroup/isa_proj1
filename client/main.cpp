//HTTP nástěnka - client
//Autor: Filip Jerabek xjerab24
//1.11.2019

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <getopt.h>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;


//enum for command type in program arguments
enum commandType {boards, board_add, board_delete, board_list, item_add, item_delete, item_update};

//struktura for loading arguments
struct myArgs {
    string      host;           //host
    bool        host_set;       //check if host is set
    long        port;           //port
    bool        port_set;       //check if port is set
    commandType command;        //command type
    string      name;           //parameter <name> in program arguments
    string      content;        //parameter <content> in program arguments
    long        id;             //parameter <id> in program arguments
};

//function to check if -h is set
bool checkHelp(int argc, char *argv[]){
    if(argc == 2){
        if(strcmp(argv[1], "-h") == 0){
            cout << "Client part of isa http board project.\n\
Program loads command, which is set as arguments and send request to server.\n\
Run like:\n\
./isaclient -H <host> -p <port> <command>\n\
where <command> may be: \n\
\tGET /boards \t Returns aviable boards. One pre line.\n\
\tPOST /boards/name \t Creates new board with name <name>.\n\
\tDELETE /boards/name \t Deletes board with name <name> and its posts.\n\
\tGET /board/name \t Shows posts of board <name>.\n\
\tPOST /board/name \t Adds a new post to the end of board <name>. \n\
\tPUT /board/name/id \t Rewrites post <id> in board <name>.\n\
\tDELETE /board/name/id \t Deletes post <id> in board <name>.\n\
Parameters:\n\
\t -H \t  - host\n\
\t -p \t  - port [0-65535]\n\
            " << endl;
            return true;
        }
    }
    return false;
}

//loads string and returns long if possible. If not, push error
//str - string number
long getLong(string str) {
    for (long i = 0; i < str.length(); i++) {
        if (!(str[i] >= '0' && str[i] <= '9')) {
            cout << "Failed to convert " << str << " to number" << endl;
            exit(-1);
        }
        if (str.length() == 0) {
            cout << "Failed to convert " << str << " to number" << endl;
            exit(-1);
        }
    }
    long res;
    char *end;
    res = strtol(str.c_str(), &end, 10);
    if (res > 65535) { //range for port.
        cout << "Port out of range. Max 65535." << endl;
        exit(-1);
    }
    return res;
}

//chcecks arg count
void checkArgCount(int argc, char *argv[]) {
    if ((argc < 5) || (argc > 10)) {
        cerr << "Bad arg count." << endl;
        exit(1);
    }
}

//loads command and its parameters
void loadCommand(int argc, char *argv[], myArgs *args) {
    if ((argc == 6) && (strcmp(argv[5],"boards") == 0)) {
        args->command = boards;
        return;
    }
    if (argc == 8) {
        if (strcmp(argv[5],"board") == 0){
            args->name = argv[7];
            if (strcmp(argv[6],"add") == 0){
                args->command = board_add;
                return;
            }
            if (strcmp(argv[6],"delete") == 0){
                args->command = board_delete;
                return;
            }
            if (strcmp(argv[6],"list") == 0){
                args->command = board_list;
                return;
            }
        }
    }
    if (argc == 9) {
        if (strcmp(argv[5],"item") == 0){
            args->name = argv[7];
            if (strcmp(argv[6],"add") == 0){
                args->command = item_add;
                args->content = argv[8];
                return;
            }
            if (strcmp(argv[6],"delete") == 0){
                args->command = item_delete;
                args->id = getLong(argv[8]);
                return;
            }
            if (strcmp(argv[6],"update") == 0){
                args->command = item_update;
                args->id = getLong(argv[8]);
                return;
            }
        }
    }
    if (argc == 10) {
        if (strcmp(argv[5],"item") == 0){
            args->name = argv[7];
            if (strcmp(argv[6],"update") == 0){
                args->command = item_update;
                args->id = getLong(argv[8]);
                args->content = argv[9];
                return;
            }
        }
    }
    cerr << "Bad arguments." << endl;
    exit(1);
}

//loads arguments
//args - struct for args to be loaded into
void loadArgs(int argc, char *argv[], myArgs *args) {
	int opt;
	args->host_set = false;
	args->port_set = false;

	static struct option long_options[] =
	{
		{"H", required_argument, NULL, 'H'},
		{"p", required_argument, NULL, 'p'},
		{NULL, 0, NULL, 0}
	};

    checkArgCount(argc, argv);

	while ((opt = getopt_long_only(argc, argv, ":", long_options, NULL)) != -1)
	{
		switch (opt)
		{
		case 'H':
			args->host_set = true;
			args->host = optarg;
			break;
		case 'p':
			args->port_set = true;
			args->port = getLong(optarg);
			break;
		case ':':
			cerr << "missing value." << endl;
			exit(1);
			break;
		case '?':
			cerr << "unknown option " << optopt << "." << endl;
			exit(1);
			break;
		}
	}
	if (!((args->host_set) && (args->port_set))) {
		cerr << "Options -H and -p are required." << endl;
		exit(1);
	}
    loadCommand(argc,argv,args);
}

//debug function to print vector
//v - vector type long to be printed
void printVect(vector<long> v) {
    for (vector<long>::iterator it = v.begin(); it != v.end(); ++it)
        cout << ' ' << *it;
    cout << '\n';
}

//debug function for printing args
void printArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i)
        cout << "Argument " << i << " ==== " << argv[i] << endl;
    cout << '\n';
}

//debug function for printing loaded args
void printArgsStruct(myArgs arguments) {
    cout << "host: " << arguments.host << endl;
    cout << "port: " << arguments.port << endl;
    cout << "command: " << arguments.command << endl;
    cout << "name: " << arguments.name << endl;
    cout << "content: " << arguments.content << endl;
    cout << "id: " << arguments.id << endl;
}

//function loads content from response
//content - loads content in this variable
bool getContent(string response, string *content){
    int pos = response.find("\r\n\r\n");
    if(pos != string::npos){
        *content = response.substr(pos+4);
        return true;
    }
    return false;
}

//function returns headers from response
string getHeaders(string response){
    int pos = response.find("\r\n\r\n");
    if(pos != string::npos){
        return response.substr(0, pos);
    }
    return response;
}

//function to create request for arg  - boards
string cmdBoards(myArgs arguments){
    string header;
    header="GET /boards HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

//function to create request for arg  - board add
string cmdBoardAdd(myArgs arguments){
    string header;
    header="POST /boards/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

//function to create request for arg  - board delete
string cmdBoardDelete(myArgs arguments){
    string header;
    header="DELETE /boards/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

//function to create request for arg  - board list
string cmdBoardList(myArgs arguments){
    string header;
    header="GET /board/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

//function to create request for arg  - item add
string cmdItemAdd(myArgs arguments){
    string header;
    header="POST /board/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port) + string("\r\n"));
    header.append("Content-Type: text/plain\r\n");
    header.append("Content-Length: " + to_string(arguments.content.length()));
    header.append("\r\n\r\n");

    header.append(arguments.content);
    return header;
}

//function to create request for arg  - item delete
string cmdItemDelete(myArgs arguments){
    string header;
    header=string("DELETE /board/") + arguments.name + string("/") + to_string(arguments.id) + string(" HTTP/1.1\r\n");
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

//function to create request for arg  - item update
string cmdItemUpdate(myArgs arguments){
    string header;
    header=string("PUT /board/") + arguments.name + string("/") + to_string(arguments.id) + string(" HTTP/1.1\r\n");
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port)  + string("\r\n"));
    header.append("Content-Type: text/plain\r\n");
    header.append("Content-Length: " + to_string(arguments.content.length()));
    header.append("\r\n\r\n");

    header.append(arguments.content);
    return header;
}

//function to create request
string createRequest(myArgs arguments){
    string hello;
    switch(arguments.command){
        case boards:
            hello = cmdBoards(arguments);
            break;
        case board_add:
            hello = cmdBoardAdd(arguments);
            break;
        case board_delete:
            hello = cmdBoardDelete(arguments);
            break;
        case board_list:
            hello = cmdBoardList(arguments);
            break;
        case item_add:
            hello = cmdItemAdd(arguments);
            break;
        case item_delete:
            hello = cmdItemDelete(arguments);

            break;
        case item_update:
            hello = cmdItemUpdate(arguments);
            break;
    };
    return hello;
}

int main(int argc, char *argv[])
{
    if (checkHelp(argc, argv)){
        return 0;
    }
    myArgs arguments;
    loadArgs(argc, argv, &arguments);

    int sock = 0;
    long valread;
    struct sockaddr_in serv_addr;
    string request;
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr << "Failed to create socket." << endl;
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(arguments.port);

    //ipv4 a ipv6 do binarni podoby
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
        cerr << "Wrong address.";
        return -1;
    }

    if(connect(sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        cerr << "Connection failed.";
        return -1;
    }

    request = createRequest(arguments);

    send(sock, request.c_str(), request.length(), 0);
    valread = read (sock, buffer, 1024);

    cerr << getHeaders(buffer) << endl;
    string content;
    if (getContent(buffer, &content) > 0 ){
        cout << content << endl;
    }

    return 0;
}