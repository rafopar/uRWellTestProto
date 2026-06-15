#include "TranslationTable.h"
#include "sqlite3.h"

#include <iostream>
#include <vector>
#include <sstream>

namespace {

// Strip a CCDB connection string down to the sqlite file path.
std::string sqlitePath(const std::string &conn) {
    const std::string p = "sqlite://";
    if (conn.rfind(p, 0) == 0) {
        std::string rest = conn.substr(p.size());      // e.g. "//abs/path"
        while (rest.size() > 1 && rest[0] == '/' && rest[1] == '/') rest.erase(0, 1);
        return rest;                                    // -> "/abs/path"
    }
    return conn;
}

// Split "/daq/tt/clasdev/Hodo" -> dirs {daq,tt,clasdev} + table "Hodo".
void splitPath(const std::string &path, std::vector<std::string> &dirs,
               std::string &table) {
    std::vector<std::string> parts;
    std::stringstream ss(path);
    std::string item;
    while (std::getline(ss, item, '/'))
        if (!item.empty()) parts.push_back(item);
    if (parts.empty()) { table.clear(); return; }
    table = parts.back();
    dirs.assign(parts.begin(), parts.end() - 1);
}

// Run a query returning a single int; returns true on success.
bool queryInt(sqlite3 *db, const std::string &sql, int &out) {
    sqlite3_stmt *st = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &st, nullptr) != SQLITE_OK) return false;
    bool ok = false;
    if (sqlite3_step(st) == SQLITE_ROW) { out = sqlite3_column_int(st, 0); ok = true; }
    sqlite3_finalize(st);
    return ok;
}

} // namespace

bool TranslationTable::load(const std::string &connection,
                            const std::string &tablePath, int run,
                            const std::string &variation) {
    fMap.clear();

    const std::string file = sqlitePath(connection);
    sqlite3 *db = nullptr;
    if (sqlite3_open_v2(file.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        std::cerr << "TranslationTable: cannot open " << file << ": "
                  << sqlite3_errmsg(db) << "\n";
        sqlite3_close(db);
        return false;
    }

    std::vector<std::string> dirs;
    std::string table;
    splitPath(tablePath, dirs, table);

    // 1) Walk the directory tree (parentId 0 is root) to the table's directory.
    int parentId = 0;
    bool dirOk = true;
    for (const std::string &d : dirs) {
        std::ostringstream q;
        q << "SELECT id FROM directories WHERE parentId=" << parentId
          << " AND name='" << d << "'";
        int id;
        if (!queryInt(db, q.str(), id)) { dirOk = false; break; }
        parentId = id;
    }
    int typeId = -1;
    if (dirOk) {
        std::ostringstream q;
        q << "SELECT id FROM typeTables WHERE directoryId=" << parentId
          << " AND name='" << table << "'";
        queryInt(db, q.str(), typeId);
    }
    if (typeId < 0) {
        std::cerr << "TranslationTable: table not found: " << tablePath << "\n";
        sqlite3_close(db);
        return false;
    }

    // 2) Resolve column order by name (robust to schema changes).
    int cCrate=-1, cSlot=-1, cChan=-1, cSec=-1, cLay=-1, cComp=-1, cOrder=-1, nCol=0;
    {
        std::ostringstream q;
        q << "SELECT name FROM columns WHERE typeId=" << typeId << " ORDER BY \"order\"";
        sqlite3_stmt *st = nullptr;
        sqlite3_prepare_v2(db, q.str().c_str(), -1, &st, nullptr);
        while (sqlite3_step(st) == SQLITE_ROW) {
            std::string name = reinterpret_cast<const char *>(sqlite3_column_text(st, 0));
            if      (name == "crate")     cCrate = nCol;
            else if (name == "slot")      cSlot  = nCol;
            else if (name == "chan" ||
                     name == "channel")   cChan  = nCol;
            else if (name == "sector")    cSec   = nCol;
            else if (name == "layer")     cLay   = nCol;
            else if (name == "component") cComp  = nCol;
            else if (name == "order")     cOrder = nCol;
            nCol++;
        }
        sqlite3_finalize(st);
    }
    if (cCrate<0||cSlot<0||cChan<0||cSec<0||cLay<0||cComp<0||cOrder<0) {
        std::cerr << "TranslationTable: unexpected columns in " << tablePath << "\n";
        sqlite3_close(db);
        return false;
    }

    // 3) variation id.
    int varId = -1;
    {
        std::ostringstream q;
        q << "SELECT id FROM variations WHERE name='" << variation << "'";
        if (!queryInt(db, q.str(), varId)) {
            std::cerr << "TranslationTable: variation not found: " << variation << "\n";
            sqlite3_close(db);
            return false;
        }
    }

    // 4) Pick the most recent assignment whose run range covers `run`.
    int constSetId = -1;
    {
        std::ostringstream q;
        q << "SELECT a.constantSetId FROM assignments a "
             "JOIN constantSets cs ON cs.id=a.constantSetId "
             "JOIN runRanges rr ON rr.id=a.runRangeId "
          << "WHERE cs.constantTypeId=" << typeId
          << " AND a.variationId=" << varId
          << " AND rr.runMin<=" << run << " AND rr.runMax>=" << run
          << " ORDER BY a.created DESC, a.id DESC LIMIT 1";
        if (!queryInt(db, q.str(), constSetId)) {
            std::cerr << "TranslationTable: no assignment for run " << run
                      << " in " << tablePath << "\n";
            sqlite3_close(db);
            return false;
        }
    }

    // 5) Load and parse the constant-set vault ('|'-separated cells, row-major).
    std::string vault;
    {
        std::ostringstream q;
        q << "SELECT vault FROM constantSets WHERE id=" << constSetId;
        sqlite3_stmt *st = nullptr;
        sqlite3_prepare_v2(db, q.str().c_str(), -1, &st, nullptr);
        if (sqlite3_step(st) == SQLITE_ROW)
            vault = reinterpret_cast<const char *>(sqlite3_column_text(st, 0));
        sqlite3_finalize(st);
    }
    sqlite3_close(db);

    std::vector<std::string> cells;
    {
        std::stringstream ss(vault);
        std::string c;
        while (std::getline(ss, c, '|')) cells.push_back(c);
    }
    if (nCol == 0 || cells.size() % nCol != 0) {
        std::cerr << "TranslationTable: vault size " << cells.size()
                  << " not a multiple of " << nCol << " for " << tablePath << "\n";
        return false;
    }

    for (size_t r = 0; r + nCol <= cells.size(); r += nCol) {
        int crate = std::stoi(cells[r + cCrate]);
        int slot  = std::stoi(cells[r + cSlot]);
        int chan  = std::stoi(cells[r + cChan]);
        Entry e;
        e.sector    = std::stoi(cells[r + cSec]);
        e.layer     = std::stoi(cells[r + cLay]);
        e.component = std::stoi(cells[r + cComp]);
        e.order     = std::stoi(cells[r + cOrder]);
        fMap[key(crate, slot, chan)] = e;
    }
    return true;
}
