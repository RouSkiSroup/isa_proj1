//HTTP nástenka - client
//Autor: Filip Jerabek xjerab24
//1.11.2019

//#include <ctype.h>

//#include <unistd.h>


//#include <netinet/tcp.h>
//#include <netinet/ip.h>
//#include <netinet/ip6.h>
//#include <netinet/udp.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <netinet/ip_icmp.h>
////na moji adresu
//#include <ifaddrs.h>
//#include <arpa/inet.h>
////na loopback flag
//#include <sys/ioctl.h>
//#include <net/if.h>
////memset
//#include <cstring>
////timestamp
//#include<ctime>

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


//enum pro zadaný příkaz v argumentu
enum commandType {boards, board_add, board_delete, board_list, item_add, item_delete, item_update};

////enum pro stav portu
//enum State {open, closed, filtered};

//struktura pro nacteni argumentu
struct myArgs {
    string      host;           //host
    bool        host_set;
    long        port;           //port
    bool        port_set;
    commandType command;        //typ příkazu zadaný uživatelem
    string      name;           //parametr <name> v argumentu programu
    string      content;        //parametr <content> v argumentu programu
    long        id;             //parametr <id> v argumentu programu

//	bool 	pu_set; 		//zadano pu?
//	bool 	pt_set;			//zadano pt?
//	bool 	interface_set;	//zadano interface?
//	string	interface_ip;	//ip adresa meho interface (src)
//	string 	interface_name;	//nazev interface
//	string 	pu;				//retezcova podova portu pro udp testovani
//	string 	pt;				//retezcova podova portu pro tcp testovani
//	char   *ip;				//ip/nazev hosta na kterem budou testovany porty
};


////struktura pro nacteni/prevedeni moji adresy
//struct addressStruct {
//	bool 	ipv6;			//pouziva zadany host ipv6?
//	string 	address_read;	//ip adresa hosta
//};
//
////struktura pomahajici vypoctu kontrolniho souctu hlavicky
////pri tvorbe teto struktury a kodu, ktery tuto strukturu pouziva jsem se inpiroval kodem dostupnem na:
////https://www.binarytides.com/raw-sockets-c-code-linux/?fbclid=IwAR1vl3jtsRcT3_vAVQANP6IRIh2L3XexYcF3Dk87Nfs2aJqCc6iRbLZur9Y
////Autor: Silver Moon
////https://www.binarytides.com/author/admin/
//struct pseudo_header
//{
//	u_int32_t source_address;
//	u_int32_t dest_address;
//	u_int8_t placeholder;
//	u_int8_t protocol;
//	u_int16_t tcp_length;
//};

////fce pro kontrolni soucet hlavicky
////pri tvorbe teto funkce a kodu, ktery tuto funkci pouziva jsem se inpiroval kodem dostupnem na:
////https://www.binarytides.com/raw-sockets-c-code-linux/?fbclid=IwAR1vl3jtsRcT3_vAVQANP6IRIh2L3XexYcF3Dk87Nfs2aJqCc6iRbLZur9Y
////Autor: Silver Moon
////https://www.binarytides.com/author/admin/
//unsigned short csumTcpPack(unsigned short *ptr, int nbytes) {
//	register long sum;
//	unsigned short oddbyte;
//	register short answer;
//
//	sum = 0;
//	while (nbytes > 1) {
//		sum += *ptr++;
//		nbytes -= 2;
//	}
//	if (nbytes == 1) {
//		oddbyte = 0;
//		*((u_char*)&oddbyte) = *(u_char*)ptr;
//		sum += oddbyte;
//	}
//
//	sum = (sum >> 16) + (sum & 0xffff);
//	sum = sum + (sum >> 16);
//	answer = (short)~sum;
//
//	return (answer);
//}

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

//fce kontrolujici pocet zadanych argumentu
void checkArgCount(int argc, char *argv[]) {
    if ((argc < 5) || (argc > 9)) {
        cerr << "Spatny pocet argumentu." << endl;
        exit(1);
    }
}

