#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sqlite3.h>


int sqlite3_run_query(sqlite3 *db, sqlite3_stmt *stmt, unsigned retries) {
    int src = SQLITE_BUSY;
    for (unsigned i = 0; i < retries; i++) {
        src = sqlite3_step(stmt);
        if (src != SQLITE_BUSY && src != SQLITE_LOCKED) {
            break;
        }
        usleep(200);
    }
    if ((src != SQLITE_DONE) && (src != SQLITE_ROW)) {
        fprintf(stderr, "sqlite3_run_query failed:%s: %s\n", sqlite3_sql(stmt), sqlite3_errmsg(db));
    }
    return src;
}

int
sqlite3_prepare_statement(sqlite3 *db, sqlite3_stmt **stmt, const char *query, unsigned retries) {
    int src = SQLITE_BUSY;
    for (unsigned i = 0; i < retries; i++) {
        // https://www.sqlite.org/c3ref/prepare.html
        src = sqlite3_prepare_v2(db, query, (int) strlen(query), stmt, NULL);
        if (src != SQLITE_BUSY && src != SQLITE_LOCKED) {
            break;
        }
        usleep(200);
    }
    if (src) {
        fprintf(stderr, "sqlite3_prepare_v2 failed for \"%s\": %s\n", query, sqlite3_errmsg(db));
        sqlite3_finalize(*stmt);
    }
    return src;
}

int main() {
    return 0;
}