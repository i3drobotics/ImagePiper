///// CLIENT SAMPLE PROGRAM /////
#include "imagepiper.h"
#include "pipeconfig.h"

int main(int argc, const char **argv)
{
    std::cout << "Connecting to pipe..." << std::endl;
    Piper::ImageClient client = Piper::ImageClient(PipeConfig::pipe_name,PipeConfig::packet_size);
    bool connected = client.open();
    if (connected){
        /*
        std::cout << "Reading data from file.." << std::endl;
        //compare against original file
        // read data from file
        std::ifstream infile("data.txt");
        std::string sLine;
        if (infile.good())
        {
            getline(infile, sLine);
            //cout << sLine << endl;
        }
        infile.close();

        std::string message;
        for (int i = 0; i < 10; i++){
            std::cout << "Reading data from pipe.." << std::endl;
            bool res = client.readLine(message);
            if (res){
                int compare_index = message.compare(sLine);
                if (compare_index != 0){
                    std::cout << "Received message does not match test data from file." << compare_index <<  std::endl;
                    std::cout << "Size: " << message.size() << "," << sLine.size() << std::endl;
                } else {
                    std::cout << "Received message matches data from file." << std::endl;
                }
            }
        }
        */
        cv::Mat image;
        while(true){
            bool res = client.readImage(image);
            if (res){
                std::cout << "Image message received" << std::endl;
                std::cout << "Type: " << image.type() << std::endl;
                cv::imshow("Client",image);
                //cv::resizeWindow("Client", 600,600);
                cv::waitKey(1);
            } else {
                break;
            }
       }
       /*
       std::string test_message = "Hello World!";
       std::string message;
       bool res = client.readLine(message);
       std::cout << message << std::endl;
       if (res){
            int compare_index = message.compare(test_message);
            if (compare_index != 0){
                std::cout << "Received message does not match test data from file." << compare_index <<  std::endl;
                std::cout << "Size: " << message.size() << "," << test_message.size() << std::endl;
            } else {
                std::cout << "Received message matches data from file." << std::endl;
            }
        }
        */
    }
    client.close();
    return 0;
}