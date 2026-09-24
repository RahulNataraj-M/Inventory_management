#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>

/* ---- Database credentials (edit here if needed) ----
   Default XAMPP MySQL/MariaDB: user root, empty password, port 3306. */
#define DB_HOST     "localhost"
#define DB_PORT     3306
#define DB_USER     "root"
#define DB_PASS     ""
#define DB_NAME     "inventory_db"

/* Opens a connection to the MySQL database.
   Returns a valid MYSQL* on success, NULL on failure. */
MYSQL *connectDatabase(void);

/* Closes the database connection. */
void closeDatabase(MYSQL *conn);

#endif /* DATABASE_H */