#include "image2string.h"
#include "piper.h"

namespace Piper {
    class ImageServer : public Server {
        public:
            ImageServer(LPCSTR pipe_name, size_t packet_size);

            bool sendImage(cv::Mat image);
    };

    class ImageClient : public Client  {
        public:
            ImageClient(LPCSTR pipe_name, size_t packet_size);

            bool readImage(cv::Mat &image, long long timeout = 0);
    };
};