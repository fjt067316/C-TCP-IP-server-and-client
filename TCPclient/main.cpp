#include <iostream>
#include <string>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")//tell c++ to link this with winsock library -- we are making a 32 bit program

int main() {

    std::string ipAddress = "127.0.0.1"; //Ip of the server to connect to
    int port = 54000; //listening port on server we want to connect to

    //Initialize WinSock
    WSAData data{};
    WORD ver = MAKEWORD(2,2);

    int wsResult = WSAStartup(ver, &data); //data is 0 if there are no errors and something else if there is an error

    if(wsResult != 0){
        std::cerr << "Can't start Winsock " << wsResult << std::endl;
        return 0;
    }

    //create socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); //ipv4

    if(sock == INVALID_SOCKET){ //if socket is -1 there was an error
        std::cerr << "Can't create a socket " << WSAGetLastError << std::endl;
        WSACleanup();
        return 0;
    }


    //fill in a hint structure

        //hint structure tells winsock what server and port we want to connect to
    sockaddr_in hint{}; //"sockaddr uses the remaining 14 bytes to represent sa_data, while sockaddr_in splits the 14 bytes into sin_port, sin_addr and sin_zero respectively represent the port and IP address." https://www.tutorialfor.com/blog-225191.htm
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port); //host to network short -- converts a u_short (unsigned short) to TCP/IP network byte order which is big endian
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr); //inet_pton() magically works now but not on my original server -- inet_pton converts a c style string host address to a numeric address
        //the memory address hint.sin_addr is where the numeric ip address is stored, inet_pton overwrote it (it was a buffer anyway) and it now contains our ip in a hexadecimal format


    //connect to server
    int connResult = connect(sock, (sockaddr *) &hint, sizeof(hint));
    if(connResult == SOCKET_ERROR){
        std::cerr << "Unable to connect to server " << WSAGetLastError << std::endl;
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    //do-while loop to send and receive data
    char buf[4096];
    std::string userInput{};

    do{
        //get text from user
        std::cout << "Enter some text!! " << std::endl;
        std::getline (std::cin,userInput); //can't jsut use cin or else whenever a space is pressed it will reset

        if(userInput.size() > 0){       //make sure the user typed something
            //send text
            int sendResult = send(sock, userInput.c_str(), userInput.size()+1, 0); //need the + 1 for the null char at the end of the c-style string array
            if(sendResult != SOCKET_ERROR){
                //wait for response
                ZeroMemory(buf, 4096); //zero every byte
                int bytesReceived = recv(sock, buf, 4096, 0);
                //echo response to console
                if(bytesReceived > 0){
                    std::cout << "SERVER > " << std::string(buf, 0, bytesReceived) << std::endl; //output a string with the contents of buf from index 0 for bytesReceived characters
                }
            }
        }

    }while(userInput.size() > 0);

    //close down everything
    closesocket(sock);
    WSACleanup();


    return 0;
}