//fce nacitajici zadany command a jeho parametry
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
    cerr << "Spatne zadane argumenty." << endl;
    exit(1);
}

//fce nacitajici argumenty
//args - struktura pro nactene argumenty
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
			cerr << "Zadej hodnotu k prepinaci." << endl;
			exit(1);
			break;
		case '?':
			cerr << "Neznamy prepinac " << optopt << "." << endl;
			exit(1);
			break;
		}
	}
	if (!((args->host_set) && (args->port_set))) {
		cerr << "Argumenty -H a -p jsou povinne." << endl;
		exit(1);
	}
    loadCommand(argc,argv,args);
}



////fce vracejici typ Pu/Pt
////var - nacteny retezec zadanych portu z agrumentu
//PuPtType getPuPtType(string var) {
//	size_t pos;
//	if ((pos = var.find('-')) != string::npos) {
//		if ((pos == 0) || (pos == var.length() - 1)) {
//			cout << "Zadano neplatne cislo." << endl;
//			exit(1);
//		}
//		return range;
//	}
//	if ((pos = var.find(',')) != string::npos) {
//		return list;
//	}
//	return value;
//}

////fce prevadejici PuPt do vektoru
////vect - vektor pro ulozeni prevedenych portu
////var - retezcova reprezentace zadanych portu
//void convertPuPt(vector<long> *vect, string var) {
//	size_t pos;
//	string number_str;
//	long number;
//
//	PuPtType type = getPuPtType(var);
//	switch (type) {
//	case range :
//		long start;
//		long end;
//		pos = var.find('-');
//		start = getLong(var.substr(0, pos));
//		end = getLong(var.substr(pos + 1, string::npos));
//		if (end < start) {
//			cerr << "Spatne zadany rozsah." << endl;
//			exit(1);
//		}
//		for (long i = start; i <= end; i++) {
//			vect->push_back(i);
//		}
//		break;
//	case list :
//		while ((pos = var.find(',')) != string::npos) {
//			number_str = var.substr(0, pos);
//			var.erase(0, pos + 1);
//			number = getLong(number_str);
//			vect->push_back(number);
//		}
//		number = getLong(var);
//		vect->push_back(number);
//		break;
//	case value :
//		number = getLong(var);
//		vect->push_back(number);
//		break;
//	}
//
//}

////fce pro nalezeni ip adresy zadaneho hosta
////host - host pro nalezeni ip
//addressStruct getAddress(char *host) {
//	addrinfo *adinfo;
//	addrinfo hints = {0, };
//	int err;
//	sockaddr address;
//
//	addressStruct returnAddr;
//	returnAddr.ipv6 = false;
//	struct sockaddr_in *addr;
//
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_socktype = SOCK_RAW;
//
//	err = getaddrinfo(host, NULL, &hints, &adinfo);
//
//	if (err != 0) {
//		cout << "Chyba nacteni ip adresy." << endl;
//		exit(1);
//	}
//	//kontroluji, zda mam hleda tpv4 nebo ipv6 adresu
//	if (adinfo->ai_family == AF_INET) {
//		addr = (struct sockaddr_in *)adinfo->ai_addr;
//		returnAddr.address_read = inet_ntoa((struct in_addr)addr->sin_addr);
//	}
//	else if (adinfo->ai_family == AF_INET6) {
//		returnAddr.ipv6 = true;
//		char buff[INET6_ADDRSTRLEN + 1];
//		returnAddr.address_read = inet_ntop(AF_INET6, &(((sockaddr_in6 *)adinfo->ai_addr)->sin6_addr), buff, sizeof(buff));
//	}
//	else {
//		cout << "Chyba nacteni ip adresy." << endl;
//		exit(1);
//	}
//	freeaddrinfo(adinfo);
//	return returnAddr;
//}

