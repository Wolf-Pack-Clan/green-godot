/**************************************************************************/
/*  gdsqlite.cpp                                                          */
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

/* gdsqlite.cpp */

#include "gdsqlite.h"

GDSqlite::GDSqlite() :
		next_db_id(1) {}

GDSqlite::~GDSqlite() {
	for (auto &pair : db_store) {
		sqlite3_close(pair.second);
	}
	db_store.clear();
}

int GDSqlite::open_database(const String &path) {
	if (!path.begins_with("user://")) {
		ERR_PRINT("Only user:// paths are supported for opening databases.");
		return -1;
	}
	String realpath = ProjectSettings::get_singleton()->globalize_path(path);

	sqlite3 *db = nullptr;
	int rc = sqlite3_open(realpath.utf8().get_data(), &db);

	if (rc != SQLITE_OK) {
		ERR_PRINT(vformat("Failed to open database '%s': %s", path, sqlite3_errmsg(db)));
		sqlite3_close(db);
		return -1;
	}

	int dbId = next_db_id++;
	db_store[dbId] = db;
	return dbId;
}

bool GDSqlite::close_database(int dbId) {
	auto it = db_store.find(dbId);
	if (it == db_store.end()) {
		ERR_PRINT(vformat("Database ID '%d' not found", dbId));
		return false;
	}
	sqlite3_close(it->second);
	db_store.erase(it);
	return true;
}

Array GDSqlite::query_database(int dbId, const String &query) {
	auto it = db_store.find(dbId);
	if (it == db_store.end()) {
		ERR_PRINT(vformat("Database ID '%d' not found", dbId));
		return Array();
	}

	sqlite3 *db = it->second;
	sqlite3_stmt *stmt = nullptr;

	const String esc_query = escape_string(query);

	int rc = sqlite3_prepare_v2(db, esc_query.utf8().get_data(), -1, &stmt, nullptr);

	if (rc != SQLITE_OK) {
		ERR_PRINT(vformat("Failed to prepare query '%s': %s", esc_query, sqlite3_errmsg(db)));
		return Array();
	}

	Array results;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		Array row;
		for (int i = 0; i < sqlite3_column_count(stmt); i++) {
			const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
			row.append(text ? String(text) : "");
		}
		results.append(row);
	}
	sqlite3_finalize(stmt);
	return results;
}

String GDSqlite::escape_string(const String &input) {
	char *escaped = sqlite3_mprintf("%q", input.utf8().get_data());
	String result = String(escaped);
	sqlite3_free(escaped);
	return result;
}

void GDSqlite::_bind_methods() {
	ClassDB::bind_method(D_METHOD("opendb", "path"), &GDSqlite::open_database);
	ClassDB::bind_method(D_METHOD("closedb", "dbId"), &GDSqlite::close_database);
	ClassDB::bind_method(D_METHOD("querydb", "dbId", "query"), &GDSqlite::query_database);
	//ClassDB::bind_method(D_METHOD("escape_str", "input"), &GDSqlite::escape_string);
}
