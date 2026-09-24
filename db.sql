-- Inventory Management System - MySQL database setup
-- Run with:  "D:\Xampp\mysql\bin\mysql.exe" -u root < db.sql
-- (or paste into phpMyAdmin SQL tab)

CREATE DATABASE IF NOT EXISTS inventory_db
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

USE inventory_db;

CREATE TABLE IF NOT EXISTS products (
    id INT PRIMARY KEY,
    name VARCHAR(50) NOT NULL,
    quantity INT NOT NULL DEFAULT 0,
    price DECIMAL(10,2) NOT NULL DEFAULT 0.00
);

-- Optional sample data:
-- INSERT INTO products (id, name, quantity, price) VALUES
--     (1, 'Laptop', 10, 499.99),
--     (101, 'Wireless Mouse', 25, 12.50);