//fce pro vypsani vectoru
//v - vypisovany vektor
void printVect(vector<long> v) {
    for (vector<long>::iterator it = v.begin(); it != v.end(); ++it)
        cout << ' ' << *it;
    cout << '\n';
}

//debugovaci funkce pro vypis argumentu
void printArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i)
        cout << "Argument " << i << " ==== " << argv[i] << endl;
    cout << '\n';
}

//debugovaci funkce pro vypis struktury nactenych argumentu
void printArgsStruct(myArgs arguments) {
    cout << "host: " << arguments.host << endl;
    cout << "port: " << arguments.port << endl;
    cout << "command: " << arguments.command << endl;
    cout << "name: " << arguments.name << endl;
    cout << "content: " << arguments.content << endl;
    cout << "id: " << arguments.id << endl;
}

////funkce pro nalezeni adresy meho interface
////pokud je interface zadany v argumentu, pak se pokusi nalezt jeho ip
////pokud neni interface zadan, pokusi se nalezt libovolny interface ivp4, ktery neni loopback
////arguments - struktura pro ulozeni nazvu a ip interface
//void getMyIp(myArgs &arguments, addressStruct address) {
//	struct ifaddrs *ifaddr;
//	struct sockaddr_in6 *sAddr6;
//	struct sockaddr_in *sAddr;
//	char *addr;
//	if (address.ipv6) { //hledam ipv6
//		getifaddrs (&ifaddr);
//		if (arguments.interface_set) { //mam zadany nazev ipv6 interface, hledam k nemu ip
//			ifaddrs *tmp = ifaddr;
//			while (tmp) {
//				if (tmp->ifa_addr->sa_family == AF_INET6){
//					//if (!(tmp->ifa_flags & IFF_LOOPBACK)) {
//						sAddr6 = (struct sockaddr_in6 *) tmp->ifa_addr;
//						if (strcmp(arguments.interface_name.c_str(), tmp->ifa_name) == 0) {
//							char buff[INET6_ADDRSTRLEN + 1];
//							arguments.interface_ip = inet_ntop(AF_INET6, &sAddr6->sin6_addr, buff, sizeof(buff));
//							return;
//						}
//					}
//				tmp = tmp->ifa_next;
//			}
//		}
//		else { // hledam jakykoliv ipv6 interface ktery neni loopback
//			ifaddrs *tmp = ifaddr;
//			while (tmp) {
//				if (tmp->ifa_addr->sa_family == AF_INET6)
//					if (!(tmp->ifa_flags & IFF_LOOPBACK)) {
//						sAddr6 = (struct sockaddr_in6 *) tmp->ifa_addr;
//						char buff[INET6_ADDRSTRLEN + 1];
//						arguments.interface_ip = inet_ntop(AF_INET6, &sAddr6->sin6_addr, buff, sizeof(buff));
//						arguments.interface_name = tmp->ifa_name;
//						return;
//					}
//				tmp = tmp->ifa_next;
//			}
//		}
//		freeifaddrs(ifaddr);
//	}
//	else { // hledam ipv4
//		getifaddrs (&ifaddr);
//		if (arguments.interface_set) { //mam zadany nazev ipv4 interface, hledam k nemu ip
//			ifaddrs *tmp = ifaddr;
//			while (tmp) {
//				if (tmp->ifa_addr->sa_family == AF_INET){
//					//if (!(tmp->ifa_flags & IFF_LOOPBACK)) {
//						sAddr = (struct sockaddr_in *) tmp->ifa_addr;
//						if (strcmp(arguments.interface_name.c_str(), tmp->ifa_name) == 0) {
//							arguments.interface_ip = inet_ntoa(sAddr->sin_addr);
//							return;
//						}
//					}
//				tmp = tmp->ifa_next;
//			}
//		}
//		else {
//			ifaddrs *tmp = ifaddr;
//			while (tmp) { // hledam jakykoliv ipv4 interface ktery neni loopback
//				if (tmp->ifa_addr->sa_family == AF_INET)
//					if (!(tmp->ifa_flags & IFF_LOOPBACK)) {
//						sAddr = (struct sockaddr_in *) tmp->ifa_addr;
//						arguments.interface_ip = inet_ntoa(sAddr->sin_addr);
//						arguments.interface_name = tmp->ifa_name;
//						return;
//					}
//				tmp = tmp->ifa_next;
//			}
//		}
//		freeifaddrs(ifaddr);
//	}
//	cout << "Rozhrani " << arguments.interface_name << " nebylo nalezeno.!" << endl;
//	exit(1);
//}



