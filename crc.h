#ifndef CRC_H
#define CRC_H

#include <iostream>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <signal.h>
#include <vector>
// #include <ctime>
#include <time.h>
#define ACK "10"
#define NACK "01"

using namespace std;

class CRCUtil {

    // input: The string provided by the user
    // tMsg: The string that would be transmitted after padding
    // crcGenerator: the CRC polynomial string to generate the padding
    string input, tMsg, crcGenerator;

    // tMsgLen: The length of the input after padding
    // crcLen: the degree of crc polynomial
    int crcLen;
    public:
        float BER;

        CRCUtil();

        string xorFun(string a, string b);
        
        string moduloDivision(string divident, string divisor, int msgLen);

        string getEncryptedMsg();

        void getInput();

        void updateInput(string s);

        bool getDecryptedMsg(string recMsg);

        string errorGenerator(string msg);
};

#endif