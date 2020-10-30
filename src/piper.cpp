#include "piper.h"

namespace Piper {

    const std::string Pipe::pipe_prefix_ = "\\\\.\\pipe\\";
    Pipe::Pipe(std::string pipe_name, size_t packet_size): 
        pipe_name_(pipe_name),
        full_pipe_name_(pipe_prefix_+pipe_name),
        packet_size_(packet_size)
    {
        std::wstring stemp = std::wstring(full_pipe_name_.begin(), full_pipe_name_.end());
        full_pipe_name_w_ = stemp.c_str();
    }

    std::string Pipe::getPipeName(){return pipe_name_;}

    std::string Pipe::getFullPipeName(){return full_pipe_name_;}

    LPCWSTR Pipe::getFullPipeNameW(){return full_pipe_name_w_;}

    size_t Pipe::getPacketSize(){return packet_size_;}

    bool Pipe::openThreaded(){
        if (!isOpen()){
            future_opener_ = std::async(&Pipe::open, this);
            future_opener_init_ = true;
            return true;
        } else {
            std::cerr << "Pipe already open" << std::endl;
            return false;
        }
    }

    bool Pipe::isOpen(){
        if (future_opener_init_){
            using namespace std::chrono_literals;
            auto status = future_opener_.wait_for(0ms);
            bool open_complete_ = (status == std::future_status::ready);
            bool result = false;
            if (open_complete_){
                result = future_opener_.get();
                future_opener_init_ = false;
            }
            //pipe_open_ = result;
            return result;
        } else {
            return pipe_open_;
        }
    }

    std::vector<std::string> Pipe::splitPackets(std::string message, size_t packet_size, bool pad_packets){
        std::string srvMsg_msg = message;
        std::vector<std::string> srvMsg_msg_packets;
        //split message into packets
        size_t i = 0;
        while (true){
            size_t start_index = i*packet_size;
            if ((start_index + packet_size) < srvMsg_msg.size()){
                std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, packet_size);
                srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                i++;
            } else {
                size_t end_index = srvMsg_msg.size();
                size_t length = end_index - start_index;
                std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, length);
                if (pad_packets){
                    padString(srvMsg_msg_packet,packet_size);
                }
                srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                break;
            }
        }
        return srvMsg_msg_packets;
    }

    void Pipe::padString(std::string& str, size_t packet_size, char padding_char){
        str.append(std::string( (packet_size - str.size() ), padding_char));
    }

    bool Pipe::close(){
        future_opener_init_ = false;
        try_connecting_ = false;
        if (isOpen()){
            // Close the pipe (automatically disconnects client too)
            CloseHandle(pipe_);
            pipe_open_ = false;
        }
        return true;
    }

    Server::Server(std::string pipe_name, size_t packet_size) : 
        Pipe(pipe_name,packet_size){}

    bool Server::open(){
        try_connecting_ = true;
        //DWORD pipe_mode = PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED;
        DWORD pipe_mode = PIPE_ACCESS_OUTBOUND;
        // Create a pipe to send data
        pipe_ = CreateNamedPipeA(
            full_pipe_name_.c_str(), // name of the pipe
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
            DWORD err = GetLastError();
            std::cout << GetLastError() << std::endl;
            std::cout << "pipe name: " << full_pipe_name_ << std::endl;
            pipe_open_ = false;
            return pipe_open_;
        }
        std::cout << "Waiting for a client to connect to the pipe..." << std::endl;
        if (!ConnectNamedPipe(pipe_, NULL)) {
            std::cout << "Failed to make connection on named pipe." << std::endl;
            // look up error code here using GetLastError()
            std::cout << GetLastError() << std::endl;
            pipe_open_ = false;
            return pipe_open_;
        }
       /*
        // This call blocks until a client process connects to the pipe
        while (try_connecting_) { //TODO add optional timeout
            ConnectNamedPipe(pipe_, &overlapped);
            DWORD err = GetLastError();
            if (err == ERROR_PIPE_CONNECTED)
            {
                break;
            } else {
                //std::cout << err << std::endl;
            }
        }
        */
        pipe_open_ = true;
        return pipe_open_;
    }

    bool Server::send(std::string message){
        std::vector<std::string> msg_packets = Pipe::splitPackets(message,packet_size_);
        for (std::vector<std::string>::iterator it = msg_packets.begin() ; it != msg_packets.end(); ++it){
            std::cout << "Characters in string: " << it->size() << std::endl;
            const char* data = it->c_str();
            DWORD datalength = (DWORD)(packet_size_ * sizeof(char));
            //std::cout << "Number of bytes to send: " << datalength << std::endl;
            DWORD numBytesWritten = 0;
            // This call blocks until a client process reads all the data
            bool result = WriteFile( //TODO: add timeout (https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o)
                pipe_, // handle to our outbound pipe
                data, // data to send
                datalength, // length of data to send (bytes)
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
        return true;
    }

    bool Server::sendThreaded(std::string message){
        if (!isSendThreadBusy()){
            future_sender_ = std::async(&Server::send, this, message);
            future_sender_init_ = true;
            return true;
        } else {
            std::cerr << "Pipes send thread is busy" << std::endl;
            return false;
        }
    }

    bool Server::isSendThreadBusy(){
        if (future_sender_init_){
            using namespace std::chrono_literals;
            auto status = future_sender_.wait_for(0ms);
            bool send_complete = (status == std::future_status::ready);
            return !send_complete;
        } else {
            return false;
        }
    }

    Client::Client(std::string pipe_name, size_t packet_size) : Pipe(pipe_name,packet_size){}

    bool Client::open(){
        // Open the named pipe
        // Most of these parameters aren't very relevant for pipes.
        pipe_ = CreateFileA(
            full_pipe_name_.c_str(),
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
            std::cout << GetLastError() << std::endl;
            pipe_open_ = false;
            return pipe_open_;
        }
        pipe_open_ = true;
        return pipe_open_;
    }

    bool Client::readPacket(std::string & packet){
        // The read operation will block until there is data to read
        char* buffer = new char[packet_size_]();
        DWORD numBytesRead = 0;
        DWORD datalength = (DWORD)(packet_size_ * sizeof(char));
        BOOL result = ReadFile( //TODO: add timeout (https://docs.microsoft.com/en-us/windows/win32/ipc/named-pipe-server-using-overlapped-i-o)
            pipe_,
            buffer, // the data from the pipe will be put here
            datalength, // number of bytes allocated
            &numBytesRead, // this will store number of bytes actually read
            NULL // not using overlapped IO
        );
        std::cout << "Bytes read: " << numBytesRead << std::endl;
        packet = std::string(buffer);
        std::cout << "Packet string length: " << packet.size() << std::endl;
        packet = packet.substr(0,packet_size_); // used make sure string is not overflowing with extra data
        return result;
    }

    bool Client::readLine(std::string & line, long long timeout){
        std::string message = "";
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
                message += packet;
                std::size_t found = message.find('\n');
                if (found!=std::string::npos){
                    //std::cout << "Line received" << std::endl;
                    // remove padding and newline from end of string
                    line = message.substr(0,found);
                    return true;
                }
            } else {
                std::cout << "Failed to read data from the pipe." << std::endl;
                return false;
            }
        }
    }
};
