#include <iostream>
#include <ws2tcpip.h> //windows sockets -- api / framework used by windows to access network sockets

//https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html --> struct sockaddr_in

#pragma comment (lib, "ws2_32.lib") // This compiler directive fixes everything I guess

int main() {

    //Initialize winsock     implement error checking later

    WSADATA wsData{};
    WORD ver = MAKEWORD(2,2); //specify what version to use in this case 2.2

    int wsOK = WSAStartup(ver, &wsData); //winsock ok

    if(wsOK != 0){
        std::cerr << "unable to initialize winsock" << std::endl; //cerr is apparantly faster than cout for displaying errors because its "unbuffered"
        return 0; //return 0 for now
    }


    //create a socket --- create endpoint at port and ip address in unix its called a file descriptor

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0); //ipv4 protocol and reliable byte stream and 0 means default protocol eg either tcp or udp

    if(listening == INVALID_SOCKET){
        std::cerr << "Unable to listen on socket" << std::endl;
        return 0; //return 0 for now
    }

    //bind ip address and port to a socket

    sockaddr_in hint{}; //sockaddr has 4 member variables ->short sin_family,u_short sin_port, struct in_addr sin_addr, char sin_zero[8]; https://docs.microsoft.com/en-us/windows/win32/winsock/sockaddr-2

    hint.sin_family = AF_INET; // ipv4
    hint.sin_port = htons(54000); //host to network short - port number
    hint.sin_addr.S_un.S_addr = INADDR_ANY; //we will listen on all available interfaces --- inet_pton is another way to do this -- more stuff https://stackoverflow.com/questions/16508685/understanding-inaddr-any-for-socket-programming
    //127.0.0.1:xxxx is the normal loopback address however we are listening to any address thats available with INADDR_ANY
    // we are binding port 54000 on any address to this socket

    bind(listening, (sockaddr*)&hint, sizeof(hint)); //bind (SOCKET , const sockaddr * addr, namelen) idk why it wants a pass by reference pointer


    //tell winsock the socket is listening

    listen(listening, SOMAXCONN);//socket we are listening on then how many connections, SOMAXCONN == 0x7FFFFFFFF

    //wait for a connection

    sockaddr_in client{};
    int clientsize = sizeof(client);

    SOCKET clientsocket = accept(listening, (sockaddr*)&client, &clientsize);//once a connection happens and is successful the returned value is the socket which the connection was made on https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-accept

    char host[NI_MAXHOST];  //client's remote name
    char service[NI_MAXSERV];   //service eg port the client is connected on

    ZeroMemory(host, NI_MAXHOST); //takes a socket and a length --- ZeroMemory literally replaces each allocated byte with 0
    ZeroMemory(service, NI_MAXSERV);// or use std::memset(host, 0, NI_MAXHOST) if not on windows or to make it portable
    //set the first NI_MAXHOST bytes of host to 0 eg overwrite the first NI_MAXHOST entries in the char array and replace it with 0

    if(getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0){ //"Use the GETNAMEINFO command to translate a socket address to a node name and service location" "his command returns a string that contains the return code, the host name, and the service"  https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-getnameinfo
        std::cout << host << " connected on port " << service << std::endl;
    }else{
        //InetNtopW(AF_INET, &client.sin_addr, host, NI_MAXHOST);// doesnt work on my pc idk
        inet_ntoa(client.sin_addr); //convert ipv4 to string

        std::cout << host << " connectd on port " << ntohs(client.sin_port) << std::endl; //ntohs == network to host short
    }

    //Close listening socket
    closesocket(listening); //guess we only get one client for now then it closes

    // while loop: accept and echo message back to client
    char buf[4096];

    while(true){
        ZeroMemory(buf, 4096);
        //wait for client to send data
        int bytesReceived = recv(clientsocket, buf, 4096, 0);//received function returns number of bytes received if its 0 then client disconnected

        if(bytesReceived == SOCKET_ERROR){
            std::cerr << "Error in recv()" << std::endl;
            break;
        }

        if(bytesReceived == 0){
            std::cout << "client disconnected " << std::endl;
            break;
        }


        //echo message back to client
        std::string rep{"lol"};
        //send(clientsocket, buf, bytesReceived + 1,0 ); //when we receive a message we dont get the terminating 0 so we add 1 to bytes received
        send(clientsocket, rep.c_str(), bytesReceived + 1,0 );
    }


    //close the socket
    closesocket(clientsocket);

    //shutdown/clean up winsock
    WSACleanup();
    return 0;
}


