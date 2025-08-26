
#ifndef DISCORD_RPC_PLUGIN_H
#define DISCORD_RPC_PLUGIN_H

#include "editor/editor_plugin.h"
#include "thirdparty/discord-rpc/include/discord_rpc.h"
#include "thirdparty/discord-rpc/include/discord_register.h"
#include <time.h>
#include <thread>
#include <chrono>

class DiscordRPCPlugin : public EditorPlugin {
    GDCLASS(DiscordRPCPlugin, EditorPlugin)

private:
    static DiscordRPCPlugin *singleton;
    static int64_t startTime;

    static void updateDiscordPresence();
    static void handleDiscordReady(const DiscordUser* connectedUser);
    static void handleDiscordDisconnected(int errcode, const char* message);
    static void handleDiscordError(int errcode, const char* message);
    static void discordInit();
    static void discordLoop();

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    static DiscordRPCPlugin *get_singleton();
    static int sendPresence;
    DiscordRPCPlugin();
    ~DiscordRPCPlugin();
};

#endif
