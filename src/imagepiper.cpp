#include "imagepiper.h"

namespace Piper {
        ImageServer::ImageServer(std::string pipe_name, size_t packet_size) : 
        Server(pipe_name,packet_size){}

        bool ImageServer::sendImage(cv::Mat image){
            std::string message = Image2String::mat2str(image,100);
            message += "\n";
            return send(message);
        }

        bool ImageServer::sendImageThreaded(cv::Mat image){
            if (!isSendThreadBusy()){
                future_sender_ = std::async(&ImageServer::sendImage, this, image);
                future_sender_init_ = true;
                return true;
            } else {
                std::cerr << "Pipes send thread is busy" << std::endl;
                return false;
            }
        }

        bool ImageServer::sendImagePair(cv::Mat image1, cv::Mat image2){
            bool res = sendImage(image1);
            res &= sendImage(image2);
            return res;
        }

        bool ImageServer::sendImagePairThreaded(cv::Mat image1, cv::Mat image2){
            if (!isSendThreadBusy()){
                future_sender_ = std::async(&ImageServer::sendImagePair, this, image1, image2);
                future_sender_init_ = true;
                return true;
            } else {
                std::cerr << "Pipes send thread is busy" << std::endl;
                return false;
            }
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
