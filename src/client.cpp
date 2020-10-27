///// CLIENT SAMPLE PROGRAM /////
#include "imagepiper.h"

int main(int argc, const char **argv)
{
    std::cout << "Connecting to pipe..." << std::endl;
    ImagePiper::Client client = ImagePiper::Client();
    bool connected = client.open();
    if (connected){
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
                if (message.compare(sLine)){
                    std::cout << "Received message matches data from file." << std::endl;
                }
            }
        }
    }
    client.close();
    return 0;
}