#include <vector>

namespace StereoPipe {
    static const int PACKET_SIZE = 524288;
    //static const int PACKET_SIZE = 917504;
    //static const int PACKET_SIZE = 32768;
    static LPCSTR pipename = "\\\\.\\pipe\\i3dr_message_pipe";
    
    class Support {
        public:
            static std::vector<std::string> splitPackets(std::string message, size_t max_buffer_length, bool pad_packets = true){
                std::string srvMsg_msg = message;
                std::vector<std::string> srvMsg_msg_packets;
                //split message into packets
                int i = 0;
                while (true){
                    int start_index = i*max_buffer_length;
                    if ((start_index + max_buffer_length) < srvMsg_msg.size()){
                        std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, max_buffer_length);
                        srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                        i++;
                    } else {
                        int end_index = srvMsg_msg.size();
                        int length = end_index - start_index;
                        std::string srvMsg_msg_packet = srvMsg_msg.substr(start_index, length);
                        srvMsg_msg_packet = padString(srvMsg_msg_packet,max_buffer_length);
                        srvMsg_msg_packets.push_back(srvMsg_msg_packet);
                        break;
                    }
                }
                return srvMsg_msg_packets;
            }

            static std::string padString(std::string str, size_t packet_size, char padding_char = '\r'){
                return str.append(std::string( (packet_size - str.size() ), padding_char));
            }
    };
};