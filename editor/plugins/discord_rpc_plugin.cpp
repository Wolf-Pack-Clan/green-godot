/**************************************************************************/
/*  discord_rpc_plugin.cpp                                                */
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


#include "discord_rpc_plugin.h"
#include "core/os/os.h"
#include "editor/editor_log.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"
#include "scene/main/scene_tree.h"

DiscordRPCPlugin *DiscordRPCPlugin::singleton = nullptr;
int64_t DiscordRPCPlugin::startTime = 0;
std::atomic<bool> DiscordRPCPlugin::shouldRun(false);
std::thread DiscordRPCPlugin::updateThread;

bool DiscordRPCPlugin::enableRPC = false;
bool DiscordRPCPlugin::showProjectName = false;
bool DiscordRPCPlugin::showSceneName = false;
bool DiscordRPCPlugin::showSceneType = false;

void DiscordRPCPlugin::_bind_methods() {}

void DiscordRPCPlugin::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		handleSettingsChange();

		if (enableRPC) {
			EditorNode::get_log()->add_message("Discord RPC Plugin: ready", EditorLog::MSG_TYPE_STD);
			initDiscord();
		} else {
			EditorNode::get_log()->add_message("Discord RPC Plugin: disabled in settings", EditorLog::MSG_TYPE_STD);
		}
	}

	if (p_what == EditorSettings::NOTIFICATION_EDITOR_SETTINGS_CHANGED) {
		bool wasEnabled = enableRPC;
		handleSettingsChange();

		// Handle enable/disable changes
		if (enableRPC && !wasEnabled) {
			// RPC was just enabled
			EditorNode::get_log()->add_message("Discord RPC: Enabling...", EditorLog::MSG_TYPE_STD);
			initDiscord();
		} else if (!enableRPC && wasEnabled) {
			// RPC was just disabled
			EditorNode::get_log()->add_message("Discord RPC: Disabling...", EditorLog::MSG_TYPE_STD);
			shutdownDiscord();
		}
		// If already running, presence will update automatically with new settings
	}
}

DiscordRPCPlugin *DiscordRPCPlugin::get_singleton() {
	return singleton ? singleton : memnew(DiscordRPCPlugin);
}

void DiscordRPCPlugin::handleSettingsChange() {
	enableRPC = EDITOR_GET("discord_rpc/enable");
	showProjectName = EDITOR_GET("discord_rpc/show_project_name");
	showSceneName = EDITOR_GET("discord_rpc/show_scene_name");
	showSceneType = EDITOR_GET("discord_rpc/show_scene_type");
}

void DiscordRPCPlugin::updateDiscordPresence() {
	if (!singleton || !singleton->rpc || !enableRPC)
		return;

	SimpleDiscordRPC::Activity activity;

	// Build state string (scene type info)

	if (showSceneName) {
		String state = "";
		// Try to get current scene info
		SceneTree *scene_tree = SceneTree::get_singleton();
		if (scene_tree && scene_tree->get_current_scene()) {
			Node *current_scene = scene_tree->get_current_scene();
			String scene_name = current_scene->get_filename();
			print_line("Scene Name 1:");
			print_line(scene_name);
			if (!scene_name.empty()) {
				// Extract just the filename
				scene_name = scene_name.get_file();
				print_line("Scene Name 2:");
				print_line(scene_name);
				state = "Editing: " + scene_name;
			} else {
				state = "Editing unsaved scene";
			}

			if (showSceneType) {
				String scene_type = current_scene->get_class();
				print_line("Scene Type:");
				print_line(scene_type);
				print_line(scene_type);
				if (!scene_type.empty()) {
					state += " (" + scene_type + ")";
				}
			}
		}
		print_line(state);
		activity.state = state;
	}

	// Build details string (project info)
	if (showProjectName) {
		String details = "";
		String project_name = ProjectSettings::get_singleton()->get_setting("application/config/name");
		if (!project_name.empty()) {
			details = "Project: " + project_name;
		} else {
			details = "Untitled Project";
		}
		activity.details = details;
	}

	activity.large_image = "green-godot-1024";
	activity.large_text = "Green Godot";
	activity.start_timestamp = startTime;

	singleton->rpc->update_activity(activity);
}

void DiscordRPCPlugin::discordLoop() {
	while (shouldRun.load()) {
		if (enableRPC) {
			updateDiscordPresence();
		}

		// Sleep for 15 seconds (Discord recommends not updating too frequently)
		for (int i = 0; i < 50 && shouldRun.load(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
}

void DiscordRPCPlugin::initDiscord() {
	// Don't init if already running or if disabled
	if (rpc || !enableRPC)
		return;

	EditorSettings *settings = EditorSettings::get_singleton();
	String app_id = settings->get_setting("discord_rpc/application_id");

	if (app_id.empty()) {
		EditorNode::get_log()->add_message("Discord RPC: No application ID set", EditorLog::MSG_TYPE_ERROR);
		return;
	}

	print_line("[DiscordRPC] Initializing with app ID: " + app_id);
	rpc = std::make_unique<SimpleDiscordRPC>(app_id);

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
