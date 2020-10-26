///// SERVER PROGRAM /////
#include <iostream>
#include <fstream>      // std::ifstream
#include <string>       // getline
#include <windows.h>

#include "stereopipe.hpp"

using namespace std;
int main(int argc, const char **argv)
{
    wcout << "Creating an instance of a named pipe..." << endl;
    // Create a pipe to send data
    HANDLE pipe = CreateNamedPipe(
        StereoPipe::pipename, // name of the pipe
        PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
        PIPE_TYPE_BYTE, // send data as a byte stream
        1, // only allow 1 instance of this pipe
        0, // no outbound buffer
        0, // no inbound buffer
        0, // use default wait time
        NULL // use default security attributes
    );
    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to create outbound pipe instance.";
        // look up error code here using GetLastError()
        system("pause");
        return 1;
    }
    wcout << "Waiting for a client to connect to the pipe..." << endl;
    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);
    if (!result) {
        wcout << "Failed to make connection on named pipe." << endl;
        // look up error code here using GetLastError()
        CloseHandle(pipe); // close the pipe
        system("pause");
        return 1;
    }

    // read data from file
    ifstream infile("data.txt");
    string sLine;
    if (infile.good())
    {
        getline(infile, sLine);
        //cout << sLine << endl;
    }
    infile.close();

    //add eol to string
    sLine += '\n';
    for (int i = 0; i < 1000; i++){
        std::vector<std::string> msg_packets = StereoPipe::Support::splitPackets(sLine,StereoPipe::PACKET_SIZE);
        wcout << "Sending data to pipe..." << endl;
        for (std::vector<std::string>::iterator it = msg_packets.begin() ; it != msg_packets.end(); ++it){
            //wcout << "Number of characters in string: " << it->size() << endl;
            const char* data = it->c_str();
            //wcout << "Number of bytes to send: " << strlen(data) * sizeof(char) << endl;
            DWORD numBytesWritten = 0;
            // This call blocks until a client process reads all the data
            result = WriteFile(
                pipe, // handle to our outbound pipe
                data, // data to send
                strlen(data) * sizeof(char), // length of data to send (bytes)
                &numBytesWritten, // will store actual amount of data sent
                NULL // not using overlapped IO
            );
            if (result) {
                wcout << "Number of bytes sent: " << numBytesWritten << endl;
            } else {
                wcout << "Failed to send data." << endl;
                // look up error code here using GetLastError()
            }
        }
    }
    // Close the pipe (automatically disconnects client too)
    CloseHandle(pipe);
    wcout << "Done." << endl;
    return 0;
}