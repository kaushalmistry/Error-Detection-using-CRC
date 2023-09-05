// Assignment 1: Socket programming 
// Group Information:
// Kaushal Mistry : 224101031
// Harsh Verma : 224101021
// Chinmay Rathod : 224101063

// Server utility program
// Header includes
#include "crc.h"
#include <thread>

// Global variable to store all open sockets that need to be closed on interrupt while server is running
vector<int> openSocket;

// Client class
class Client {
    // The private members of the Client class
    CRCUtil* crc;
    sockaddr_in clientAddr;
    int clientSocket;
    socklen_t clientSize;
    char buff[4096];

    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

public:

    // Constructor
    Client(int clientSocket, sockaddr_in clientAddr) {
        this->clientSocket = clientSocket;
        this->clientAddr = clientAddr;
        this->clientSize = sizeof(clientAddr);
        this->crc = new CRCUtil();
        srand(time(0));
        this->crc->BER = 0.05;
    }

    // Getting the information about the connection
    void connectionSet() {
        // Clear the memory
        memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        int result = getnameinfo((sockaddr*)&clientAddr, clientSize, host, NI_MAXHOST, service, NI_MAXSERV, 0);

        if (result) {
            cout << host << " connected on " << service << endl;
        } else {
            inet_ntop(AF_INET, &clientAddr.sin_addr, host, NI_MAXHOST);
            cout << host << " connected on " << ntohs(clientAddr.sin_port) << endl;
        }
    }

    // Send the messsage
    void sendMessage(string response) {
        
        memset(buff, 0, 4096);
        crc->updateInput(response);

        response = crc->getEncryptedMsg();
        string s = crc->errorGenerator(response);
        
        strcpy(buff, s.c_str());
        send(clientSocket, buff, sizeof(buff), 0);
    }

    // Receiving the message from the client
    int recvMesage() {
        //clear the buffer
        memset(buff, 0, 4096);
        int byteRecv = recv(clientSocket, buff, 4096, 0);

        if (byteRecv == -1) {
            cerr << "There was a connection issue." << endl;
            return -2;
        }
        if (byteRecv == 0) {
            cout << "\n-------------------------------------------------\n";
            cout << "\tClient disconnected!";
            cout << "\n-------------------------------------------------\n\n\n";
            return -2;
        }

        // Decrypt the message
        return crc->getDecryptedMsg(buff);

    }

    // Destructor
    ~Client() {
        // close connection
        close(clientSocket);
    }

};


// Server class
class Server {
    // Private members
    int port;
    int sock;
    sockaddr_in servAddr;

    public:
    // Server constructor
    Server(int port) {
        this->port = port;
        this->sock = socket(AF_INET, SOCK_STREAM, 0);
        openSocket.push_back(sock);

        if (sock < 0) {
            cerr << "=> Error while creating the server socket!" << endl;
            exit(EXIT_FAILURE);
        }
    }

    // Binding the server to the IP and port number
    void listenOnServer() {
        // Bind the connection
        sockaddr_in servAddr;
        servAddr.sin_port = htons(port);
        servAddr.sin_family = AF_INET;
        servAddr.sin_addr.s_addr = INADDR_ANY;

        // Binding the socket
        bind(sock, (sockaddr*)&servAddr, sizeof(servAddr));

        // Listen to the client
        if (listen(sock, 0) == -1) {
            cerr << "Unable to listen!" << endl;
            exit(EXIT_FAILURE);
        } else {
            cout << "\n-------------------------------------------------\n";
            cout << "\tServer established. Listening..." << endl;
            cout << "-------------------------------------------------\n\n";
        }
    }

    // Accepting new connection from the client
    Client* acceptConnection() {
        sockaddr_in client;
        socklen_t clientSize = sizeof(client);
        int clientSocket = accept(sock, (sockaddr*) &client, &clientSize);

        // Append the socket for error handling during interruption
        openSocket.push_back(clientSocket);
        cout << "\n-------------------------------------------------\n";
        cout << "\tClient connected on the socket: " << clientSocket << endl;
        cout << "-------------------------------------------------\n\n";

        Client* cl = new Client(clientSocket, client);

        return cl;
    }

    // Destructor
    ~Server() {
        // close the socket connection
        close(sock);
    }
};

/**
 * The function to handle the communication with each client
*/
void helperForClientCommunication(Client* client) {
    // int i = 1;
    while (1) {
        int byteRecv = client->recvMesage();

        if (byteRecv == -2) break;

        // if (i % 2 == 0) {
        if (byteRecv == 1) {
            cout << "=> Sending ACKNOWLEDGEMENT to the client.\n\n";
            client->sendMessage(ACK);
        } else {
            cout << "=> Sending NEGATIVE ACKNOWLEDGEMENT to the client.\n\n";
            client->sendMessage(NACK);
        }
        // }
        // i++;
    }
}

/**
 * Interrupt handler
*/
void handle_sigint(int sig)
{
    cout << "\n----------------------------------------------";
    cout << "\n[Interrupt handler] Closing open sockets!";
    cout << "\n----------------------------------------------\n";
    for (int i = 0; i < openSocket.size(); i++) {
        close(openSocket[i]);
    }
    sleep(1);
    exit(EXIT_FAILURE);
}

// Main function
int main(int argc, char* argv[]) {
        srand(time(0));


    // Checking if the port is passed from command line or not
    if (argc < 2) {
        cout << "=> Please provide port number to run the server on!" << endl;
        return 0;
    }
    int port;
    sscanf(argv[1], "%d", &port); 

    // Creating the server with the port provided
    Server* server = new Server(port);
    server->listenOnServer();

    // Accepting connections from the clients
    while (true) {
        signal(SIGINT, handle_sigint);
        Client* cl = server->acceptConnection();
        // helperForClientCommunication(cl);

        // Multithreading for handling multiple clients concurrently
        thread t(helperForClientCommunication, cl);
        t.detach();
    }

    return 0;
}