////funkce pro naplneni ipv4 hlavicky pri tcp testovani
////iph - hlavicka
////arguments - struktura s zpracovanymi vstupnimi argumenty
////address - struktura s adresou hosta
////port - testovany port prijemce
////datagram - pole pro tcp a ip hlavicku
////sin - struktura s daty o prijemci pro fci sendto
////my_port - port odesilatele
//void prepareTcpIpv4Header(iphdr *iph, myArgs arguments, addressStruct address, \
//                          int port, char *datagram, sockaddr_in *sin, int my_port) {
//
//	//pomocna struktura s adresou prijemce
//	sin->sin_family = AF_INET;
//	sin->sin_port = htons(port);
//	sin->sin_addr.s_addr = inet_addr (address.address_read.c_str());
//
//	//naplneni IP hlavicky
//	iph->ihl = 5;
//	iph->version = 4;
//	iph->tos = 0;
//	iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr);
//	iph->id = htonl (rand());
//	iph->frag_off = 0;
//	iph->ttl = 255;
//	iph->protocol = IPPROTO_TCP;
//	iph->check = 0;
//	iph->saddr = inet_addr(arguments.interface_ip.c_str());
//	iph->daddr = sin->sin_addr.s_addr;
//	iph->check = csumTcpPack((unsigned short *)datagram, iph->tot_len);
//}

////funkce pro naplneni ipv4 hlavicky pri udp testovani
////iph - hlavicka
////arguments - struktura s zpracovanymi vstupnimi argumenty
////address - struktura s adresou hosta
////port - testovany port prijemce
////datagram - pole pro tcp a ip hlavicku
////sin - struktura s daty o prijemci pro fci sendto
////my_port - port odesilatele
//void prepareUdpIpv4Header(iphdr *iph, myArgs arguments, addressStruct address, \
//                          int port, char *datagram, sockaddr_in *sin, int my_port) {
//
//	////pomocna struktura s adresou prijemce
//	sin->sin_family = AF_INET;
//	sin->sin_port = htons(port);
//	sin->sin_addr.s_addr = inet_addr (address.address_read.c_str());
//
//
//	//naplneni IP hlavicky
//	iph->ihl = 5;
//	iph->version = 4;
//	iph->tos = 0;
//	iph->tot_len = sizeof(struct iphdr) + sizeof(struct udphdr);
//	iph->id = htonl (rand());
//	iph->frag_off = 0;
//	iph->ttl = 255;
//	iph->protocol = IPPROTO_UDP;
//	iph->check = 0;
//	iph->saddr = inet_addr(arguments.interface_ip.c_str());
//	iph->daddr = sin->sin_addr.s_addr;
//	iph->check = csumTcpPack((unsigned short *)datagram, iph->tot_len);
//}

