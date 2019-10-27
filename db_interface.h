//
// Created by gregkwaste on 5/6/19.
//
#ifndef CRF_RESCHEDULING_DB_INTERFACE_H
#define CRF_RESCHEDULING_DB_INTERFACE_H

#include <mysql/mysql.h>
#include <assert.h>
#include <stdio.h>
#include <cstdio>
#include <iostream>

using namespace std;


//DB Settings Struct
struct database_settings {
    string hostname;
    string user;
    string pass;
    unsigned int port;
    string db_name;
};

void disconnect_from_db(MYSQL m){
    mysql_close(&m);
    //Cleanup mysql elements
}


//DB Connection Struct
struct database_connection {
    MYSQL m;
    MYSQL_RES *result;

    void free_result(){
        mysql_free_result(result);
    }

    void disconnect(){
        disconnect_from_db(m);
    }
};

//-------------------------------------
//-------------------------------------
//DB CONNECTION/DISCONNECTION/QUERIES PARSERS

int connect_to_db(const database_settings* db_settings, database_connection *conn){
    //Return Statuses
    //1: All Good
    //0: Connection Failed
    //-1: Error - Not handled for now

    //Initiate connection to database
    cout << "SQL Report: client version: "<<mysql_get_client_info()<<endl;
    mysql_init(&conn->m);
    if (!mysql_real_connect(&conn->m, db_settings->hostname.c_str(),
                            db_settings->user.c_str(), db_settings->pass.c_str(),
                            db_settings->db_name.c_str(), db_settings->port, nullptr, 0)){
        cerr << mysql_error(&conn->m) << endl;
        printf("Connection failed\n");
        return 0;
    }

    printf("Connection Successful\n");
    return 1;
}



void execute_query_fast(database_connection& db_conn, const char* query)
{
    if(mysql_query(&db_conn.m, query) == 0){
        db_conn.result = mysql_store_result(&db_conn.m);
    }
    else {
        printf("%d %s\n", mysql_errno(&db_conn.m), mysql_error(&db_conn.m));
        //cout << query;
        assert(false);
    }
}



void execute_query_fast(database_connection& db_conn, const string& query)
{
    if(mysql_query(&db_conn.m, query.c_str()) == 0){
        db_conn.result = mysql_store_result(&db_conn.m);
    }
    else {
        printf("%d %s\n", mysql_errno(&db_conn.m), mysql_error(&db_conn.m));
        cout << query;
        assert(false);
    }
}

void execute_query(database_connection& db_conn, const string& query, const string& success_string)
{
    if(mysql_query(&db_conn.m, query.c_str()) == 0){
        cout << success_string << endl;
        db_conn.result = mysql_store_result(&db_conn.m);
    }
    else {
        printf("%d %s\n", mysql_errno(&db_conn.m), mysql_error(&db_conn.m));
        cerr << "Query failed: "<<  mysql_error(&db_conn.m) << endl;
        cout << query;
        assert(false);
    }
}

void execute_query(database_connection& db_conn, stringstream& query)
{
    return execute_query(db_conn, query.str(), "SUCCESS");
}

void execute_query(database_connection *db_conn, stringstream& query)
{
    return execute_query_fast(*db_conn, query.str());
}

void execute_query(database_connection *db_conn, char* query)
{
    return execute_query_fast(*db_conn, query);
}



#endif //CRF_RESCHEDULING_DB_INTERFACE_H
