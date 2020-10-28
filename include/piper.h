#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>      // std::ifstream
#include <string>       // getline
#include <chrono>       // chrono::steady_clock
#include <thread>       // std::thread

#include <windows.h>

namespace Piper {
    // Generic functions and parameters across both server and client
    // used to avoid repeated code
    class Pipe {
        public:
            Pipe(LPCSTR pipe_name, size_t packet_size);

            LPCSTR getPipeName();
            size_t getPacketSize();
            std::vector<std::string> splitPackets(std::string message, size_t packet_size, bool pad_packets = true);

            virtual bool open() = 0;
            bool close();

            void waitForThreadFinish();

        protected:
            void padString(std::string& str, size_t packet_size, char padding_char = '\r');

            LPCSTR pipe_name_;
            size_t packet_size_;
            HANDLE pipe_;
    };

    class Server : public Pipe {
        public:
            Server(LPCSTR pipe_name, size_t packet_size);

            bool open();
            bool send(std::string message);
    };

    class Client : public Pipe  {
        public:
            Client(LPCSTR pipe_name, size_t packet_size);

            bool open();
            bool readPacket(std::string & packet);
            bool readLine(std::string & line, long long timeout=0); // timeout=0 means never timeout
    };
};