/* gdsqlite.h */

#ifndef GDSQLITE_H
#define GDSQLITE_H

#include "core/reference.h"
#include "sqlite/sqlite3.h"
#include <map>

class GDSqlite : public Reference {
    GDCLASS(GDSqlite, Reference);

    //int reslt;

private:
    std::map<int, sqlite3*> db_store;
    int next_db_id;

protected:
    static void _bind_methods();

public:
    //int get_result();
    int open_database(const String &path);
    bool close_database(int dbId);
    Array query_database(int dbId, const String &query);
    String escape_string(const String &input);

    GDSqlite();
    ~GDSqlite();
};

#endif