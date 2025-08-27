/**************************************************************************/
/*  discord_rpc_plugin.h                                                  */
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


#ifndef DISCORD_RPC_PLUGIN_H
#define DISCORD_RPC_PLUGIN_H

#include "discord_rpc_simple.h"
#include "editor/editor_plugin.h"
#include <time.h>
#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

class DiscordRPCPlugin : public EditorPlugin {
	GDCLASS(DiscordRPCPlugin, EditorPlugin)

private:
	static DiscordRPCPlugin *singleton;
	static int64_t startTime;
	static std::atomic<bool> shouldRun;
	static std::thread updateThread;
	static bool enableRPC;
	static bool showProjectName;
	static bool showSceneName;
	static bool showSceneType;

	std::unique_ptr<SimpleDiscordRPC> rpc;

	static void handleSettingsChange();
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