////funkce pro naplneni tcpheaderu
////tcph - hlavicka
////port - testovany port prijemce
////my_port - port odesilatele
////arguments - struktura s zpracovanymi vstupnimi argumenty
////sin - struktura s daty o prijemci pro fci sendto
////psh - pomocna struktura ro vypocet kontrolniho souctu
//void prepareTcpHeader(tcphdr *tcph, int port, int my_port, myArgs arguments, sockaddr_in sin, pseudo_header *psh) {
//	//naplneni TCP hlavicky
//	tcph->source = htons(my_port);
//	tcph->dest = htons(port);
//	tcph->seq = 0;
//	tcph->ack_seq = 0;
//	tcph->doff = 5;
//	tcph->fin = 0;
//	tcph->syn = 1;
//	tcph->rst = 0;
//	tcph->ack = 0;
//	tcph->urg = 0;
//	tcph->window = htons(5840);
//	tcph->check = 0;
//	tcph->urg_ptr = 0;
//
//	//naplneni pomocne struktury pro kontrolni soucet
//	psh->source_address = inet_addr(arguments.interface_ip.c_str());
//	psh->dest_address = sin.sin_addr.s_addr;
//	psh->placeholder = 0;
//	psh->protocol = IPPROTO_TCP;
//	psh->tcp_length = htons(sizeof(struct tcphdr));
//}

////fce pro zachyceni paketu pri testovani stavu portu pomoci tcp
////s - soket
////souceIp - moje ip adresa (odesilatele)
////destIp - ip adresa prijemce
////muj - muj port (odesilatele)
////cizi - port prijemce
//State catchTpcPacket(int s, string sourceIp, string destIp, int muj, int cizi) {
//	char datagram[256];
//	memset (datagram, 0, 256);
//
//	//nastaveni timeoutu socketu
//	//pri tvorbe teto casti kodu jsem se inspiroval kodem dostupnym na:
//	//https://stackoverflow.com/questions/393276/socket-with-recv-timeout-what-is-wrong-with-this-code
//	//Autor otazky: chessweb
//	//Autor odpovedi: Ignas2526
//	//Profil: https://stackoverflow.com/users/1515360/ignas2526
//	const struct timeval sock_timeout = {.tv_sec = 2, .tv_usec = 0};
//	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&sock_timeout, sizeof(sock_timeout));
//
//	//pokud nic neobdrzim do 2 sekund, vracim filtered
//	if ( recv(s, datagram , 256 , 0) == -1)
//	{
//		return filtered;
//	}
//	struct iphdr *iph = (struct iphdr *) datagram;
//	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct iphdr));
//	if ((iph->saddr == inet_addr(destIp.c_str())) && (tcph->source == htons(cizi)) && \
//	        (iph->daddr == inet_addr(sourceIp.c_str())) && (tcph->dest == htons(muj))) {
//		if (tcph->ack) {
//			if (tcph->syn) {
//				return open;
//			}
//		}
//		//pokud obdrzim a neni nastaven flag ack a syn vracim closed
//		return closed;
//	}
//	//pokud obrzim, ale nesouhlasi nikdy ip a porty, vracim filtered
//	return filtered;
//}

