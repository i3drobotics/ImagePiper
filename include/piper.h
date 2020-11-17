#ifndef PIPER_H_
#define PIPER_H_

#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>      // std::ifstream
#include <string>       // getline
#include <chrono>       // chrono::steady_clock
#include <thread>       // std::thread
#include <future>       // std::future

#define NOMINMAX
#include <windows.h>

namespace Piper {
    // Generic functions and parameters across both server and client
    // used to avoid repeated code
    class Pipe {
        public:
            Pipe(std::string pipe_name, size_t packet_size);

            
            void setPipeName(std::string pipe_name);
            void setPacketSize(int packet_size);

            std::string getPipeName();
            LPCWSTR getFullPipeNameW();
            std::string getFullPipeName();
            size_t getPacketSize();
            
            std::vector<std::string> splitPackets(std::string message, size_t packet_size, bool pad_packets = true);

            bool isOpen();

            virtual bool open() = 0;
            bool openThreaded();
            bool close();

            ~Pipe();

        protected:
            void padString(std::string& str, size_t packet_size, char padding_char = '\r');

            static const std::string pipe_prefix_;
            std::string pipe_name_;
            std::string full_pipe_name_;
            LPCWSTR full_pipe_name_w_;
            size_t packet_size_;
            HANDLE pipe_;

            std::future<bool> future_opener_;
            bool future_opener_init_ = false;
            bool pipe_open_ = false;
            bool try_connecting_ = true;
    };

    class Server : public Pipe {
        public:
            Server(std::string pipe_name, size_t packet_size);

            bool open();
            bool send(std::string message);
            bool sendThreaded(std::string message);

            bool isSendThreadBusy();

        protected:
            std::future<bool> future_sender_;
            bool future_sender_init_ = false;
    };

    class Client : public Pipe  {
        public:
            Client(std::string pipe_name, size_t packet_size);

            bool open();
            bool readPacket(std::string & packet);
            bool readLine(std::string & line, long long timeout=0); // timeout=0 means never timeout
    };
};

#endif /* PIPER_H_ */
