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

namespace otawa {

class CFGAsSQL: public Processor {
public:
    static p::declare reg;
    CFGAsSQL(p::declare& r = reg): Processor(r) {}

protected:
    
    void configure(const PropList& props) override {
        Processor::configure(props);
        path = CFG_DUMP_PATH(props); 
    }
    
    void processWorkSpace(WorkSpace * ws) override {
        
        // compute path
        Path path = this->path;
        if(path.isEmpty())
            path = TASK_INFO_FEATURE.get(ws)->workDirectory();
        path = path / "cfg.db";
        
        try {
            sqlite3* db = nullptr;
            char* err = nullptr;
            
            // Open / create the db
            if (sqlite3_open(path.toString().asSysString(), &db) != SQLITE_OK)
                throw ProcessorException(*this, _ <<  "Error opening db: " << sqlite3_errmsg(db));

            // Enable foreign key contraint
            sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
        
            // Create tables
            const char* sql_create_basic_blocks =
                "CREATE TABLE IF NOT EXISTS basic_blocks ("
                "address INTEGER PRIMARY KEY"
                ");";

            const char* sql_create_sequences = 
                "CREATE TABLE IF NOT EXISTS sequences ("
                "address INTEGER UNIQUE, "
                "address_next INTEGER UNIQUE, "
                "FOREIGN KEY (address) REFERENCES basic_blocks(address), "
                "FOREIGN KEY (address_next) REFERENCES basic_blocks(address)"
                ");";

            const char* sql_create_exit_points =
                "CREATE TABLE IF NOT EXISTS exit_points ("
                "address INTEGER"
                ");";

            const char* sql_static_analysis_info =
                "CREATE TABLE IF NOT EXISTS binary_info ("
                "binary_path TEXT NOT NULL PRIMARY KEY, "
                "binary_digest TEXT NOT NULL, "
                "function_name TEXT, "
                "entry_point INTEGER NOT NULL, "
                "generation_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
                "FOREIGN KEY (entry_point) REFERENCES basic_blocks(address)"
                ");";

            if (sqlite3_exec(db, sql_create_basic_blocks, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating basic_blocks table: " << err);

            if (sqlite3_exec(db, sql_create_sequences, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating sequences table: " << err);

            if (sqlite3_exec(db, sql_create_exit_points, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating exit_points table: " << err);

            if (sqlite3_exec(db, sql_static_analysis_info, nullptr, nullptr, &err) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error creating static analysis table: " << err);

            // Insert requests
            sqlite3_stmt* stmt_bb;
            sqlite3_stmt* stmt_seq;
            sqlite3_stmt* stmt_exit;
            sqlite3_stmt* stmt_static_info;
            const char* insert_bb_sql = "INSERT OR IGNORE INTO basic_blocks (address) VALUES (?);";
            const char* insert_seq_sql = "INSERT OR IGNORE INTO sequences (address, address_next) VALUES (?, ?);";
            const char* insert_exit_sql = "INSERT INTO exit_points (address) VALUES (?);";
            const char* insert_static_info_sql = 
                "INSERT OR REPLACE INTO binary_info "
                "(binary_path, binary_digest, function_name, entry_point) "
                "VALUES (?, ?, ?, ?);";
    
            if (sqlite3_prepare_v2(db, insert_bb_sql, -1, &stmt_bb, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing basic_blocks insert: " << sqlite3_errmsg(db));
        
            if (sqlite3_prepare_v2(db, insert_seq_sql, -1, &stmt_seq, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing edges insert: " << sqlite3_errmsg(db));

            if (sqlite3_prepare_v2(db, insert_exit_sql, -1, &stmt_exit, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing exit_points insert: " << sqlite3_errmsg(db));

            if (sqlite3_prepare_v2(db, insert_static_info_sql, -1, &stmt_static_info, nullptr) != SQLITE_OK)
                throw ProcessorException(*this, _ << "Error preparing binary_info insert: " << sqlite3_errmsg(db));
            
            /* Use an explicit transaction
            Wrapping all heavy INSERT operations inside a single BEGIN ... COMMIT block
            significantly improves performance */
            sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
            
            // Insert BBs first
            for(auto g: *COLLECTED_CFG_FEATURE.get(ws)) {
                for(auto v: g->vertices()) {
                    if (!v->isBasic())
                        continue;

                    auto bb = v->toBasic();
                    uint64_t bb_addr = address_to_uint64(bb->address());
                    
                    insert_addr(bb_addr, stmt_bb, db);

                    // Insert seq
                    if (bb->control() == nullptr) {
                        uint64_t next_bb_addr =  address_to_uint64(bb->last()->nextInst()->address());
                        // Insert (or ignore) next BB first
                        insert_addr(next_bb_addr, stmt_bb, db);
                        
                        // Now insert seq
                        insert_sequence(bb_addr, next_bb_addr, stmt_seq, db);
                    }

                    // Insert exit points
                    if (g->name() == TASK_INFO_FEATURE.get(workspace())->entryName()) {
                        for(auto e: v->outEdges()) {
                            auto snk = e->sink();
                            // Exit addr
                            if (snk->isExit()) {
                                uint64_t exit_bb_addr = address_to_uint64(v->toBasic()->last()->address());
                                insert_addr(exit_bb_addr, stmt_exit, db);
                            }
                        }
                    }
                }
            }
           
            // Insert binary info ...
            auto program = workspace()->process()->program()->name();
            const char* bin_path = program.chars();
            const char* function_name = TASK_INFO_FEATURE.get(workspace())->entryName().asNullTerminated();
            uint64_t entry_point = address_to_uint64(TASK_INFO_FEATURE.get(workspace())->entryInst()->address());
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
            sqlite3_bind_text(stmt_static_info, 3, function_name, -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt_static_info, 4, static_cast<sqlite3_int64>(entry_point));

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
    
    uint64_t address_to_uint64(const Address& addr) {
        uint64_t val = (static_cast<uint64_t>(addr.page()) << 32) | addr.offset();
        return val;
    }

    void insert_addr(uint64_t bb, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(bb));
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting basic_block: " << sqlite3_errmsg(db));
        sqlite3_reset(stmt);
    }

    void insert_sequence(uint64_t src, uint64_t snk, sqlite3_stmt* stmt, sqlite3* db) {
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(src));
        sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(snk));
        if (sqlite3_step(stmt) != SQLITE_DONE)
            throw ProcessorException(*this, _ << "Error inserting sequence: " << sqlite3_errmsg(db));
        sqlite3_reset(stmt);
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
    
} // otawa
