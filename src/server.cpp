///// SERVER SAMPLE PROGRAM /////
#include "imagepiper.h"
#include "pipeconfig.h"

void run(){
    std::cout << "Creating an instance of a named pipe..." << std::endl;
    Piper::ImageServer server = Piper::ImageServer(PipeConfig::pipe_name,PipeConfig::packet_size);
    bool connected = server.openThreaded();
    while(!server.isOpen()){
        std::cout << "Waiting for pipe to be open" << std::endl;
    }
    if (connected){
        /*
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
        */
        cv::Mat test_img(2448, 2048, CV_32FC3);
        for (int i = 0; i < 1000; i++){
            std::cout << "Sending image over pipe..." << std::endl;
            cv::randu(test_img, cv::Scalar(0, 0, 0), cv::Scalar(1, 1, 1));
            std::string message = Image2String::mat2str(test_img) + "\n";
            bool res = server.send(message);
            if (!res){
                break;
            }
        }
       /*
        server.send("Hello World!\n");
        */
    }
    server.close();
}

/*
void runThreaded(){
    using namespace std::chrono_literals;

    std::cout << "Creating an instance of a named pipe..." << std::endl;
    Piper::Server server = Piper::Server(PipeConfig::pipe_name,PipeConfig::packet_size);
    
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
            std::future<bool> future = std::async(&Piper::Server::send, server, sLine);
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
*/

int main(int argc, const char **argv)
{
    run();
    return 0;
}