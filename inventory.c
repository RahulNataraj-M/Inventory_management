/*
 * Inventory Management System (C + MySQL/MariaDB)
 *
 * Console application that stores product data in the MySQL/MariaDB
 * database server shipped with XAMPP instead of a text file.
 *
 * Database: inventory_db   Table: products (see db.sql)
 *
 * Compile & run instructions: see README.md
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "database.h"

/* Global database connection used by all operations. */
static MYSQL *conn = NULL;

/* Structure to represent a product (matches the products table). */
typedef struct product_info {
    int id;
    char name[50];
    int quantity;
    float price;
} Product;

/* Prints the last server error reported on the connection. */
static void dbError(const char *op) {
    fprintf(stderr, "Database error during %s.\nMySQL Error %u: %s\n",
            op, mysql_errno(conn), mysql_error(conn));
}

/* Prints the last error reported by a prepared statement. */
static void stmtError(MYSQL_STMT *stmt, const char *op) {
    fprintf(stderr, "Database error during %s.\nMySQL Error %u: %s\n",
            op, mysql_stmt_errno(stmt), mysql_stmt_error(stmt));
}

/* Function to add a new product (INSERT with duplicate-ID check). */
void addProduct(void) {
    Product newProduct;
    char nameBuf[64];
    MYSQL_STMT *stmt;
    MYSQL_BIND param[4];

    printf("Enter Product ID: ");
    scanf("%d", &newProduct.id);

    /* Check for duplicate ID using a prepared statement */
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        dbError("initialising statement");
        return;
    }
    {
        const char *dupQ = "SELECT id FROM products WHERE id = ?";
        MYSQL_BIND dupParam;
        if (mysql_stmt_prepare(stmt, dupQ, (unsigned long)strlen(dupQ)) != 0) {
            stmtError(stmt, "prepare duplicate check");
            mysql_stmt_close(stmt);
            return;
        }
        memset(&dupParam, 0, sizeof(dupParam));
        dupParam.buffer_type = MYSQL_TYPE_LONG;
        dupParam.buffer = (char *)&newProduct.id;
        if (mysql_stmt_bind_param(stmt, &dupParam) != 0) {
            stmtError(stmt, "bind duplicate check");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute duplicate check");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_store_result(stmt);
        if (mysql_stmt_num_rows(stmt) > 0) {
            mysql_stmt_free_result(stmt);
            mysql_stmt_close(stmt);
            printf("A product with this ID already exists. Please use a unique ID.\n");
            return;
        }
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
    }

    /* Consume the newline character left by scanf */
    while (getchar() != '\n');
    printf("Enter Product Name: ");
    if (fgets(nameBuf, sizeof(nameBuf), stdin) == NULL) return;
    nameBuf[strcspn(nameBuf, "\n")] = 0;
    if (nameBuf[0] == '\0') {
        printf("Product name cannot be empty. Product not added.\n");
        return;
    }
    if (strlen(nameBuf) >= sizeof(newProduct.name)) {
        printf("Product name too long (maximum 50 characters). Product not added.\n");
        return;
    }
    strncpy(newProduct.name, nameBuf, sizeof(newProduct.name) - 1);
    newProduct.name[sizeof(newProduct.name) - 1] = '\0';

    printf("Enter Quantity: ");
    if (scanf("%d", &newProduct.quantity) != 1) {
        printf("Invalid quantity input. Product not added.\n");
        while (getchar() != '\n');
        return;
    }
    /* Prevent negative quantity */
    if (newProduct.quantity < 0) {
        printf("Quantity cannot be negative. Product not added.\n");
        return;
    }

    printf("Enter Price: ");
    if (scanf("%f", &newProduct.price) != 1) {
        printf("Invalid price input. Product not added.\n");
        while (getchar() != '\n');
        return;
    }
    /* Prevent negative/invalid price (also catches NaN) */
    if (!(newProduct.price >= 0)) {
        printf("Price cannot be negative. Product not added.\n");
        return;
    }

    /* INSERT using a prepared statement (safe for special characters) */
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        dbError("initialising statement");
        return;
    }
    {
        const char *insQ = "INSERT INTO products (id, name, quantity, price) VALUES (?, ?, ?, ?)";
        if (mysql_stmt_prepare(stmt, insQ, (unsigned long)strlen(insQ)) != 0) {
            stmtError(stmt, "prepare insert");
            mysql_stmt_close(stmt);
            return;
        }
        memset(param, 0, sizeof(param));
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (char *)&newProduct.id;
        param[1].buffer_type = MYSQL_TYPE_STRING;
        param[1].buffer = newProduct.name;
        param[1].buffer_length = (unsigned long)strlen(newProduct.name);
        param[2].buffer_type = MYSQL_TYPE_LONG;
        param[2].buffer = (char *)&newProduct.quantity;
        param[3].buffer_type = MYSQL_TYPE_FLOAT;
        param[3].buffer = (char *)&newProduct.price;

        if (mysql_stmt_bind_param(stmt, param) != 0) {
            stmtError(stmt, "bind insert");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute insert");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_close(stmt);
    }
    printf("Product added successfully!\n");
}

