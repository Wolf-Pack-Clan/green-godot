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

void DiscordRPCPlugin::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_delayed_init"), &DiscordRPCPlugin::_delayed_init);
}

void DiscordRPCPlugin::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		handleSettingsChange();
		if (enableRPC) {
			EditorNode::get_log()->add_message("Discord RPC Plugin: ready", EditorLog::MSG_TYPE_STD);
			//updateCachedData(); // Cache initial data
			//initDiscord();
			call_deferred("_delayed_init");
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
			updateCachedData(); // Update cache before enabling
			initDiscord();
		} else if (!enableRPC && wasEnabled) {
			// RPC was just disabled
			EditorNode::get_log()->add_message("Discord RPC: Disabling...", EditorLog::MSG_TYPE_STD);
			shutdownDiscord();
		}

		// Update cached data if RPC is running
		if (enableRPC && rpc) {
			updateCachedData();
			need_update = true; // Flag that we need to send an update
		}
	}

	if (p_what == NOTIFICATION_WM_TITLE_CHANGE) {
		print_line("Title changed");
		if (enableRPC && rpc) {
			updateCachedData();
			need_update = true;
		}
	}
}

DiscordRPCPlugin *DiscordRPCPlugin::get_singleton() {
	return singleton ? singleton : memnew(DiscordRPCPlugin);
}

void DiscordRPCPlugin::_delayed_init() {
	// This runs after the editor is fully initialized
	print_line("Discord RPC: Delayed initialization");
	updateCachedData(); // Cache initial data
	initDiscord();
}

void DiscordRPCPlugin::handleSettingsChange() {
	enableRPC = EDITOR_GET("discord_rpc/enable");
	showProjectName = EDITOR_GET("discord_rpc/show_project_name");
	showSceneName = EDITOR_GET("discord_rpc/show_scene_name");
	showSceneType = EDITOR_GET("discord_rpc/show_scene_type");
}

void DiscordRPCPlugin::updateCachedData() {
	// This method runs on the main thread and caches scene/project data
	// so the background thread doesn't need to access Godot's scene tree
	print_line("updateCachedData called");
	// Cache project name
	if (showProjectName) {
		String project_name = ProjectSettings::get_singleton()->get_setting("application/config/name");
		if (!project_name.empty()) {
			cached_project_name = "Project: " + project_name;
		} else {
			cached_project_name = "Untitled Project";
		}
	}

	// Cache scene name
	if (showSceneName) {
		EditorNode *editor = EditorNode::get_singleton();
		if (!editor)
			return;
		Node *scene = editor->get_edited_scene();
		if (!scene)
			return;
		const String scene_name = scene->get_name();
		print_line(scene_name);
		cached_scene_name = scene_name.empty() ? "Untitled Scene" : "Scene: " + scene_name;
		if (showSceneType) {
			const String scene_type = scene->get_class_name();
			print_line(scene_type);
			if (!scene_type.empty()) {
				cached_scene_name += " (" + scene_type + ")";
			}
		}
	}

	print_line("[updateCachedData] cached_project_name: " + cached_project_name + " cached_scene_name: " + cached_scene_name);
}

void DiscordRPCPlugin::updateDiscordPresence() {
	if (!singleton || !singleton->rpc || !enableRPC)
		return;

	SimpleDiscordRPC::Activity activity;

	// Use cached data (safe to access from background thread)
	print_line("[updateDiscordPresence] cached_project_name: " + singleton->cached_project_name + " cached_scene_name: " + singleton->cached_scene_name);
	activity.state = singleton->cached_scene_name;
	activity.details = singleton->cached_project_name;
	activity.large_image = "green-godot-1024";
	activity.large_text = "Godot Engine";
	activity.start_timestamp = startTime;

	singleton->rpc->update_activity(activity);
}

void DiscordRPCPlugin::discordLoop() {
	while (shouldRun.load()) {
		if (enableRPC && singleton) {
			// Only send updates if needed or periodically
			if (singleton->need_update) {
				updateDiscordPresence();
				singleton->need_update = false;
			}
		}

		// Sleep for 15 seconds (Discord recommends not updating too frequently)
		for (int i = 0; i < 50 && shouldRun.load(); ++i) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		// Send periodic update every 15 seconds to keep presence alive
		if (enableRPC && singleton && singleton->rpc) {
			updateDiscordPresence();
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

		// Send initial presence update (safe since we're on main thread)
		need_update = true;
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

DiscordRPCPlugin::DiscordRPCPlugin() :
		need_update(false) {
	singleton = this;
	EditorNode::get_log()->add_message("Discord RPC Plugin: start", EditorLog::MSG_TYPE_STD);
}

DiscordRPCPlugin::~DiscordRPCPlugin() {
	EditorNode::get_log()->add_message("Discord RPC Plugin: shutdown", EditorLog::MSG_TYPE_STD);
	shutdownDiscord();
	singleton = nullptr;
}
