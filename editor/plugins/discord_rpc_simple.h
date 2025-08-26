#pragma once

#include <string>

class SimpleDiscordRPC {
public:
    struct Activity {
        std::string state;
        std::string details; 
        std::string large_image;
        std::string large_text;
        std::string small_image;
        std::string small_text;
        int64_t start_timestamp = 0;
        int64_t end_timestamp = 0;
    };
    
    SimpleDiscordRPC(const std::string& client_id);
    ~SimpleDiscordRPC();
    
    bool connect();
    void disconnect();
    bool update_activity(const Activity& activity);
    void clear_activity();
    
private:
    std::string client_id_;
    int sock_fd_ = -1;
    
    bool send_frame(int opcode, const std::string& data);
    std::string create_activity_payload(const Activity& activity);
};
