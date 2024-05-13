#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>

using namespace std;

class Handler {
private:
    int serverSocket;
    struct sockaddr_in server;

    int clientSocket;

public:
    Handler(int serverPort) {
        this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket < 0) {
            cout << "! Socket has been failed to create" << endl;

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

            clientSocket = accept(serverSocket, (struct sockaddr*)&client, (socklen_t*)&clientLength);
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
                sleep(1000);

                sendError = send(clientSocket, command, commandLength, 0);

                if (sendError > 0) {
                    break;
                }

                if (sendError < 0 && i == 5) {
                    close(clientSocket);
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
};

int main(int argc, char *argv[]) {
    cout << "**3chidna**" << endl;
    
    if (argv[1] != NULL) {
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
    } else {
       cout << "! Port number is requiered" << endl;
    }
}