////fce pro zjisteni stavu portu pomoci tcp
////arguments - struktura dat nactena z argumentu programu
////address - struktura obsahujici moji adresu (odesilatele)
////port - testovany port prijemce
//State tcpPacket(myArgs arguments, addressStruct address, int port) {
//	int my_port = 4321;
//
//	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
//	if (s == -1) {
//		cerr << "Chyba vytvoreni socketu.";
//		exit(1);
//	}
//
//	char datagram[4096];
//	memset (datagram, 0, 4096);
//
//
//	struct sockaddr_in sin;
//	struct pseudo_header psh;
//	struct iphdr *iph = (struct iphdr *) datagram;
//
//	//ip hlavicka
//	if (address.ipv6) {
//		cerr << "Naprogramovat spravne odeslani ipv6 se mi nepovedlo. :(" << endl;
//		exit(1);
//	}
//	else {
//		prepareTcpIpv4Header(iph, arguments, address, port, datagram, &sin, my_port);
//	}
//
//	//tcp hlavicka
//	struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct iphdr));
//	prepareTcpHeader(tcph, port, my_port, arguments, sin, &psh);
//	//kontrolni soucet tcp
//	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr);
//	char *pseudogram = (char *)malloc(psize);
//	memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
//	memcpy(pseudogram + sizeof(struct pseudo_header) , tcph , sizeof(struct tcphdr));
//	tcph->check = csumTcpPack((unsigned short *)pseudogram , psize);
//
//	int one = 1;
//	const int *val = &one;
//	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
//	{
//		cerr << "Chyna seckopt." << endl;
//		exit(1);
//	}
//	if (sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//	{
//		cerr << "Chyba sendto." << endl;
//		exit(1);
//	}
//	//nastaveni limitu, po ktery pojede while s odchytavanim paketu
//	time_t cas = time(0);
//	time_t cas_3 = cas + 2;
//	State st;
//	//cout << "==============================Chytani paketu===================================="<< endl;
//	while (cas < cas_3) {
//		cas = time(0);
//		st = catchTpcPacket(s, arguments.interface_ip, address.address_read, my_port, port);
//		if (st != filtered) {
//			return st;
//		}
//	}
//	//druhe volani pro overeni filtrovaneho portu
//	//neni jiste, jestli se paket jen neztratil
//	if (sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//	{
//		cerr << "Chyba sendto." << endl;
//		exit(1);
//	}
//	cas = time(0);
//	cas_3 = cas + 1;
//	while (cas < cas_3) {
//		cas = time(0);
//		st = catchTpcPacket(s, arguments.interface_ip, address.address_read, my_port, port);
//		if (st != filtered) {
//			return st;
//		}
//	}
//	return filtered;
//
//}

////fce pro zjisteni stavu portu ve vektoru pomoci tcp
////v - vektor s porty
////arguments - struktura dat nactena z argumentu programu
////address - struktura obsahujici moji adresu (odesilatele)
//void testTcp(vector<long> v, myArgs arguments, addressStruct address) {
//	State vysl;
//	for (vector<long>::iterator it = v.begin(); it != v.end(); ++it) {
//		vysl = tcpPacket(arguments, address, *it);
//		if (vysl == closed) {
//			cout << *it << "/tcp\tclosed" << endl;
//		}
//		if (vysl == open) {
//			cout << *it << "/tcp\topen" << endl;
//		}
//		if (vysl == filtered) {
//			cout << *it << "/tcp\tfiltered" << endl;
//		}
//	}
//}



////fce pro zachyceni paketu pri testovani stavu portu pomoci udp
////souceIp - moje ip adresa (odesilatele)
////destIp - ip adresa prijemce
////muj - muj port (odesilatele)
////cizi - port prijemce
//State catchUdpPacket(string sourceIp, string destIp, int muj, int cizi) {
//	char datagram[256];
//	memset (datagram, 0, 256);
//	struct iphdr *iph = (struct iphdr *) datagram;
//	int s = socket (PF_INET, SOCK_RAW, IPPROTO_ICMP);
//	if (s == -1) {
//		cerr << "Chyba vytvoreni socketu.";
//		exit(1);
//	}
//
//	//nastaveni timeoutu socketu
//	//pri tvorbe teto casti kodu jsem se inspiroval kodem dostupnym na:
//	//https://stackoverflow.com/questions/393276/socket-with-recv-timeout-what-is-wrong-with-this-code
//	//Autor otazky: chessweb
//	//Autor odpovedi: Ignas2526
//	//Profil: https://stackoverflow.com/users/1515360/ignas2526
//	const struct timeval sock_timeout = {.tv_sec = 2, .tv_usec = 0};
//	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&sock_timeout, sizeof(sock_timeout));
//
//	if ( recv(s, datagram , 256 , 0) == -1)
//	{
//		//timeout recv v pripade neobrzeneho icmp
//		//vracim open
//		return open;
//	}
//	icmphdr *icmph = (icmphdr *)(datagram + sizeof (struct iphdr));
//	udphdr *udph = (udphdr *)(datagram + sizeof (struct iphdr) + sizeof (struct icmphdr) + sizeof (struct iphdr));
//	if (iph->protocol == 1) { //icmp
//		if ((iph->saddr == inet_addr(destIp.c_str())) && (udph->uh_sport == htons(muj)) && \
//		        (iph->daddr == inet_addr(sourceIp.c_str())) && (udph->uh_dport == htons(cizi))) {
//			if ((icmph->code == 3) && (icmph->type == 3)) {
//				return closed;
//			}
//		}
//	}
//	return filtered;
//}

