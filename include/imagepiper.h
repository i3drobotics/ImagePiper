#ifndef IMAGEPIPER_H_
#define IMAGEPIPER_H_

#include "image2string.h"
#include "piper.h"

namespace Piper {
    class ImageServer : public Server {
        public:
            ImageServer(std::string pipe_name, size_t packet_size);

            bool sendImage(cv::Mat image);
            bool sendImageThreaded(cv::Mat image);
    };

    class ImageClient : public Client  {
        public:
            ImageClient(std::string pipe_name, size_t packet_size);

            bool readImage(cv::Mat &image, long long timeout = 0);
    };
};

#endif // IMAGEPIPER_H_