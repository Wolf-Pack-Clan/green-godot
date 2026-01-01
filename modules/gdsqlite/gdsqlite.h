/**************************************************************************/
/*  gdsqlite.h                                                            */
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

/* gdsqlite.h */

#ifndef GDSQLITE_H
#define GDSQLITE_H

#include "core/project_settings.h"
#include "core/reference.h"
#include "sqlite/sqlite3.h"
#include <map>

class GDSqlite : public Reference {
	GDCLASS(GDSqlite, Reference);

	//int reslt;

private:
	std::map<int, sqlite3 *> db_store;
	int next_db_id;
	String escape_string(const String &input);

protected:
	static void _bind_methods();

public:
	//int get_result();
	int open_database(const String &path);
	bool close_database(int dbId);
	Array query_database(int dbId, const String &query);

	GDSqlite();
	~GDSqlite();
};

#endif
