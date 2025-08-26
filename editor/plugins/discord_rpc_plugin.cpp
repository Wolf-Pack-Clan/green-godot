
#include "discord_rpc_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_log.h"
#include "editor/editor_settings.h"

DiscordRPCPlugin *DiscordRPCPlugin::singleton = nullptr;
int DiscordRPCPlugin::sendPresence = 0;
int64_t DiscordRPCPlugin::startTime = 0;

void DiscordRPCPlugin::_bind_methods() {}

void DiscordRPCPlugin::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        EditorNode::get_log()->add_message("Discord RPC Plugin: ready", EditorLog::MSG_TYPE_STD);
        sendPresence = 1;
        discordInit();
        discordLoop();
    }
    if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
        sendPresence = 0;
    }
}

DiscordRPCPlugin *DiscordRPCPlugin::get_singleton() {
    return singleton ? singleton : memnew(DiscordRPCPlugin);
}

void DiscordRPCPlugin::updateDiscordPresence()
{
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    //discordPresence.state = "Editing a 2D scene";
    //discordPresence.details = "Project: project";
    discordPresence.startTimestamp = startTime;
    discordPresence.endTimestamp = time(0) + 5 * 60;
    discordPresence.largeImageKey = "green-godot-1024";
    discordPresence.largeImageText = "Green Godot";
    //discordPresence.joinSecret = " ";
    Discord_UpdatePresence(&discordPresence);
}

void DiscordRPCPlugin::handleDiscordReady(const DiscordUser* connectedUser)
{
    printf("\nDiscord: connected to user %s#%s - %s\n",
           connectedUser->username,
           connectedUser->discriminator,
           connectedUser->userId);
}

void DiscordRPCPlugin::handleDiscordDisconnected(int errcode, const char* message)
{
    printf("\nDiscord: disconnected (%d: %s)\n", errcode, message);
}

void DiscordRPCPlugin::handleDiscordError(int errcode, const char* message)
{
    printf("\nDiscord: error (%d: %s)\n", errcode, message);
}

void DiscordRPCPlugin::discordInit()
{
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = DiscordRPCPlugin::handleDiscordReady;
    handlers.disconnected = DiscordRPCPlugin::handleDiscordDisconnected;
    handlers.errored = DiscordRPCPlugin::handleDiscordError;
    //handlers.joinGame = handleDiscordJoin;
    //handlers.spectateGame = handleDiscordSpectate;
    //handlers.joinRequest = handleDiscordJoinRequest;
    const char* APPLICATION_ID = "2985228562";
    Discord_Initialize(APPLICATION_ID, &handlers, 1, NULL);
}

void DiscordRPCPlugin::discordLoop()
{
    startTime = time(0);

    while(sendPresence) {
        updateDiscordPresence();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

DiscordRPCPlugin::DiscordRPCPlugin() {
    singleton = this;
    EditorNode::get_log()->add_message("Discord RPC Plugin: start", EditorLog::MSG_TYPE_STD);
}

DiscordRPCPlugin::~DiscordRPCPlugin() {
    EditorNode::get_log()->add_message("Discord RPC Plugin: shutdown", EditorLog::MSG_TYPE_STD);
}
