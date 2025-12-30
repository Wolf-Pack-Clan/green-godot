/**************************************************************************/
/*  discord_rpc_simple.cpp                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                              Green Godot                               */
/*             https://github.com/Wolf-Pack-Clan/green-godot              */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/* Copyright (c) 2025 Wolf Pack                                           */
/*                                                                        */
/* This program is free software: you can redistribute it and/or modify   */
/* it under the terms of the GNU General Public License as published by   */
/* the Free Software Foundation, either version 3 of the License, or      */
/* (at your option) any later version.                                    */
/*                                                                        */
/* This program is distributed in the hope that it will be useful,        */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/* GNU General Public License for more details.                           */
/*                                                                        */
/* You should have received a copy of the GNU General Public License      */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */
/**************************************************************************/

#include "discord_rpc_simple.h"
#include "core/os/os.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

SimpleDiscordRPC::SimpleDiscordRPC(const String &client_id) :
        client_id_(client_id) {}

SimpleDiscordRPC::~SimpleDiscordRPC() {
    disconnect();
}

bool SimpleDiscordRPC::connect() {
    // Multiple socket path possibilities
    Vector<String> socket_bases;
    socket_bases.push_back("/tmp/discord-ipc-");

    // Check for XDG_RUNTIME_DIR (common on Linux)
    const char *xdg_runtime = getenv("XDG_RUNTIME_DIR");
    if (xdg_runtime) {
        socket_bases.push_back(String(xdg_runtime) + "/discord-ipc-");
    }

    // Check for TMPDIR
    const char *tmpdir = getenv("TMPDIR");
    if (tmpdir) {
        socket_bases.push_back(String(tmpdir) + "/discord-ipc-");
    }

    // Also try some common paths
    socket_bases.push_back("/run/user/" + String::num(getuid()) + "/discord-ipc-");

    print_line("[DiscordRPC] Searching for Discord sockets...");

    // Try each base path with different socket numbers
    for (int base_idx = 0; base_idx < socket_bases.size(); base_idx++) {
        String socket_base = socket_bases[base_idx];

        for (int i = 0; i < 10; ++i) {
            String socket_path = socket_base + String::num(i);
            CharString socket_path_utf8 = socket_path.utf8();

            //print_line("[DiscordRPC] Attempting to connect to socket: " + socket_path);

            // Check if socket file exists first
            if (access(socket_path_utf8.get_data(), F_OK) != 0) {
                //print_line("[DiscordRPC] Socket file doesn't exist: " + socket_path);
                continue;
            }

            sock_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
            if (sock_fd_ < 0) {
                print_line("[DiscordRPC] Failed to create socket");
                continue;
            }
#ifdef SO_NOSIGPIPE
            int set = 1;
            setsockopt(sock_fd_, SOL_SOCKET, SO_NOSIGPIPE, &set, sizeof(set));
#endif

            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socket_path_utf8.get_data(), sizeof(addr.sun_path) - 1);

            if (::connect(sock_fd_, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
                print_line("[DiscordRPC] Successfully connected to: " + socket_path);

                // Send handshake
                String handshake = String("{\"v\":1,\"client_id\":\"") + client_id_ + String("\"}");
                if (send_frame(0, handshake)) {
                    print_line("[DiscordRPC] Handshake successful");
                    return true;
                } else {
                    print_line("[DiscordRPC] Handshake failed");
                }
            } else {
                print_line("[DiscordRPC] Failed to connect to socket: " + socket_path + " (errno: " + String::num(errno) + ")");
            }

            close(sock_fd_);
            sock_fd_ = -1;
        }
    }

    print_line("[DiscordRPC] No Discord sockets found. Make sure Discord is running.");
    return false;
}

void SimpleDiscordRPC::disconnect() {
    if (sock_fd_ >= 0) {
        close(sock_fd_);
        sock_fd_ = -1;
    }
}

bool SimpleDiscordRPC::update_activity(const Activity &activity) {
    if (sock_fd_ < 0)
        return false;

    String payload = create_activity_payload(activity);
    return send_frame(1, payload);
}

