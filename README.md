# 💰 C++ Console-Based Banking System (MySQL)

A simple, secure, and functional **console-based banking system** developed in C++ with **MySQL database integration**. This project demonstrates CRUD operations, transactions, and basic banking features such as deposits, withdrawals, fund transfers, and account management.

---

## 📌 Features

- Create new bank accounts
- Deposit and withdraw funds
- Modify or delete accounts
- View all customer accounts
- Transfer money between accounts
- Safe transaction handling with rollback on failure
- MySQL database connectivity using `MySQL Connector/C++`

---

## 🔧 Technologies Used

- C++
- MySQL Server
- MySQL Connector/C++ (C++ API for MySQL)
- Standard C++ Libraries (I/O, String, etc.)

---

## 📂 Project Structure

/banking-system
├── main.cpp # Main application source code
├── README.md # Project documentation

---

## 🗃️ Database Setup

Make sure you have **MySQL Server** running locally.

1. Open your MySQL client (like MySQL Workbench or terminal).
2. Run the following SQL:

```sql
CREATE DATABASE IF NOT EXISTS banking;
USE banking;

CREATE TABLE IF NOT EXISTS accounts (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(100),
    balance DOUBLE
);
