# Inventory Management System (C + MySQL/MariaDB via XAMPP)

Console-based inventory app in C. Product data is stored in the
MySQL/MariaDB server that ships with XAMPP, replacing the old `inventory.txt`
file storage entirely. The original file-based version is kept in
`inventory_file_version.c`.

## Files

| File              | Purpose                                              |
|-------------------|------------------------------------------------------|
| `inventory.c`     | Main menu + Add / View / Update / Search operations  |
| `database.h`      | Connection credentials (edit here) and prototypes    |
| `database.c`      | `connectDatabase()` / `closeDatabase()`              |
| `db.sql`          | Creates `inventory_db` database and `products` table |
| `inventory_file_version.c` | Original `inventory.txt` version (backup)    |

## 1. Start the database

1. Open XAMPP Control Panel and start **MySQL**.
2. Confirm it is listening on port **3306** (`D:\Xampp\mysql\bin` = MySQL/MariaDB).

Credentials used by the app (defaults, edit `database.h`):

- Host: `localhost`, Port: `3306`, User: `root`, Password: *(empty)*, Database: `inventory_db`

XAMPP's MySQL server is actually **MariaDB**, which is fully compatible with the
MySQL C Client API used below.

## 2. Create the database and table

```bat
"D:\Xampp\mysql\bin\mysql.exe" -u root < db.sql
```

or paste the contents of `db.sql` into phpMyAdmin's SQL tab.

## 3. Required MySQL C client library

XAMPP does **not** include the C client headers/DLL. You need
**MySQL Connector/C**. For Windows x64, download:

- **MySQL Connector/C 6.1.11 (winx64)** (compatible):  
  https://cdn.mysql.com/Downloads/Connector-C/mysql-connector-c-6.1.11-winx64.zip

Extract it to this folder so the structure is:

```
D:\Inventory Management System\mysql-connector-c-6.1.11-winx64\
  include\mysql.h
  lib\libmysql.lib
  lib\libmysql.dll
```

(If you use a different version, update the paths in `build.bat`.)

## 4. Building the `.exe`

Run:

```bat
build.bat
```

Requirements: **Visual Studio Build Tools 2019+** with MSVC (`cl.exe`) must be installed.
`build.bat` will:

- call `vcvars64.bat` to set up the compiler,
- compile `inventory.c` and `database.c`,
- link against `mysql-connector-c-6.1.11-winx64\lib\libmysql.lib`,
- copy `mysql-connector-c-6.1.11-winx64\lib\libmysql.dll` to this folder as `libmysql.dll`,
- create `inventory.exe`.

MinGW/GCC alternative (if available):

```bat
gcc -o inventory.exe inventory.c database.c ^
  -I mysql-connector-c-6.1.11-winx64\include ^
  -L mysql-connector-c-6.1.11-winx64\lib -lmysql
```

## 5. Running the program

1. Start **XAMPP Control Panel** → Start **MySQL** (should listen on `localhost:3306`).
2. Create the database/table: run `db.sql` once (e.g. `D:\Xampp\mysql\bin\mysql.exe -u root < db.sql` or phpMyAdmin).
3. Ensure `inventory.exe` and `libmysql.dll` are in the same folder.
4. Run from CMD in this folder:

```bat
inventory.exe
```

The program connects to `inventory_db` at `localhost:3306` (user `root`, no password). You can change credentials in `database.h`.

## 6. Menu (unchanged)

```
--- Inventory Management System ---
1. Add Product
2. View Inventory
3. Update Stock
4. Search Product
0. Exit
```

All operations use prepared statements (SQL-injection-safe), including the
duplicate-ID check, `INSERT`, stock `UPDATE`, `DELETE` when quantity reaches
zero, and `SELECT`-based search with partial name matching.