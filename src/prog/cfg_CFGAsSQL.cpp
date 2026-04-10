/*
 *	CFGAsSQL class interface
 *
 *	This file is part of OTAWA
 *	Copyright (c) 2022, IRIT UPS.
 *
 *	OTAWA is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	OTAWA is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with OTAWA; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <otawa/cfg/CFG.h>
#include <otawa/cfg/features.h>
#include <otawa/proc/Processor.h>
#include <otawa/prog/Process.h>
#include <otawa/prog/File.h>
#include <cstdint>
#include <sqlite3.h>
#include <elm/checksum/MD5.h>
#include <elm/sys/System.h>
#include <elm/string/StringBuffer.h>

//
// demangle.hpp — C++ symbol demangler
//
// Uses platform-provided library calls only:
//   • GCC / Clang (Linux, macOS, MinGW): abi::__cxa_demangle  (libstdc++ / libc++)
//   • MSVC (Windows):                    UnDecorateSymbolName  (DbgHelp.lib)
//
// Usage:
//   std::string name = demangle("_ZN3Foo3barEv");    // GCC/Clang mangling
//   std::string name = demangle("?bar@Foo@@QEAAXXZ"); // MSVC mangling
//

// -------------------------------------------------------------------------
// GCC / Clang  (also covers MinGW on Windows)
// -------------------------------------------------------------------------
#if defined(__GNUC__) || defined(__clang__)

#include <cstdlib>
#include <cxxabi.h>

inline elm::string demangle(const char* mangled) {
    if (!mangled) return {};
    int status = 0;
    char* buf = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    if (status == 0 && buf) {
        elm::string result(buf);
        free(buf);
        return result;
    }
    return mangled; // pass-through on failure
}

// -------------------------------------------------------------------------
// MSVC  (Windows, native toolchain)
// -------------------------------------------------------------------------
#elif defined(_MSC_VER)

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

inline elm::string demangle(const char* mangled) {
    if (!mangled) return {};
    char buf[1024];
    DWORD len = UnDecorateSymbolName(
        mangled, buf, static_cast<DWORD>(sizeof(buf)),
        UNDNAME_COMPLETE);
    if (len > 0) return elm::string(buf, len);
    return mangled; // pass-through on failure
}

// -------------------------------------------------------------------------
// Unsupported platform — hard error at compile time
// -------------------------------------------------------------------------
#else
#  error "demangle.hpp: unsupported compiler/platform"
#endif

// Convenience overload for std::string input
inline elm::string demangle(const elm::string& mangled) {
    return demangle(mangled.asSysString());
}

namespace otawa {

class CFGAsSQL: public Processor {
public:
    static p::declare reg;
    CFGAsSQL(p::declare& r = reg): Processor(r) {}

protected:
    
    void configure(const PropList& props) override {
        Processor::configure(props);
        path = CFG_DUMP_PATH(props); 
        dbpath = CFG_DUMP_PATH_DB(props); 
    }
    
    void processWorkSpace(WorkSpace * ws) override {
        
        // compute path
        Path path = this->dbpath;
        if(path.isEmpty()) {
            path = this->path;
            if(path.isEmpty())
               path = TASK_INFO_FEATURE.get(ws)->workDirectory();
            path = path / "cfg.db";
        }
        
        try {
            sqlite3* db = nullptr;
            char* err = nullptr;
            
            // Open / create the db
            if (sqlite3_open(path.toString().asSysString(), &db) != SQLITE_OK)
                throw ProcessorException(*this, _ <<  "Error opening db: " << sqlite3_errmsg(db));

            // Enable foreign key contraint
            sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        
            // Create tables
            const char * sql_create_functions =
                "CREATE TABLE IF NOT EXISTS functions ("
                "id INTEGER UNIQUE PRIMARY KEY AUTOINCREMENT,"
                "name TEXT NOT NULL,"
                "demangled_name TEXT NOT NULL,"
                "entry INTEGER"
                ");";

            const char* sql_create_basic_blocks =
                "CREATE TABLE IF NOT EXISTS basic_blocks ("
                "address INTEGER PRIMARY KEY,"
                "function INTEGER NOT NULL,"
                "FOREIGN KEY (function) REFERENCES functions(id) ON DELETE CASCADE"
                ");";

            const char* sql_create_sequences = 
                "CREATE TABLE IF NOT EXISTS sequences ("
                "address INTEGER UNIQUE, "
                "address_next INTEGER UNIQUE, "
                "is_control INTEGER,"
                "FOREIGN KEY (address) REFERENCES basic_blocks(address) ON DELETE CASCADE, "
                "FOREIGN KEY (address_next) REFERENCES basic_blocks(address) ON DELETE CASCADE"
                ");";

            const char* sql_create_exit_points =
                "CREATE TABLE IF NOT EXISTS exit_points ("
                "function INTEGER NOT NULL,"
                "address INTEGER NOT NULL,"
                "FOREIGN KEY(function) REFERENCES functions(id) ON DELETE CASCADE"
                ");";

            const char* sql_static_analysis_info =
                "CREATE TABLE IF NOT EXISTS binary_info ("
                "binary_path TEXT NOT NULL PRIMARY KEY, "
                "binary_digest TEXT NOT NULL, "
                "entry_point INTEGER NOT NULL, "
                "generation_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
                "FOREIGN KEY (entry_point) REFERENCES functions(id) ON DELETE CASCADE"
                ");";
                
            if (sqlite3_exec(db, sql_create_functions, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating fucntions table: " << err);

            if (sqlite3_exec(db, sql_create_basic_blocks, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating basic_blocks table: " << err);

            if (sqlite3_exec(db, sql_create_sequences, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating sequences table: " << err);

            if (sqlite3_exec(db, sql_create_exit_points, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating exit_points table: " << err);

            if (sqlite3_exec(db, sql_static_analysis_info, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating static analysis table: " << err);

            sqlite3_exec(db, "DELETE FROM binary_info;", nullptr, nullptr, &err);

            // Insert requests
            sqlite3_stmt *stmt_fn, *stmt_bb, *stmt_seq, *stmt_exit, *stmt_static_info, *stmt_up_entry;
            const char* insert_fn_sql = "INSERT OR IGNORE INTO functions (name, demangled_name) VALUES (?,?);";
            const char* insert_bb_sql = "INSERT OR IGNORE INTO basic_blocks (function, address) VALUES (?, ?);";
            const char* insert_seq_sql = "INSERT OR IGNORE INTO sequences (address, address_next, is_control) VALUES (?, ?, ?);";
            const char* insert_exit_sql = "INSERT INTO exit_points (function, address) VALUES (?, ?);";
            const char* insert_static_info_sql = "INSERT INTO binary_info (binary_path, binary_digest, entry_point) VALUES (?, ?, ?);";
            const char* update_entry_sql = "UPDATE functions SET entry=? WHERE id=?;";
    
            if (sqlite3_prepare_v2(db, insert_fn_sql, -1, &stmt_fn, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing function insert: " << sqlite3_errmsg(db));

            if (sqlite3_prepare_v2(db, insert_bb_sql, -1, &stmt_bb, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing basic_blocks insert: " << sqlite3_errmsg(db));
        
            if (sqlite3_prepare_v2(db, insert_seq_sql, -1, &stmt_seq, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing edges insert: " << sqlite3_errmsg(db));

            if (sqlite3_prepare_v2(db, insert_exit_sql, -1, &stmt_exit, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing exit_points insert: " << sqlite3_errmsg(db));

            if (sqlite3_prepare_v2(db, insert_static_info_sql, -1, &stmt_static_info, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing binary_info insert: " << sqlite3_errmsg(db));

            if(sqlite3_prepare_v2(db, update_entry_sql, -1, &stmt_up_entry, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing entry update: " << sqlite3_errmsg(db));

            /* Use an explicit transaction
            Wrapping all heavy INSERT operations inside a single BEGIN ... COMMIT block
            significantly improves performance */
            sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

            int entrypoint_id = -1;
            const char* function_name = TASK_INFO_FEATURE.get(workspace())->entryName().asNullTerminated();

            // Insert BBs first
            for(auto g: *COLLECTED_CFG_FEATURE.get(ws)) {
                int fn_id = insert_fn(g->name(), stmt_fn, db);
                if(g->name() == function_name)
                    entrypoint_id = fn_id;

                for(auto v: g->vertices()) {
                    if (v->isEntry()) {
                        const auto& succ = v->succs().begin();
                        if (!(succ.ended())) {
                            otawa::Address entry = succ.item()->toBasic()->address();
                            update_entry_addr(address_to_uint64(entry), fn_id, stmt_up_entry, db);
                        }
                    }

                    if (!v->isBasic())
                        continue;

                    auto bb = v->toBasic();
                    uint64_t bb_addr = address_to_uint64(bb->address());
                    insert_addr(bb_addr, fn_id, stmt_bb, db);

                    // Insert exit points
                    if (g->name() == TASK_INFO_FEATURE.get(workspace())->entryName()) {
                        for(auto e: v->outEdges()) {
                            auto snk = e->sink();
                            // Exit addr
                            if (snk->isExit()) {
                                uint64_t exit_bb_addr = address_to_uint64(v->toBasic()->last()->address());
                                insert_exit(exit_bb_addr, fn_id, stmt_exit, db);
                            }
                        }
                    }
                }
            }
            for(auto g: *COLLECTED_CFG_FEATURE.get(ws)) {
                for(auto v: g->vertices()) {
                    if (!v->isBasic())
                        continue;

                    auto bb = v->toBasic();
                    uint64_t bb_addr = address_to_uint64(bb->address());

                    for(otawa::Block *succ : v->succs()) {
                        if(succ->isCall())
                            succ = *(succ->succs().begin());
                        else if(!succ->isBasic())
                            continue;
                        uint64_t next_bb_addr =  address_to_uint64(succ->toBasic()->address());
                        insert_sequence(bb_addr, next_bb_addr, bb->control(), stmt_seq, db);
                    }
                }
            }
           
            // Insert binary info ...
            auto program = workspace()->process()->program()->name();
            const char* bin_path = program.chars();
            // Calculate checksum
            checksum::MD5 md5;
            io::InStream *in = sys::System::readFile(program);
            StringBuffer out;
            md5.put(*in);
            delete in;
            md5.print(out);
            auto s_checksum = out.toString();
            const char* checksum = s_checksum.asNullTerminated();            
            
            // Insert binary infos
            sqlite3_bind_text(stmt_static_info, 1, bin_path, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt_static_info, 2, checksum, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(stmt_static_info, 3, entrypoint_id);

            if (sqlite3_step(stmt_static_info) != SQLITE_DONE)
                throw ProcessorException(*this, _ << "Error inserting binary_info: " << sqlite3_errmsg(db));
            
            // Commit
            sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);

            // Clean up
            sqlite3_finalize(stmt_bb);
            sqlite3_finalize(stmt_seq);
            sqlite3_finalize(stmt_exit);
            sqlite3_finalize(stmt_static_info);
            sqlite3_close(db);
        }
        catch(sys::SystemException& e) {
            throw ProcessorException(*this, _ << "cannot create " << path << ": " << e.message());
        }
    }

