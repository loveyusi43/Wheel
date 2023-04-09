#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <unistd.h>

bool GetQueryString(std::string &query_string);

bool InsertSQL(std::string sql);

int main(void){
    std::string query_string;
    if (GetQueryString(query_string)) {
        std::string sql;
        InsertSQL(sql);
    }
    return 0;
}

bool InsertSQL(std::string sql) {
    MYSQL *conn = mysql_init(nullptr);
    mysql_set_character_set(conn, "utf8");

    const char* host = "43.143.4.79";
    const char* user = "loveyusi";
    const char* password = "3502";
    const char* db = "httpdb";
    unsigned int port = 3306;

    if (mysql_real_connect(conn, host, user, password, db, port, nullptr, 0) == nullptr) {
        std::cerr << "connect error\n";
        return false;
    }
    std::cerr << "connect success\n";
    std::cerr << sql << std::endl;
    int ret = mysql_query(conn, sql.c_str());
    std::cerr << ret << std::endl;

    mysql_close(conn);
    return true;
}

bool GetQueryString(std::string &query_string) {
    std::string method = getenv("METHOD");

    if (method == "GET") {
        query_string = getenv("QUERY_STRING");
        return true;
    }else if (method == "POST") {
        int content_length = std::atoi(getenv("CONTENT_LENGTH"));
        char c = 0;
        while (content_length--) {
            read(0, &c, 1);
            query_string += c;
        }
        return true;
    }else{
        return false;
    }
}