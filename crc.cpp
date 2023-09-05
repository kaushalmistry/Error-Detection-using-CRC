// CRC utility methods

#include "crc.h"

// Constuctor for initializing the CRC polynomial
CRCUtil::CRCUtil() {
    // The Cyclic Redundancy Check Polynomial: x^8 + x^5 + x^3 + x
    this->crcGenerator = "100101010";
    this->crcLen = 9;
}

// XOR Function used while division
string CRCUtil::xorFun(string a, string b) {
    if (a[0] == '0') return a.substr(1);

    string res = "";
    for (int i = 0; i < a.size(); i++)
        res += (a[i] == b[i]) ? '0' : '1';
    return res.substr(1);
}

// Division fucntion to perform the division with the crc polynomial
string CRCUtil::moduloDivision(string divident, string divisor, int msgLen) {
    string tmpDivident = divident.substr(0, crcLen);

    int j = crcLen;

    while (j < msgLen) {
        tmpDivident = xorFun(tmpDivident, divisor);
        tmpDivident += divident[j];
        j++;
    }

    return xorFun(tmpDivident, divisor);

}

// Getting the encrypted message of the message need to be send
string CRCUtil::getEncryptedMsg() {
    tMsg = input; // The divident message

    // Appending 0's at the end for the division operation
    for (int i = 0; i < crcLen - 1; i++) tMsg += '0';


    // cout << "Input after padding 0s: " << tMsg << endl;

    int tMsgLen = tMsg.size();

    return input + moduloDivision(tMsg, crcGenerator, tMsgLen);
    
}

// Getting the input from the user to send over the server
void CRCUtil::getInput() {
    cout << "\n\n=> Enter the bits to send: ";
    cin >> input;
}

void CRCUtil::updateInput(string s) {
    this->input = s;
}

/**
 * Decrypts the message and checks whether there is error or not.
 * If error then returns false or else returns true;
*/
bool CRCUtil::getDecryptedMsg(string recMsg) {

    int recMsgLen = recMsg.size();

    if (recMsgLen == 0) {
        return false;
    }

    cout << "\n\n\n=> The received message: " << recMsg << endl;
    string rem = moduloDivision(recMsg, crcGenerator, recMsgLen);

    for (int i = 0; i < rem.size(); i++) {
        if (rem[i] == '1') {
            cout << "=> There is bit error in the received message during CRC Check at " << i << " bit!" << endl;
            return false; // error
        }
    }

    cout << "=> No bit error found in the message during CRC check!" << endl;
    return true; // no error
}


string CRCUtil::errorGenerator(string msg) {
    int i;
    cout << "--------------- BER: " << this->BER << endl;
    for (i = 0; i < msg.size(); i++) {
        float n = (float) rand() / RAND_MAX;
        if (n < (this->BER)) {
            cout << "- Random probability less than BER: " << n << endl;
            if (msg[i] == '0')
                msg[i] = '1';
            else if (msg[i] == '1')
                msg[i] = '0';
            else
                cerr << "Convert to binary was not successful!" << endl;
        }
    }

    return msg;
}