private:
    Path path;
    Path dbpath;
    
    uint64_t address_to_uint64(const Address& addr) {
        uint64_t val = (static_cast<uint64_t>(addr.page()) << 32) | addr.offset();
        return val;
    }

    void insert_addr(uint64_t bb, int fn_id, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int(stmt, 1, fn_id);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(bb));
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting basic_block: " << sqlite3_errmsg(db));
        sqlite3_reset(stmt);
    }

    void insert_exit(uint64_t bb, int fn_id, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int(stmt, 1, fn_id);
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(bb));
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting basic_block: " << sqlite3_errmsg(db));
        sqlite3_reset(stmt);
    }

    void insert_sequence(uint64_t src, uint64_t snk, bool is_control, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(src));
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(snk));
        sqlite3_bind_int(stmt, 3, (is_control) ? 1 : 0);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting sequence: " << sqlite3_errmsg(db));
        sqlite3_reset(stmt);
    }

    sqlite3_int64 insert_fn(const string &name, sqlite3_stmt* stmt, sqlite3* db) {
        const char *demangled_name = demangle(name).asSysString();
        sqlite3_bind_text(stmt, 1, name.asSysString(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, demangled_name, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting sequence: " << sqlite3_errmsg(db));

        sqlite3_reset(stmt);
        sqlite3_int64 id = sqlite3_last_insert_rowid(db);
        if(id != 0)
            return id;
        
        const char* sql = "SELECT id FROM functions WHERE name = ?;";
        sqlite3_stmt* stmt2;
        sqlite3_prepare_v2(db, sql, -1, &stmt2, nullptr);
        sqlite3_bind_text(stmt, 1, name.asSysString(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_ROW)
            throw ProcessorException(*this, _ << "Can't find the id of function " << name);
        id = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return id;
    }
    void update_entry_addr(uint64_t entry, size_t fn_id, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(entry));
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(fn_id));
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting sequence: " << sqlite3_errmsg(db));
    }
};


///
p::declare CFGAsSQL::reg
    = p::init("otawa::CFGAsSQL", Version(1, 0, 0))
    .require(COLLECTED_CFG_FEATURE)
    .require(TASK_INFO_FEATURE)
    .provide(CFG_DUMP_FEATURE)
    .make<CFGAsSQL>();


/**
 * Feature ensuring that the CFG representation has been exported into SQL database file.
 * 
 * Configuration:
 * * @ref CFG_DUMP_PATH
 * 
 * @ingroup cfg
 */
p::feature CFG_AS_SQL_FEATURE("otawa::CFG_AS_SQL_FEATURE", p::make<CFGAsSQL>());

/**
 * Configuration property to select where to dump the CFG DB representation;
 * If not defined, the task working directory is used.
 * 
 * Features:
 * * @ref CFG_DUMP_FEATURE_DB
 * 
 * @ingroup cfg
 */
p::id<Path> CFG_DUMP_PATH_DB("otawa::CFG_DUMP_PATH_DB");
    
} // otawa
