#include "discord_rpc_simple.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <cstdint>

SimpleDiscordRPC::SimpleDiscordRPC(const std::string& client_id) 
    : client_id_(client_id) {}

SimpleDiscordRPC::~SimpleDiscordRPC() {
    disconnect();
}

bool SimpleDiscordRPC::connect() {
    // Try connecting to Discord IPC socket
    for (int i = 0; i < 10; ++i) {
        std::string socket_path = "/tmp/discord-ipc-" + std::to_string(i);
        
        sock_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock_fd_ < 0) continue;
        
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
        
        if (::connect(sock_fd_, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            // Send handshake
            std::string handshake = R"({"v":1,"client_id":")" + client_id_ + R"("})";
            if (send_frame(0, handshake)) {
                return true;
            }
        }
        
        close(sock_fd_);
        sock_fd_ = -1;
    }
    
    return false;
}

void SimpleDiscordRPC::disconnect() {
    if (sock_fd_ >= 0) {
        close(sock_fd_);
        sock_fd_ = -1;
    }
}

bool SimpleDiscordRPC::update_activity(const Activity& activity) {
    if (sock_fd_ < 0) return false;
    
    std::string payload = create_activity_payload(activity);
    return send_frame(1, payload);
}

void SimpleDiscordRPC::clear_activity() {
    if (sock_fd_ < 0) return;
    
    std::string payload = R"({"cmd":"SET_ACTIVITY","args":{"pid":)" + 
                         std::to_string(getpid()) + R"(,"activity":null},"nonce":"clear"})";
    send_frame(1, payload);
}

bool SimpleDiscordRPC::send_frame(int opcode, const std::string& data) {
    if (sock_fd_ < 0) return false;
    
    uint32_t length = data.length();
    
    // Discord IPC frame format: [opcode:4][length:4][data:length]
    char header[8];
    memcpy(header, &opcode, 4);
    memcpy(header + 4, &length, 4);
    
    if (send(sock_fd_, header, 8, 0) != 8) return false;
    if (send(sock_fd_, data.c_str(), length, 0) != (ssize_t)length) return false;
    
    return true;
}

std::string SimpleDiscordRPC::create_activity_payload(const Activity& activity) {
    std::ostringstream payload;
    payload << R"({"cmd":"SET_ACTIVITY","args":{"pid":)" << getpid() << R"(,"activity":{)";
    
    bool first = true;
    
    if (!activity.state.empty()) {
        payload << R"("state":")" << activity.state << R"(")";
        first = false;
    }
    
    if (!activity.details.empty()) {
        if (!first) payload << ",";
        payload << R"("details":")" << activity.details << R"(")";
        first = false;
    }
    
    if (activity.start_timestamp > 0 || activity.end_timestamp > 0) {
        if (!first) payload << ",";
        payload << R"("timestamps":{)";
        
        bool ts_first = true;
        if (activity.start_timestamp > 0) {
            payload << R"("start":)" << activity.start_timestamp;
            ts_first = false;
        }
        if (activity.end_timestamp > 0) {
            if (!ts_first) payload << ",";
            payload << R"("end":)" << activity.end_timestamp;
        }
        payload << "}";
        first = false;
    }
    
    if (!activity.large_image.empty() || !activity.small_image.empty()) {
        if (!first) payload << ",";
        payload << R"("assets":{)";
        
        bool asset_first = true;
        if (!activity.large_image.empty()) {
            payload << R"("large_image":")" << activity.large_image << R"(")";
            if (!activity.large_text.empty()) {
                payload << R"(,"large_text":")" << activity.large_text << R"(")";
            }
            asset_first = false;
        }
        if (!activity.small_image.empty()) {
            if (!asset_first) payload << ",";
            payload << R"("small_image":")" << activity.small_image << R"(")";
            if (!activity.small_text.empty()) {
                payload << R"(,"small_text":")" << activity.small_text << R"(")";
            }
        }
        payload << "}";
    }
    
    payload << R"(}},"nonce":"update"})";
    return payload.str();
}
