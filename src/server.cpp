///// SERVER SAMPLE PROGRAM /////
#include "imagepiper.h"
#include <future>

void run(){
    std::cout << "Creating an instance of a named pipe..." << std::endl;
    ImagePiper::Server server = ImagePiper::Server();
    bool connected = server.open();
    if (connected){
        std::cout << "Reading data from file.." << std::endl;
        // read data from file
        std::ifstream infile("data.txt");
        std::string sLine;
        if (infile.good())
        {
            getline(infile, sLine);
        }
        infile.close();

        //add eol to string
        sLine += '\n';
        for (int i = 0; i < 10; i++){
            std::cout << "Sending data over pipe..." << std::endl;
            server.send(sLine);
        }
    }
    server.close();
}

void runThreaded(){
    using namespace std::chrono_literals;

    std::cout << "Creating an instance of a named pipe..." << std::endl;
    ImagePiper::Server server = ImagePiper::Server();
    
    bool connected = server.open();
    if (connected){
        std::cout << "Reading data from file.." << std::endl;
        // read data from file
        std::ifstream infile("data.txt");
        std::string sLine;
        if (infile.good())
        {
            getline(infile, sLine);
        }
        infile.close();

        //add eol to string
        sLine += '\n';
        for (int i = 0; i < 10; i++){
            std::cout << "Sending data over pipe..." << std::endl;
            //std::thread thread = std::thread(&ImagePiper::Server::send, server, sLine);
            std::future<bool> future = std::async(&ImagePiper::Server::send, server, sLine);
            std::cout << "Waiting for send thread to finish..." << std::endl;
            while(true){ // you could do other stuff here while the data is sent
                // Use wait_for() with zero milliseconds to check thread status.
                auto status = future.wait_for(0ms);

                // Print status.
                if (status == std::future_status::ready) {
                    std::cout << "Send complete." << std::endl;
                    break;
                }
            }
        }
    }
    server.close();
}

int main(int argc, const char **argv)
{
    runThreaded();
    return 0;
}