#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_PRODUCTS 100
#define INVENTORY_FILE "inventory.txt"
// Structure to represent a product
typedef struct product_info{
    int id;
    char name[50];
    int quantity;
    float price;
} Product;
// Array to store the products
Product inventory[MAX_PRODUCTS];
int productCount = 0;
// Function to load inventory from file
void loadInventory() {
    FILE *file = fopen(INVENTORY_FILE, "r");
    productCount = 0;
    if (file != NULL) {
        while (fscanf(file, "%d,%49[^,],%d,%f\n",
                      &inventory[productCount].id,
                      inventory[productCount].name,
                      &inventory[productCount].quantity,
                      &inventory[productCount].price) == 4) {
            productCount++;
            if (productCount >= MAX_PRODUCTS) break;
        }
        fclose(file);
    }
}
// Function to save inventory to file
void saveInventory() {
    FILE *file = fopen(INVENTORY_FILE, "w");
    if (file != NULL) {
        for (int i = 0; i < productCount; i++) {
            fprintf(file, "%d,%s,%d,%.2f\n",
                    inventory[i].id,
                    inventory[i].name,
                    inventory[i].quantity,
                    inventory[i].price);
        }
        fclose(file);
    }
}
// Function to add a new product
void addProduct() {
    if (productCount < MAX_PRODUCTS) {
        Product newProduct;
        printf("Enter Product ID: ");
        scanf("%d", &newProduct.id);
        // Check for duplicate ID
        for (int i = 0; i < productCount; i++) {
            if (inventory[i].id == newProduct.id) {
                printf("A product with this ID already exists. Please use a unique ID.\n");
                return;
            }
        }
        // Consume the newline character left by scanf
        while (getchar() != '\n');
        printf("Enter Product Name: ");
        fgets(newProduct.name, sizeof(newProduct.name), stdin);
        newProduct.name[strcspn(newProduct.name, "\n")] = 0;
        printf("Enter Quantity: ");
        scanf("%d", &newProduct.quantity);
        // Prevent negative quantity
        if (newProduct.quantity < 0) {
            printf("Quantity cannot be negative. Product not added.\n");
            return;
        }
        printf("Enter Price: ");
        scanf("%f", &newProduct.price);
        inventory[productCount++] = newProduct;
        saveInventory();
        printf("Product added successfully!\n");
    } else {
        printf("Inventory is full. Cannot add more products.\n");
    }
}
// Function to view the inventory
void viewInventory() {
    loadInventory();
    if (productCount > 0) {
        printf("\n--- Inventory ---\n");
        printf("%-5s %-30s %-10s %-10s\n", "ID", "Name", "Quantity", "Price");
        for (int i = 0; i < productCount; i++) {
            printf("%-5d %-30s %-10d %.2f\n", inventory[i].id, inventory[i].name,
                   inventory[i].quantity, inventory[i].price);
        }
        printf("-------------------\n");
    } else {
        printf("Inventory is empty.\n");
    }
}
// Function to update the stock of a product
void updateStock() {
    int productId;
    int quantityChange;
    loadInventory();
    printf("Enter Product ID to update stock: ");
    scanf("%d", &productId);
    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == productId) {
            printf("Enter the quantity to add or subtract (e.g., +10 or -5): ");
            scanf("%d", &quantityChange);
            // Prevent negative quantity after update
            if (inventory[i].quantity + quantityChange < 0) {
                printf("Error: Quantity cannot go negative. Operation cancelled.\n");
                found = 1;
                break;
            }
            inventory[i].quantity += quantityChange;
            if (inventory[i].quantity == 0) {
                // Remove product by shifting the rest left
                for (int j = i; j < productCount - 1; j++) {
                    inventory[j] = inventory[j + 1];
                }
                productCount--;
                saveInventory();
                printf("Product removed from inventory as quantity reached 0.\n");
            } else {
                saveInventory();
                printf("Stock updated successfully!\n");
            }
            found = 1;
            break;
        }
    }
    if (!found) {
        printf("Product with ID %d not found.\n", productId);
    }
}
// Function to search for a product by name or ID
void searchProduct() {
    char searchInput[50];
    loadInventory();
    printf("Enter Product Name or ID to search: ");
    // Consume the newline character left by previous scanf
    while (getchar() != '\n');
    fgets(searchInput, sizeof(searchInput), stdin);
    searchInput[strcspn(searchInput, "\n")] = 0; // Remove trailing newline
    printf("\n--- Search Results ---\n");
    printf("%-5s %-30s %-10s %-10s\n", "ID", "Name", "Quantity", "Price");
    int found = 0;
    for (int i = 0; i < productCount; i++) {
        if (inventory[i].id == atoi(searchInput) || strstr(inventory[i].name,
            searchInput) != NULL) {
            printf("%-5d %-30s %-10d %.2f\n", inventory[i].id, inventory[i].name,
                   inventory[i].quantity, inventory[i].price);
            found = 1;
        }
    }
    if (!found) {
        printf("No products found matching your search criteria.\n");
    }
    printf("----------------------\n");
}
int main() {
    int choice;
    loadInventory();
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
    return 0;
}