#include "discord_rpc_plugin.h"
#include "editor/editor_node.h"
#include "editor/editor_log.h"
#include "editor/editor_settings.h"
#include "core/os/os.h"

DiscordRPCPlugin *DiscordRPCPlugin::singleton = nullptr;
int64_t DiscordRPCPlugin::startTime = 0;
std::atomic<bool> DiscordRPCPlugin::shouldRun(false);
std::thread DiscordRPCPlugin::updateThread;

void DiscordRPCPlugin::_bind_methods() {}

void DiscordRPCPlugin::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        EditorNode::get_log()->add_message("Discord RPC Plugin: ready", EditorLog::MSG_TYPE_STD);
        initDiscord();
    }
    if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
        // Handle settings changes if needed
    }
}

DiscordRPCPlugin *DiscordRPCPlugin::get_singleton() {
    return singleton ? singleton : memnew(DiscordRPCPlugin);
}

void DiscordRPCPlugin::updateDiscordPresence() {
    if (!singleton || !singleton->rpc) return;
    
    SimpleDiscordRPC::Activity activity;
    activity.state = "Editing a scene";
    activity.details = "Working in Godot Editor";
    activity.large_image = "green-godot-1024";  // Make sure this asset exists in your Discord app
    activity.large_text = "Godot Engine";
    activity.start_timestamp = startTime;
    // Remove end timestamp to show elapsed time instead of countdown
    // activity.end_timestamp = time(0) + 5 * 60;
    
    singleton->rpc->update_activity(activity);
}

void DiscordRPCPlugin::discordLoop() {
    while (shouldRun.load()) {
        updateDiscordPresence();
        
        // Sleep for 15 seconds (Discord recommends not updating too frequently)
        for (int i = 0; i < 150 && shouldRun.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void DiscordRPCPlugin::initDiscord() {
    const char* APPLICATION_ID = "298522856241889280"; // Your Discord application ID
    
    rpc = std::make_unique<SimpleDiscordRPC>(APPLICATION_ID);
    
    if (rpc->connect()) {
        EditorNode::get_log()->add_message("Discord RPC: Connected successfully", EditorLog::MSG_TYPE_STD);
        startTime = OS::get_singleton()->get_unix_time();
        
        shouldRun.store(true);
        updateThread = std::thread(discordLoop);
        
        // Send initial presence update
        updateDiscordPresence();
    } else {
        EditorNode::get_log()->add_message("Discord RPC: Failed to connect", EditorLog::MSG_TYPE_ERROR);
        rpc.reset();
    }
}

void DiscordRPCPlugin::shutdownDiscord() {
    shouldRun.store(false);
    
    if (updateThread.joinable()) {
        updateThread.join();
    }
    
    if (rpc) {
        rpc->clear_activity();
        rpc->disconnect();
        rpc.reset();
    }
}

DiscordRPCPlugin::DiscordRPCPlugin() {
    singleton = this;
    EditorNode::get_log()->add_message("Discord RPC Plugin: start", EditorLog::MSG_TYPE_STD);
}

DiscordRPCPlugin::~DiscordRPCPlugin() {
    EditorNode::get_log()->add_message("Discord RPC Plugin: shutdown", EditorLog::MSG_TYPE_STD);
    shutdownDiscord();
    singleton = nullptr;
}
