/* register_types.cpp */

#include "register_types.h"

#include "core/class_db.h"
#include "gdsqlite.h"

void register_gdsqlite_types() {
    ClassDB::register_class<GDSqlite>();
}

void unregister_gdsqlite_types() {
    // nothing 
}
