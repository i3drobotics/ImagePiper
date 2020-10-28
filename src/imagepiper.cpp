#include "imagepiper.h"

namespace Piper {
        ImageServer::ImageServer(std::string pipe_name, size_t packet_size) : 
        Server(pipe_name,packet_size){}

        bool ImageServer::sendImage(cv::Mat image){
            std::string message = Image2String::mat2str(image,100);
            message += "\n";
            return send(message);
        }

        ImageClient::ImageClient(std::string pipe_name, size_t packet_size) : 
        Client(pipe_name,packet_size){}

        bool ImageClient::readImage(cv::Mat &image, long long timeout){
            std::string message;
            bool res = readLine(message,timeout);
            if (res){
                image = Image2String::str2mat(message);
            }
            return res;
        }
};