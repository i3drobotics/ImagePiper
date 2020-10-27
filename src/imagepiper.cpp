#include "imagepiper.h"

namespace ImagePiper {
    Pipe::Pipe(bool runThreaded, LPCSTR pipe_name, int packet_size): 
        pipe_name_(pipe_name), 
        packet_size_(packet_size),
        runThreaded_(runThreaded){}

    LPCSTR Pipe::getPipeName(){return pipe_name_;}

    int Pipe::getPacketSize(){return packet_size_;}

    std::vector<std::string> Pipe::splitPackets(std::string message, size_t packet_size, bool pad_packets){
        std::string srvMsg_msg = message;
        std::vector<std::string> srvMsg_msg_packets;
        //split message into packets
        int i = 0;
        while (true){
            int start_index = i*packet_size;
            if ((start_index + packet_size) < srvMsg_msg.size()){
                std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, packet_size);
                srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                i++;
            } else {
                int end_index = srvMsg_msg.size();
                int length = end_index - start_index;
                std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, length);
                if (pad_packets){
                    srvMsg_msg_packet = padString(srvMsg_msg_packet,packet_size);
                }
                srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                break;
            }
        }
        return srvMsg_msg_packets;
    }

    std::string Pipe::padString(std::string str, size_t packet_size, char padding_char){
        return str.append(std::string( (packet_size - str.size() ), padding_char));
    }

    bool Pipe::close(){
        // Close the pipe (automatically disconnects client too)
        CloseHandle(pipe_);
        return true;
    }

    Server::Server(bool runThreaded, LPCSTR pipe_name, int packet_size) : 
        Pipe(runThreaded,pipe_name,packet_size){}

    bool Server::open(){
        DWORD pipe_mode;
        if (runThreaded_){
            pipe_mode = PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED;
        } else {
            pipe_mode = PIPE_ACCESS_OUTBOUND;
        }

        // Create a pipe to send data
        pipe_ = CreateNamedPipe(
            pipe_name_, // name of the pipe
            pipe_mode, // 1-way pipe -- send only
            PIPE_TYPE_BYTE, // send data as a byte stream
            1, // only allow 1 instance of this pipe
            0, // no outbound buffer
            0, // no inbound buffer
            0, // use default wait time
            NULL // use default security attributes 
        );
        if (pipe_ == NULL || pipe_ == INVALID_HANDLE_VALUE) {
            std::cout << "Failed to create outbound pipe instance." << std::endl;
            // look up error code here using GetLastError()
            return false;
        }

        if (runThreaded_){
            // Create an event handle for each pipe instance. This
            // will be used to monitor overlapped I/O activity on each pipe
            if ((event_ = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
            {
                printf("CreateEvent() for pipe failed with error %d\n", GetLastError());
                return false;
            }
            else {
                printf("CreateEvent() for pipe is OK!\n");
            }
                
            ZeroMemory(&overlap_, sizeof(OVERLAPPED));
            overlap_.hEvent = event_;

            // Listen for client connections using ConnectNamedPipe()
            if (!ConnectNamedPipe(pipe_, &overlap_))
            {
                if (GetLastError() != ERROR_IO_PENDING)
                {
                    printf("ConnectNamedPipe() for pipe failed with error %d\n", GetLastError());
                    return false;
                }
            }
            else{
                printf("ConnectNamedPipe() for pipe is OK!\n");
            }
        } else {
            std::cout << "Waiting for a client to connect to the pipe..." << std::endl;
            // This call blocks until a client process connects to the pipe
            if (!ConnectNamedPipe(pipe_, NULL)) {
                std::cout << "Failed to make connection on named pipe." << std::endl;
                // look up error code here using GetLastError()
                return false;
            }
        }
        return true;
    }

    bool Server::send(std::string message){
        std::vector<std::string> msg_packets = ImagePiper::Pipe::splitPackets(message,packet_size_);
        for (std::vector<std::string>::iterator it = msg_packets.begin() ; it != msg_packets.end(); ++it){
            //wcout << "Number of characters in string: " << it->size() << endl;
            const char* data = it->c_str();
            //wcout << "Number of bytes to send: " << strlen(data) * sizeof(char) << endl;
            DWORD numBytesWritten = 0;
            if (runThreaded_){
                bool result = WriteFile(
                    pipe_, 
                    data, 
                    strlen(data) * sizeof(char), 
                    NULL, 
                    &overlap_
                );
                if (result){
                    printf("WriteFile() should be OK!\n");
                } else {
                    if (GetLastError() != ERROR_IO_PENDING)
                    {
                        printf("ConnectNamedPipe() for pipe failed with error %d\n", GetLastError());
                        return false;
                    } else {
                        //std::cout << "Cannot write, IO pending" << std::endl;
                        return false;
                    }
                }
            } else {
                // This call blocks until a client process reads all the data
                bool result = WriteFile( //TODO: add timeout (https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o)
                    pipe_, // handle to our outbound pipe
                    data, // data to send
                    strlen(data) * sizeof(char), // length of data to send (bytes)
                    &numBytesWritten, // will store actual amount of data sent
                    NULL // not using overlapped IO
                );
                if (result) {
                    std::cout << "Number of bytes sent: " << numBytesWritten << std::endl;
                } else {
                    std::cout << "Failed to send data." << std::endl;
                    // look up error code here using GetLastError()
                    return false;
                }
            }
        }
        return true;
    }

    void Server::test(std::string message){
        while(true){
            std::cout << "Waiting for client..." << std::endl;
            DWORD Ret;
            if ((Ret = WaitForSingleObject(event_,INFINITE)) == WAIT_FAILED)
            {
                printf("WaitForSingleObject() failed with error %d\n", GetLastError());
            }
            else {
                printf("WaitForSingleObject() is OK!\n");
                std::cout << "Client connected." << std::endl;
                break;
            }
            ResetEvent(event_);
            ZeroMemory(&overlap_, sizeof(OVERLAPPED));
            overlap_.hEvent = event_;
        }
        while(true){
            std::cout << "sending message..." << std::endl;
            bool res = send(message);
        }
    }

    Client::Client(bool runThreaded, LPCSTR pipe_name, int packet_size) : Pipe(runThreaded,pipe_name,packet_size){}

    bool Client::open(){
        // Open the named pipe
        // Most of these parameters aren't very relevant for pipes.
        pipe_ = CreateFile(
            pipe_name_,
            GENERIC_READ, // only need read access
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
        if (pipe_ == INVALID_HANDLE_VALUE) {
            std::cout << "Failed to connect to pipe." << std::endl;
            // look up error code here using GetLastError()
            return false;
        }
        return true;
    }

    bool Client::readPacket(std::string & packet){
        // The read operation will block until there is data to read
        char* buffer = new char[packet_size_]();
        DWORD numBytesRead = 0;
        BOOL result = ReadFile( //TODO: add timeout (https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o)
            pipe_,
            buffer, // the data from the pipe will be put here
            packet_size_, // number of bytes allocated
            &numBytesRead, // this will store number of bytes actually read
            NULL // not using overlapped IO
        );
        std::cout << "Bytes read: " << numBytesRead << std::endl;
        packet = buffer;
        return result;
    }

    bool Client::readLine(std::string & line, long long timeout){
        line = "";
        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        while(true){
            if (timeout != 0){ // timeout checking only enabled if timeout is not 0
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                long long dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
                if (dur > timeout){
                    std::cout << "Read line timed out" << std::endl;
                    return false;
                }
            }
            // The read operation will block until there is data to read
            std::string packet;
            bool result = readPacket(packet);
            if (result) {
                line += packet;
                std::size_t found = line.find('\n');
                if (found!=std::string::npos){
                    std::cout << "Line received" << std::endl;
                    return true;
                }
            } else {
                std::cout << "Failed to read data from the pipe." << std::endl;
                return false;
            }
        }
    }
};