#include <sqlite3.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include "sha256.h"

#define DB_NAME "/oem/db/lpr.db"

using namespace std;

static int callback(
    void *NotUsed,
    int argc,
    char **argv, 
    char **azColName)
{    
    NotUsed = 0;
    
    for (int i = 0; i < argc; i++)
    {
        printf("[%d]%s = %s\n", i, azColName[i], argv[i] ? argv[i] : "NULL");
    }
    
    printf("(%d)\n", (int*)NotUsed);
    
    return 0;
}

static int callbackCount(
    void *count, 
    int argc, 
    char **argv, 
    char **azColName) 
{
    int *c = (int*)count;
    *c = atoi(argv[0]);

    return 0;
}

int createTable()
{
    return 0;
}

int getCount(sqlite3 *db)
{
    int count = 0;
    char *err_msg = 0;

    char *sql = "SELECT COUNT(*) from wanted";

    int rc = sqlite3_exec(db, sql, callbackCount, &count, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("count: %d\n", count);
    }

    return count;
}

int getCarInfo(sqlite3 *db)
{
    char *err_msg = 0;

    char *sql = "SELECT * from wanted where no='FFFE592A18CD085ADDD2C69A737DEC000F46F4AEB16C264B9ADB349822467442'";

    int rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if (rc != SQLITE_OK) {

        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);

    }

    return 0;
}

int createDB(char *dbName)
{
    sqlite3 *db;
    char *sql = 0;
    char *err_msg = 0;
    sqlite3_stmt *res;

    int rc = sqlite3_open(dbName, &db);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    // create table
    // tbName = wanted
    sql = "CREATE TABLE wanted (carNo VCHAR(64), code VCHAR(8), reserved VCHAR(8));";
    rc = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    // close db
    sqlite3_close(db);

    return 0;
}

int deleteDB(char *dbName)
{
    remove(dbName);
    return 0;
}


int main(void)
{
    printf("####################################################################\n"); 
    printf("sqlite3 version = %s\n", sqlite3_libversion()); 
    printf("####################################################################\n"); 

    int rc;
    sqlite3 *db;
    char *err_msg = 0;
    sqlite3_stmt *res;

    // 0. check file existence (SQLITE_OK = 0)
    //rc = createDB(DB_NAME);
    //rc = deleteDB(DB_NAME);

    rc = sqlite3_open(DB_NAME, &db);    // 
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        
        return 1;
    }

    if( getCount(db) <= 0)
    {
        fprintf(stderr, "DB Empty!!!\n");

        sqlite3_free(err_msg);
        sqlite3_close(db);

        return 1;
    }

    getCarInfo(db);

    string input = "998가4568";
    string output1 = sha256(input);
 
    cout << "sha256('"<< input << "'):" << output1 << endl;

#if false
    char *sql = "SELECT * FROM wanted where code='7'";
    rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    if (rc != SQLITE_OK )
    {
        fprintf(stderr, "Failed to select data\n");
        fprintf(stderr, "SQL error: %s\n", err_msg);

        sqlite3_free(err_msg);
        sqlite3_close(db);
        
        return 1;
    } 
#endif

    sqlite3_close(db);
    
    return 0;
}
