/**************************************************************************/
/*  discord_rpc_simple.h                                                  */
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

#pragma once

#include "core/ustring.h"

class SimpleDiscordRPC {
public:
	struct Activity {
		String state;
		String details;
		String large_image;
		String large_text;
		String small_image;
		String small_text;
		int64_t start_timestamp = 0;
		int64_t end_timestamp = 0;
	};

	SimpleDiscordRPC(const String &client_id);
	~SimpleDiscordRPC();

	bool connect();
	void disconnect();
	bool update_activity(const Activity &activity);
	void clear_activity();
	bool is_connected() const;

private:
	String client_id_;
	int sock_fd_ = -1;

	bool send_frame(int opcode, const String &data);
	String create_activity_payload(const Activity &activity);
};
