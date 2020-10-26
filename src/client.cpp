///// CLIENT PROGRAM /////
#include <iostream>
#include <windows.h>
#include <fstream>      // std::ifstream
#include <string>       // getline

#include "stereopipe.hpp"

using namespace std;
int main(int argc, const char **argv)
{
    wcout << "Connecting to pipe..." << endl;
    // Open the named pipe
    // Most of these parameters aren't very relevant for pipes.
    HANDLE pipe = CreateFile(
        StereoPipe::pipename,
        GENERIC_READ, // only need read access
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to connect to pipe." << endl;
        // look up error code here using GetLastError()
        system("pause");
        return 1;
    }
    wcout << "Reading data from pipe..." << endl;
    std::string message;
    while(true){
        // The read operation will block until there is data to read
        char buffer[StereoPipe::PACKET_SIZE];
        DWORD numBytesRead = 0;
        BOOL result = ReadFile(
            pipe,
            buffer, // the data from the pipe will be put here
            StereoPipe::PACKET_SIZE, // number of bytes allocated
            &numBytesRead, // this will store number of bytes actually read
            NULL // not using overlapped IO
        );
        if (result) {
            //buffer[numBytesRead / sizeof(char)] = '\0'; // null terminate the string
            wcout << "Number of bytes read: " << numBytesRead << endl;
            message += buffer;
            std::size_t found = message.find('\n');
            if (found!=std::string::npos){
                cout << "full message received" << endl;

                //compare against original file
                // read data from file
                ifstream infile("data.txt");
                string sLine;
                if (infile.good())
                {
                    getline(infile, sLine);
                    //cout << sLine << endl;
                }
                infile.close();

                if (message.compare(sLine)){
                    cout << "message matches original" << endl;
                }

                //reset message
                message = "";
            }
        } else {
            wcout << "Failed to read data from the pipe." << endl;
            break;
        }
    }
    // Close our pipe handle
    CloseHandle(pipe);
    wcout << "Done." << endl;
    return 0;
}