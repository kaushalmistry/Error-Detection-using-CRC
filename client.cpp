// Assignment 1 : Socket Programming 
// Client utility

#include "crc.h"

int openSocket;

class Client {
    // Utility members of client class
    int sock, clientFd, port;
    sockaddr_in clientAddr;
    char* ip;
    char buff[4096];
    CRCUtil* crc;
    string message;
    timeval tv;

    public:

    // Constuctor
    Client(char* ip, int port, float ber) {
        this->port = port;
        this->ip = ip;
        this->crc = new CRCUtil();
        this->crc->BER = ber;
        this->tv.tv_sec = 10;
        this->tv.tv_usec = 0;

        // Initializing the socket
        this->sock = socket(AF_INET, SOCK_STREAM, 0);

        if (this->sock < 0) {
            cerr << "=> Error while creating the socket!" << endl;
            exit(EXIT_FAILURE);
        }

        setsockopt(this->sock, 1, SO_RCVTIMEO, (const char*) &(this->tv), sizeof this->tv);
    }

    // Connecting to the server
    int connectToServer() {
        clientAddr.sin_port = htons(port);
        clientAddr.sin_family = AF_INET;
        if (inet_pton(AF_INET, ip, &clientAddr.sin_addr) <= 0) {
            cerr << "=> Invalid address. The address not supported" << endl;
            return -1;
        }
        
        clientFd = connect(sock, (sockaddr*)&clientAddr, sizeof(clientAddr));

        openSocket = clientFd;

        if (clientFd < 0) {
            cerr << "=> Connection to the provided IP and port failed!" << endl;
            return -1;
        }

        cout << "\n\n----------------------- Connected to the Server! -----------------------" << endl;

        return 0;
    }

    // transmits the message after adding random error with the probability provided by the user 
    void transmitMsg() {
        // Display the transmitted original message
        cout << "=> The encrypted message transmitted: " << message << endl << endl;

        string s = crc->errorGenerator(message);
        // Clearing the buffer
        memset(buff, 0, 4096);
        strcpy(buff, s.c_str());

        if (send(sock, buff, strlen(buff), 0) == -1)
            cout << "\n------------------  Server disconnected! ------------------" << endl;
    }

    void sendMsg() {

        // Reading the input from the user
        crc->getInput();

        // Encrypting the message using the CRC method and using 8 bit polynomial
        message = crc->getEncryptedMsg();
        
        // Sending the encrypted message to the server
        transmitMsg();
    }

    int receiveMsg() {
        memset(buff, 0, 4096);
        int byteRecv = -1;

        while ((byteRecv = recv(sock, buff, 4096, 0)) < 0 || byteRecv >= 0) {
            // if (buff.size() == 0) return 1;
            if (byteRecv < 0) {
                cout << "\n=> Timed out while waiting for response from Server!" << endl;
                return 1;
            } else
                break;
        }

        if (byteRecv == -1) {
            cerr << "=> There was a connection issue." << endl;
            return -1;
        }
        if (byteRecv == 0) {
            cerr << "\n------------------  Server disconnected! ------------------" << endl;
            return -1;
        }

        // true - no error
        // false - error
        bool e = crc->getDecryptedMsg(buff);

        if (e) { // No error
            if (buff[0] == '1') {
                cout << "=> Received \"ACKNOWLEDGEMENT\" from the server!" << endl;
                return 0; // no error
            }
            cout << "=> Received \"NEGATIVE ACKNOWLEDGEMENT\" from the server!" << endl;
            return 1; // error
        }
        return 1; // no error
        
    }

    // Destructor 
    ~Client() {
        close(clientFd);
    }

};

/**
 * Interrupt handler
*/
void handle_sigint(int sig)
{
    cout << "\n----------------------------------------------";
    cout << "\n[Interrupt handler] Closing open sockets!";
    cout << "\n----------------------------------------------\n";
    if (openSocket > 0) {
        close(openSocket);
    }
    sleep(1);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {

    if (argc < 3) {
        cout << "Please provide the IP and port number to connect to Server." << endl;
        return 0;
    }

    int port;
    sscanf(argv[2], "%d", &port);
    float ber;

    cout << "\n=> Enter the Bit Error Rate (0 - 100): ";
    cin >> ber;

    Client* client = new Client(argv[1], port, ber / 100);

    if (client->connectToServer() == -1) {
        return -1;
    }

        srand(time(0));


    while (1) {
        signal(SIGINT, handle_sigint);
        client->sendMsg();
        int rec = client->receiveMsg();
        if (rec == -1) return -1;

        // Tracks the number of retransmissions
        int noOfRetransmits = 0;

        // Re-transmission logic
        while (rec == 1 && noOfRetransmits < 10) {
            cout << "\n\n\n=> Retrasmitting the message to Server!\n\n\n";
            client->transmitMsg();
            rec = client->receiveMsg();
            noOfRetransmits++;
        }

        if (noOfRetransmits >= 10) {
            cout << "\n\n=> Stopped retransmitting after 10 tries.\n" << endl;
        }
    }

    return 0;
}