////fce pro zjisteni stavu portu pomoci udp
////arguments - struktura dat nactena z argumentu programu
////address - struktura obsahujici moji adresu (odesilatele)
////port - testovany port prijemce
//State udpPacket(myArgs arguments, addressStruct address, int port) {
//	int my_port = 4321;
//
//	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
//	if (s == -1) {
//		cerr << "Chyba vytvoreni socketu.";
//		exit(1);
//	}
//
//	char datagram[4096], *pseudogram;
//	memset (datagram, 0, 4096);
//
//
//	struct sockaddr_in sin;
//
//	//ip hlavicka
//	struct iphdr *iph = (struct iphdr *) datagram;
//
//	if (address.ipv6) {
//		cerr << "Odeslat ipv6 paket se mi bohuzel nepovedlo." << endl;
//		exit(1);
//	}
//	else {
//		prepareUdpIpv4Header(iph, arguments, address, port, datagram, &sin, my_port);
//	}
//
//	//udp hlavicka
//	struct udphdr *udph = (struct udphdr *) (datagram + sizeof (struct iphdr));
//
//	udph->uh_dport = htons(port);
//	udph->uh_sport = htons(my_port);
//	udph->uh_sum = 0;
//	udph->uh_ulen = htons(8);
//
//	int one = 1;
//	const int *val = &one;
//	if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
//	{
//		cerr << "Chyba seckopt." << endl;
//		exit(1);
//	}
//
//	if (sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//	{
//		cerr << "Chyba sendto." << endl;
//		exit(1);
//	}
//	//nastaveni casoveho limitu pro odchytavani
//	time_t cas = time(0);
//	time_t cas_3 = cas + 3;
//	State st;
//	while (cas < cas_3) {
//		cas = time(0);
//		st = catchUdpPacket(arguments.interface_ip, address.address_read, my_port, port);
//		if (st != filtered) {
//			return st;
//		}
//	}
//
//	//druhe odeslani pro kontrolu
//	if (sendto (s, datagram, iph->tot_len ,	0, (struct sockaddr *) &sin, sizeof (sin)) < 0)
//	{
//		cerr << "Chyba sendto." << endl;
//		exit(1);
//	}
//	cas = time(0);
//	cas_3 = cas + 3;
//	while (cas < cas_3) {
//		cas = time(0);
//		st = catchUdpPacket(arguments.interface_ip, address.address_read, my_port, port);
//		if (st != filtered) {
//			return st;
//		}
//	}
//	return open;
//}

////fce pro zjisteni stavu portu ve vektoru pomoci udp
////v - vektor s porty
////arguments - struktura dat nactena z argumentu programu
////address - struktura obsahujici moji adresu (odesilatele)
//void testUdp(vector<long> v, myArgs arguments, addressStruct address) {
//	State vysl;
//	for (vector<long>::iterator it = v.begin(); it != v.end(); ++it) {
//		vysl = udpPacket(arguments, address, *it);
//		if (vysl == closed) {
//			cout << *it << "/udp\tclosed" << endl;
//		}
//		if (vysl == open) {
//			cout << *it << "/udp\topen" << endl;
//		}
//		if (vysl == filtered) {
//			cout << *it << "/udp\tfiltered" << endl;
//		}
//	}
//}