/* Function to view the inventory (SELECT). */
void viewInventory(void) {
    MYSQL_RES *res;
    MYSQL_ROW row;

    if (mysql_query(conn, "SELECT id, name, quantity, price FROM products ORDER BY id") != 0) {
        dbError("viewing inventory");
        return;
    }
    res = mysql_store_result(conn);
    if (res == NULL) {
        dbError("storing result");
        return;
    }

    if (mysql_num_rows(res) == 0) {
        printf("Inventory is empty.\n");
    } else {
        printf("\n--- Inventory ---\n");
        printf("%-5s %-30s %-10s %-10s\n", "ID", "Name", "Quantity", "Price");
        while ((row = mysql_fetch_row(res)) != NULL) {
            printf("%-5s %-30s %-10s %s\n", row[0], row[1], row[2], row[3]);
        }
        printf("-------------------\n");
    }
    mysql_free_result(res);
}

/* Function to update the stock of a product (UPDATE / DELETE). */
void updateStock(void) {
    int productId;
    int quantityChange;
    int currQty = 0;
    int newQty;
    MYSQL_STMT *stmt;
    MYSQL_BIND param[2];

    printf("Enter Product ID to update stock: ");
    scanf("%d", &productId);

    /* Look up the product and read its current quantity */
    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        dbError("initialising statement");
        return;
    }
    {
        const char *selQ = "SELECT quantity FROM products WHERE id = ?";
        MYSQL_BIND result;
        if (mysql_stmt_prepare(stmt, selQ, (unsigned long)strlen(selQ)) != 0) {
            stmtError(stmt, "prepare quantity lookup");
            mysql_stmt_close(stmt);
            return;
        }
        memset(param, 0, sizeof(param));
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (char *)&productId;
        if (mysql_stmt_bind_param(stmt, param) != 0) {
            stmtError(stmt, "bind quantity lookup");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute quantity lookup");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_store_result(stmt);
        if (mysql_stmt_num_rows(stmt) == 0) {
            mysql_stmt_free_result(stmt);
            mysql_stmt_close(stmt);
            printf("Product with ID %d not found.\n", productId);
            return;
        }
        memset(&result, 0, sizeof(result));
        result.buffer_type = MYSQL_TYPE_LONG;
        result.buffer = (char *)&currQty;
        mysql_stmt_bind_result(stmt, &result);
        mysql_stmt_fetch(stmt);
        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
    }

    printf("Current quantity of product %d: %d\n", productId, currQty);
    printf("Enter the quantity to add or subtract (e.g., +10 or -5): ");
    if (scanf("%d", &quantityChange) != 1) {
        printf("Invalid quantity change input. Operation cancelled.\n");
        while (getchar() != '\n');
        return;
    }

    /* Prevent negative quantity after update */
    newQty = currQty + quantityChange;
    if (newQty < 0) {
        printf("Error: Quantity cannot go negative. Operation cancelled.\n");
        return;
    }

    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        dbError("initialising statement");
        return;
    }

    if (newQty == 0) {
        /* Quantity reached zero: remove the product (preserves original behaviour) */
        const char *delQ = "DELETE FROM products WHERE id = ?";
        if (mysql_stmt_prepare(stmt, delQ, (unsigned long)strlen(delQ)) != 0) {
            stmtError(stmt, "prepare delete");
            mysql_stmt_close(stmt);
            return;
        }
        memset(param, 0, sizeof(param));
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (char *)&productId;
        if (mysql_stmt_bind_param(stmt, param) != 0) {
            stmtError(stmt, "bind delete");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute delete");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_close(stmt);
        printf("Product removed from inventory as quantity reached 0.\n");
    } else {
        const char *upQ = "UPDATE products SET quantity = ? WHERE id = ?";
        if (mysql_stmt_prepare(stmt, upQ, (unsigned long)strlen(upQ)) != 0) {
            stmtError(stmt, "prepare update");
            mysql_stmt_close(stmt);
            return;
        }
        memset(param, 0, sizeof(param));
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (char *)&newQty;
        param[1].buffer_type = MYSQL_TYPE_LONG;
        param[1].buffer = (char *)&productId;
        if (mysql_stmt_bind_param(stmt, param) != 0) {
            stmtError(stmt, "bind update");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute update");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_close(stmt);
        printf("Stock updated successfully!\n");
    }
}

/* Function to search for a product by name or ID (SELECT). */
void searchProduct(void) {
    char searchInput[50];
    char like[64];
    int idMatch;
    MYSQL_STMT *stmt;
    MYSQL_BIND param[2];

    printf("Enter Product Name or ID to search: ");
    /* Consume the newline character left by previous scanf */
    while (getchar() != '\n');
    if (fgets(searchInput, sizeof(searchInput), stdin) == NULL) return;
    searchInput[strcspn(searchInput, "\n")] = 0; /* Remove trailing newline */

    printf("\n--- Search Results ---\n");
    printf("%-5s %-30s %-10s %-10s\n", "ID", "Name", "Quantity", "Price");

    idMatch = atoi(searchInput);
    snprintf(like, sizeof(like), "%%%s%%", searchInput);

    stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        dbError("initialising statement");
        return;
    }
    {
        const char *q = "SELECT id, name, quantity, price FROM products "
                        "WHERE id = ? OR name LIKE ? ORDER BY id";
        MYSQL_BIND result[4];
        Product p;
        my_bool isNull[4];
        int found = 0;

        if (mysql_stmt_prepare(stmt, q, (unsigned long)strlen(q)) != 0) {
            stmtError(stmt, "prepare search");
            mysql_stmt_close(stmt);
            return;
        }
        memset(param, 0, sizeof(param));
        param[0].buffer_type = MYSQL_TYPE_LONG;
        param[0].buffer = (char *)&idMatch;
        param[1].buffer_type = MYSQL_TYPE_STRING;
        param[1].buffer = like;
        param[1].buffer_length = (unsigned long)strlen(like);
        if (mysql_stmt_bind_param(stmt, param) != 0) {
            stmtError(stmt, "bind search");
            mysql_stmt_close(stmt);
            return;
        }
        if (mysql_stmt_execute(stmt) != 0) {
            stmtError(stmt, "execute search");
            mysql_stmt_close(stmt);
            return;
        }
        mysql_stmt_store_result(stmt);

        memset(result, 0, sizeof(result));
        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = (char *)&p.id;
        result[1].buffer_type = MYSQL_TYPE_STRING;
        result[1].buffer = p.name;
        result[1].buffer_length = sizeof(p.name);
        result[2].buffer_type = MYSQL_TYPE_LONG;
        result[2].buffer = (char *)&p.quantity;
        result[3].buffer_type = MYSQL_TYPE_FLOAT;
        result[3].buffer = (char *)&p.price;
        for (int i = 0; i < 4; i++) result[i].is_null = &isNull[i];

        if (mysql_stmt_bind_result(stmt, result) != 0) {
            stmtError(stmt, "bind search results");
            mysql_stmt_free_result(stmt);
            mysql_stmt_close(stmt);
            return;
        }

        while (mysql_stmt_fetch(stmt) == 0) {
            printf("%-5d %-30s %-10d %.2f\n", p.id, p.name, p.quantity, p.price);
            found = 1;
        }
        if (!found) {
            printf("No products found matching your search criteria.\n");
        }
        printf("----------------------\n");

        mysql_stmt_free_result(stmt);
        mysql_stmt_close(stmt);
    }
}

int main(void) {
    int choice;

    /* Connect to the database first */
    conn = connectDatabase();
    if (conn == NULL) {
        fprintf(stderr, "Exiting: could not connect to the MySQL server.\n");
        return 1;
    }

    do {
        printf("\n--- Inventory Management System ---\n");
        printf("1. Add Product\n");
        printf("2. View Inventory\n");
        printf("3. Update Stock\n");
        printf("4. Search Product\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                addProduct();
                break;
            case 2:
                viewInventory();
                break;
            case 3:
                updateStock();
                break;
            case 4:
                searchProduct();
                break;
            case 0:
                printf("Exiting program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 0);

    closeDatabase(conn);
    return 0;
}