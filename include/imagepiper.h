#include <iostream>
#include <vector>
#include <iostream>
#include <fstream>      // std::ifstream
#include <string>       // getline
#include <chrono>       // chrono::steady_clock
#include <thread>       // std::thread

#include <windows.h>

namespace ImagePiper {
    static const LPCSTR default_pipe_name = "\\\\.\\pipe\\i3dr_image_pipe";
    static const int default_packet_size = 524288;

    // Generic functions and parameters across both server and client
    // used to avoid repeated code
    class Pipe {
        public:
            Pipe(bool runThreaded = false, LPCSTR pipe_name = default_pipe_name, int packet_size = default_packet_size);

            LPCSTR getPipeName();
            int getPacketSize();

            virtual bool open() = 0;
            bool close();

            void waitForThreadFinish();

        protected:
            std::vector<std::string> splitPackets(std::string message, size_t packet_size, bool pad_packets = true);
            std::string padString(std::string str, size_t packet_size, char padding_char = '\r');

            LPCSTR pipe_name_;
            int packet_size_;
            HANDLE pipe_;
            bool runThreaded_;

            OVERLAPPED overlap_;
            HANDLE event_;
    };

    class Server : public Pipe {
        public:
            Server(bool runThreaded = false, LPCSTR pipe_name = default_pipe_name, int packet_size = default_packet_size);

            bool open();
            bool send(std::string message);
            void test(std::string message);
    };

    class Client : public Pipe  {
        public:
            Client(bool runThreaded = false, LPCSTR pipe_name = default_pipe_name, int packet_size = default_packet_size);

            bool open();
            bool readPacket(std::string & packet);
            bool readLine(std::string & line, long long timeout=0); // timeout=0 means never timeout
    };
};