/* gdsqlite.cpp */

#include "gdsqlite.h"

GDSqlite::GDSqlite() : next_db_id(1) {}

GDSqlite::~GDSqlite() {
    for(auto &pair : db_store) {
        sqlite3_close(pair.second);
    }
    db_store.clear();
}

int GDSqlite::open_database(const String &path)
{
    sqlite3 *db = nullptr;
    int rc = sqlite3_open(path.utf8().get_data(), &db);

    if(rc != SQLITE_OK) {
        ERR_PRINT(vformat("Failed to open database '%s': %s", path, sqlite3_errmsg(db)));
        sqlite3_close(db);
        return -1;
    }

    int dbId = next_db_id++;
    db_store[dbId] = db;
    return dbId;
}

bool GDSqlite::close_database(int dbId)
{
    auto it = db_store.find(dbId);
    if(it == db_store.end()) {
        ERR_PRINT(vformat("Database ID '%d' not found", dbId));
        return false;
    }
    sqlite3_close(it->second);
    db_store.erase(it);
    return true;
}

Array GDSqlite::query_database(int dbId, const String &query) {
    auto it = db_store.find(dbId);
    if(it == db_store.end()) {
        ERR_PRINT(vformat("Database ID '%d' not found", dbId));
        return Array();
    }

    sqlite3 *db = it->second;
    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(db, query.utf8().get_data(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        ERR_PRINT(vformat("Failed to prepare query '%s': %s", query, sqlite3_errmsg(db)));
        return Array();
    }

    Array results;
    while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        Array row;
        for(int i=0; i<sqlite3_column_count(stmt); i++) {
            const char *text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.append(text ? String(text) : "");
        }
        results.append(row);
    }
    sqlite3_finalize(stmt);
    return results;
}

String GDSqlite::escape_string(const String &input)
{
    char *escaped = sqlite3_mprintf("%q", input.utf8().get_data());
    String result = String(escaped);
    sqlite3_free(escaped);
    return result;
}

//int GDSqlite::get_result() {
//    return 123;
//}

void GDSqlite::_bind_methods() {
    //ClassDB::bind_method(D_METHOD("sqlitetest"), &GDSqlite::get_result);
    ClassDB::bind_method(D_METHOD("opendb", "path"), &GDSqlite::open_database);
    ClassDB::bind_method(D_METHOD("closedb", "dbId"), &GDSqlite::close_database);
    ClassDB::bind_method(D_METHOD("querydb", "dbId"), &GDSqlite::query_database);
    ClassDB::bind_method(D_METHOD("escape_str", "input"), &GDSqlite::escape_string);
}

//GDSqlite::GDSqlite() {
//    reslt = 0;
//}