void SimpleDiscordRPC::clear_activity() {
    if (sock_fd_ < 0)
        return;

    String payload = String("{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":") +
            String::num(getpid()) + String(",\"activity\":null},\"nonce\":\"clear\"}");
    send_frame(1, payload);
}

bool SimpleDiscordRPC::send_frame(int opcode, const String &data) {
    if (sock_fd_ < 0)
        return false;

    CharString data_utf8 = data.utf8();
    uint32_t length = data_utf8.length();

    // Discord IPC frame format: [opcode:4][length:4][data:length]
    char header[8];
    memcpy(header, &opcode, 4);
    memcpy(header + 4, &length, 4);

    ssize_t sent = send(sock_fd_, header, 8, MSG_NOSIGNAL);
    if (sent != 8) {
        print_line("[DiscordRPC] Failed to send header (sent" + String::num_int64(sent) + " bytes, errno: " + String::num(errno) + ")");
        // Socket might be broken, close it
        disconnect();
        return false;
    }

    sent = send(sock_fd_, data_utf8.get_data(), length, MSG_NOSIGNAL);
    if (sent != (ssize_t)length) {
        print_line("[DiscordRPC] Failed to send data (sent " + String::num_int64(sent) + " bytes, errno: " + String::num(errno) + ")");
        // Socket might be broken, close it
        disconnect();
        return false;
    }

    return true;
}

String SimpleDiscordRPC::create_activity_payload(const Activity &activity) {
    String payload = String("{\"cmd\":\"SET_ACTIVITY\",\"args\":{\"pid\":") + String::num(getpid()) + String(",\"activity\":{");

    bool first = true;

    if (!activity.state.empty()) {
        payload += String("\"state\":\"") + activity.state.c_escape() + String("\"");
        first = false;
    }

    if (!activity.details.empty()) {
        if (!first)
            payload += ",";
        payload += String("\"details\":\"") + activity.details.c_escape() + String("\"");
        first = false;
    }

    if (activity.start_timestamp > 0 || activity.end_timestamp > 0) {
        if (!first)
            payload += ",";
        payload += String("\"timestamps\":{");

        bool ts_first = true;
        if (activity.start_timestamp > 0) {
            payload += String("\"start\":") + String::num_int64(activity.start_timestamp);
            ts_first = false;
        }
        if (activity.end_timestamp > 0) {
            if (!ts_first)
                payload += ",";
            payload += String("\"end\":") + String::num_int64(activity.end_timestamp);
        }
        payload += "}";
        first = false;
    }

    if (!activity.large_image.empty() || !activity.small_image.empty()) {
        if (!first)
            payload += ",";
        payload += String("\"assets\":{");

        bool asset_first = true;
        if (!activity.large_image.empty()) {
            payload += String("\"large_image\":\"") + activity.large_image.c_escape() + String("\"");
            if (!activity.large_text.empty()) {
                payload += String(",\"large_text\":\"") + activity.large_text.c_escape() + String("\"");
            }
            asset_first = false;
        }
        if (!activity.small_image.empty()) {
            if (!asset_first)
                payload += ",";
            payload += String("\"small_image\":\"") + activity.small_image.c_escape() + String("\"");
            if (!activity.small_text.empty()) {
                payload += String(",\"small_text\":\"") + activity.small_text.c_escape() + String("\"");
            }
        }
        payload += "}";
    }

    payload += String("}},\"nonce\":\"update\"}");
    return payload;
}

bool SimpleDiscordRPC::is_connected() const {
    if (sock_fd_ < 0)
        return false;

    // Use recv with MSG_PEEK to check if socket is still valid
    char buf[1];
    ssize_t result = recv(sock_fd_, buf, 1, MSG_PEEK | MSG_DONTWAIT);
    if (result == 0) {
        // Socket closed by peer
        return false;
    }
    if (result < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        // Socket error
        return false;
    }
    return true;
}
