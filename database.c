#include <stdio.h>
#include "database.h"

/* Opens a connection to the MySQL/MariaDB server running in XAMPP. */
MYSQL *connectDatabase(void) {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed: unable to allocate connection.\n");
        return NULL;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed.\nMySQL Error %u: %s\n",
                mysql_errno(conn), mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    printf("Connected to MySQL database '%s' at %s:%d\n", DB_NAME, DB_HOST, DB_PORT);
    return conn;
}

/* Closes the database connection. */
void closeDatabase(MYSQL *conn) {
    if (conn != NULL) {
        mysql_close(conn);
        printf("Database connection closed.\n");
    }
}