string cmdBoards(myArgs arguments){
    string header;
    header="GET /boards HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

string cmdBoardAdd(myArgs arguments){
    string header;
    header="POST /boards/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

string cmdBoardDelete(myArgs arguments){
    string header;
    header="DELETE /boards/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

string cmdBoardList(myArgs arguments){
    string header;
    header="GET /board/" + arguments.name + " HTTP/1.1\r\n";
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

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

string cmdItemDelete(myArgs arguments){
    string header;
    header=string("DELETE /board/") + arguments.name + string("/") + to_string(arguments.id) + string(" HTTP/1.1\r\n");
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port));
    return header;
}

string cmdItemUpdate(myArgs arguments){
    string header;
    header=string("PUT /board/") + arguments.name + string("/") + to_string(arguments.id) + string(" HTTP/1.1\r\n");
    header.append(string("Host: ") + arguments.host + string(":") + to_string(arguments.port)  + string("\r\n"));
    header.append("Content-Type: text/plain\r\n");
    header.append("Content-Length: " + to_string(arguments.content.length()));
    header.append("\r\n\r\n");

    return header;
}

int main(int argc, char *argv[])
{
    //nastaveni vstupnich argumentu pro usnadneni debugovani
    // argc = 8;
    // argv[1] = (char *)"-pt";
    // argv[2] = (char *)"1";
    // argv[6] = (char *)"-pu";
    // argv[7] = (char *)"20,25,68,631";
    // //argv[4] = (char *)"19,53,67,68,111,123,135,137,138,139,161,162,389,445,514,520,623,2049,3702";
    // //argv[5] = (char *)"wis.fit.vutbr.cz";
    // //argv[5] = (char *)"2600:3c01::f03c:91ff:fe98:ff4e";
    // argv[5] = (char *)"localhost";
    // //argv[5] = (char *)"10.2.1.1";
    // argv[3] = (char *)"-i";
    // argv[4] = (char *)"lo";

    myArgs arguments;
//	vector<long> pu;
//	vector<long> pt;
//	addressStruct address;
    printArgs(argc,argv);
    cout << "argc: " << argc << endl << endl;
    loadArgs(argc, argv, &arguments);
    printArgsStruct(arguments);

    int sock = 0;
    long valread;
    struct sockaddr_in serv_addr;
    string hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        cerr << "Chyba vytvoreni socketu" << endl;
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(arguments.port);

    //ipv4 a ipv6 do binarni podoby
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0){
        cerr << "Neplatna adresa";
        return -1;
    }

    if(connect(sock,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        cerr << "connect failed";
        return -1;
    }

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


    send(sock, hello.c_str(), hello.length(), 0);
    cout << "Hello message sent" << endl;
    valread = read (sock, buffer, 1024);
    cout << buffer << endl;
    return 0;


//	convertPuPt(&pu, arguments.pu);
//	convertPuPt(&pt, arguments.pt);

//	address = getAddress(arguments.ip);
//	getMyIp(arguments, address);

    // cout << "===================================DEBUG===============================" << endl;
    // cout << "host:\t\t " << arguments.ip << endl;
    // cout << "ip adresa hosta\t\t " << address.address_read << endl;
    // cout << "src:\t\t\t " << arguments.interface_ip << endl;
    // cout << "interface:\t\t " << arguments.interface_name << endl;
    // cout << "-----pu porty -----" << endl;
    // printVect(pu);
    // cout << "-----pt porty -----" << endl;
    // printVect(pt);
    // cout << "-------------------" << endl;

//	cout << "Scanning ports on " << arguments.ip << " (" << address.address_read << "):" << endl;
//
//	if (arguments.pt_set) {
//		//cout << "tcp skenovani= ===============================" << endl;
//		testTcp(pt, arguments, address);
//	}
//	//arguments.interface_ip  = (char *)"127.0.0.1";
//	if (arguments.pu_set) {
//		//cout << "udp skenovani= ===============================" << endl;
//		testUdp(pu, arguments, address);
//	}
    cout << "jsem na konci" << endl;

    return 0;
}



