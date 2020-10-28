#include <windows.h>

namespace PipeConfig {
    LPCSTR pipe_name = "\\\\.\\pipe\\i3dr_image_pipe";
    //int packet_size = 524288;
    int packet_size = 4194304;
}