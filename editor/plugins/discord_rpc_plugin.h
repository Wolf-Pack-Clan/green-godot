#ifndef DISCORD_RPC_PLUGIN_H
#define DISCORD_RPC_PLUGIN_H

#include "editor/editor_plugin.h"
#include "discord_rpc_simple.h"
#include <time.h>
#include <thread>
#include <chrono>
#include <memory>
#include <atomic>

class DiscordRPCPlugin : public EditorPlugin {
    GDCLASS(DiscordRPCPlugin, EditorPlugin)

private:
    static DiscordRPCPlugin *singleton;
    static int64_t startTime;
    static std::atomic<bool> shouldRun;
    static std::thread updateThread;
    
    std::unique_ptr<SimpleDiscordRPC> rpc;

    static void updateDiscordPresence();
    static void discordLoop();
    void initDiscord();
    void shutdownDiscord();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    static DiscordRPCPlugin *get_singleton();
    DiscordRPCPlugin();
    ~DiscordRPCPlugin();
};

#endif
