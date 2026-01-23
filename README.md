# 🐱 KittyDB

<div align="center">

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=cplusplus)](https://isocpp.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)
[![Status](https://img.shields.io/badge/Status-Complete-brightgreen?style=flat-square)](https://github.com)
[![Build](https://img.shields.io/badge/Build-Passing-brightgreen?style=flat-square)](https://github.com)

**A lightweight, file-based relational database engine built from scratch in C++**

A complete SQL database implementation showcasing database internals, custom parsing, and query execution.

[Features](#-features) • [Quick Start](#-quick-start) • [Examples](#-usage-examples) • [Architecture](#-architecture) • [Documentation](#-supported-sql-syntax)

</div>

---

## ✨ Features

### 🗄️ Core Database Operations
- ✅ **CREATE DATABASE** - Create new database directories
- ✅ **USE DATABASE** - Switch between databases  
- ✅ **CREATE TABLE** - Define tables with multiple data types and constraints
- ✅ **SHOW TABLES** - List all tables in current database
- ✅ **DESCRIBE TABLE** - View table schema and columns
- ✅ **TRUNCATE TABLE** - Clear all rows from a table

### 📝 Data Manipulation
- ✅ **INSERT INTO** - Add rows with type validation
- ✅ **SELECT** - Query with powerful filtering and aggregation
- ✅ **UPDATE** - Modify rows matching conditions
- ✅ **DELETE FROM** - Remove rows based on WHERE clause

### 🔍 Query Features
| Feature | Example | Supported |
|---------|---------|-----------|
| **WHERE Filters** | `age > 30 AND salary < 100000` | ✅ |
| **Logical Operators** | `AND`, `OR` with proper precedence | ✅ |
| **ORDER BY** | `ORDER BY age DESC` | ✅ |
| **LIMIT/OFFSET** | Pagination support | ✅ |
| **DISTINCT** | Remove duplicate rows | ✅ |
| **Aggregates** | COUNT, SUM, AVG, MIN, MAX | ✅ |
| **Pattern Matching** | LIKE with % and _ wildcards | ✅ |

### 📦 Supported Data Types
```
INT          FLOAT        TEXT/VARCHAR    BOOL
CHAR         DATE (YYYY-MM-DD)           NULL support
```

---

## 🚀 Quick Start

### Prerequisites
- C++ 17 or higher
- g++ compiler
- Linux/Windows (with path adaptation)

### Compilation

```bash
cd src
g++ main.cpp parser.cpp where.cpp schema.cpp types.cpp table.cpp db.cpp insert.cpp select.cpp delete.cpp update.cpp aggregate.cpp -o kittydb
```

### Run

```bash
./kittydb or kittydb
```

```
Welcome to KittyDB
Type 'help' to show supported commands
Type 'exit' to quit
KittyDB >
```

---

## 💡 Usage Examples

### 1️⃣ Create & Setup

```sql
create database company
use company
create table employees (id int, name text not null, salary float, joined_date date)
```

### 2️⃣ Insert Data

```sql
insert into employees values (1, "Alice Johnson", 75000.00, "2020-01-15")
insert into employees values (2, "Bob Smith", 85000.50, "2019-03-22")
insert into employees values (3, "Carol White", 92000.00, "2018-06-10")
insert into employees values (4, "David Brown", 68000.75, "2021-09-30")
insert into employees values (5, "Eve Davis", 88000.00, "2020-11-05")
```

### 3️⃣ Query Data

```sql
-- ✅ Simple SELECT
select * from employees

-- ✅ Filter results
select * from employees where salary > 80000

-- ✅ Complex conditions
select name, salary from employees 
where salary >= 80000 and id > 2

-- ✅ Pattern matching
select * from employees where name like "%Smith%"

-- ✅ Range queries
select * from employees where salary between 75000 and 90000
```

### 4️⃣ Sorting & Pagination

```sql
-- ✅ Sort descending
select * from employees order by salary desc limit 3

-- ✅ Pagination
select * from employees limit 2 offset 2

-- ✅ Multiple sorting
select name, salary from employees 
where id >= 2 
order by salary desc 
limit 5
```

### 5️⃣ Aggregation

```sql
-- ✅ Count employees
select count(*) from employees

-- ✅ Statistics
select avg(salary), min(salary), max(salary) from employees

-- ✅ Multiple aggregates
select count(*), avg(salary), sum(salary) from employees
```

### 6️⃣ Update & Delete

```sql
-- ✅ Update
update employees set salary = 95000 where name = "Alice Johnson"

-- ✅ Delete
delete from employees where id = 4
```

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────┐
│           KittyDB Engine                │
├─────────────────────────────────────────┤
│  main.cpp                               │
│  ├─ REPL / Command Interface            │
│  └─ Command Dispatcher                  │
├─────────────────────────────────────────┤
│  parser.cpp                             │
│  ├─ SQL Tokenization                    │
│  ├─ Command Parsing                     │
│  └─ WHERE Clause Analysis               │
├─────────────────────────────────────────┤
│  Execution Layer                        │
│  ├─ select.cpp (Query Execution)        │
│  ├─ insert.cpp (Row Insertion)          │
│  ├─ update.cpp (Row Modification)       │
│  ├─ delete.cpp (Row Deletion)           │
│  └─ aggregate.cpp (Calculations)        │
├─────────────────────────────────────────┤
│  Storage Layer                          │
│  ├─ table.cpp (Table Management)        │
│  ├─ schema.cpp (Schema Parsing)         │
│  ├─ db.cpp (Database Management)        │
│  ├─ types.cpp (Type Validation)         │
│  └─ where.cpp (Condition Evaluation)    │
├─────────────────────────────────────────┤
│  File System                            │
│  └─ databases/                          │
│     ├─ [database1]/                     │
│     │  ├─ table1.tbl (Data)             │
│     │  ├─ table1.meta (Schema)          │
│     │  └─ table1.seq (Sequence)         │
│     └─ [database2]/                     │
└─────────────────────────────────────────┘
```

---

## 📋 Supported SQL Syntax

### Database Commands
```sql
CREATE DATABASE database_name
USE database_name
SHOW TABLES
DESCRIBE table_name
TRUNCATE TABLE table_name
```

### Table Definition
```sql
CREATE TABLE table_name (
    column_name DATA_TYPE,
    column_name DATA_TYPE NOT NULL,
    ...
)
```

### Data Operations
```sql
-- INSERT
INSERT INTO table_name VALUES (val1, val2, val3)

-- SELECT
SELECT [DISTINCT] col1, col2, ... FROM table_name
  [WHERE condition]
  [ORDER BY column ASC|DESC]
  [LIMIT n OFFSET m]

-- UPDATE
UPDATE table_name SET col1=val1, col2=val2 WHERE condition

-- DELETE
DELETE FROM table_name WHERE condition
```

### WHERE Operators

```
=        Equal to
!=       Not equal to
<        Less than
>        Greater than
<=       Less than or equal
>=       Greater than or equal
BETWEEN  Range check
IN       Value in list
LIKE     Pattern matching (% any, _ single)
IS NULL  Null check
IS NOT NULL  Not null check
```

### Logical Operators
```
AND      Both conditions true
OR       At least one condition true
```

---

## 💾 Data Storage Format

```
databases/
├── company/
│   ├── employees.tbl          (Pipe-delimited data)
│   ├── employees.meta         (Schema definition)
│   └── employees.seq          (Auto-increment counter)
└── production/
    └── orders.tbl
```

### Example `.tbl` File (Pipe-delimited)
```
1|Alice Johnson|75000.00|2020-01-15
2|Bob Smith|85000.50|2019-03-22
3|Carol White|92000.00|2018-06-10
```

### Example `.meta` File
```
id int, name text not null, salary float, joined_date date
```

---

## ⚙️ Query Execution Pipeline

```
SQL Input
   ↓
Parser (Tokenization & Analysis)
   ↓
Command Type Identification
   ↓
WHERE Filtering (Row Selection)
   ↓
ORDER BY (Sorting)
   ↓
DISTINCT (Deduplication)
   ↓
AGGREGATES (Calculations)
   ↓
LIMIT/OFFSET (Pagination)
   ↓
Output Formatting & Display
```

---

## 🎯 System Features

### ✅ Automatic Columns
Every table includes an `__id` column:
```
__id (INT, AUTO-INCREMENT, READ-ONLY)
```

### ✅ Type Validation
```
INSERT INTO users VALUES (1, "Alice", 28)     ✅ OK
INSERT INTO users VALUES ("text", "Bob", 30)  ❌ Type Mismatch
```

### ✅ Constraint Support
```
CREATE TABLE users (
    id INT,
    email TEXT NOT NULL              ✅ Enforced
)

INSERT INTO users VALUES (1, NULL)   ❌ Error: Cannot be NULL
```

### ✅ NULL Handling
```
-- Columns without NOT NULL accept NULL
INSERT INTO logs VALUES (1, NULL)    ✅ OK

-- Columns with NOT NULL reject NULL
INSERT INTO users VALUES (1, NULL)   ❌ Error
```

---

## 📊 Performance Characteristics

| Operation | Complexity | Notes |
|-----------|-----------|-------|
| SELECT | O(n) | Full table scan (no indexes) |
| INSERT | O(1) | Append to file |
| UPDATE | O(n) | Scan + rewrite |
| DELETE | O(n) | Scan + rewrite |
| ORDER BY | O(n log n) | In-memory sort |
| WHERE | O(n) | Linear scan |

**Best for:** Small to medium datasets (< 100K rows)

---

## ❌ Limitations for Now

- No JOIN operations (single table queries only)
- No GROUP BY / HAVING clauses
- No transaction support (COMMIT/ROLLBACK)
- No indexing (all queries are full scans)
- No PRIMARY KEY or FOREIGN KEY constraints
- No ALTER TABLE (immutable schema)
- No arithmetic in WHERE clauses
- Windows-only file paths (adaptable)

---

## 🔮 Future Enhancements

- [ ] Multi-table JOINs (INNER, LEFT, RIGHT, FULL)
- [ ] GROUP BY with HAVING support
- [ ] Index creation and management
- [ ] Transaction support (ACID properties)
- [ ] Constraint system (PKs, FKs, UNIQUE)
- [ ] ALTER TABLE operations
- [ ] Cross-platform paths
- [ ] Concurrent access
- [ ] Query optimization

---

## 🎓 Learning Outcomes

This project demonstrates:

- **SQL Fundamentals** → Query syntax, semantics, execution
- **Parsing** → Custom recursive descent parser, tokenization
- **Data Structures** → Efficient use of vectors, sets, maps
- **File I/O** → Persistent storage and retrieval
- **Algorithms** → Sorting, searching, filtering
- **Software Design** → Modular architecture, separation of concerns
- **Error Handling** → Comprehensive validation and messaging

---

## 📝 Code Quality

✨ **Well-Structured**
- Modular design with clear separation of concerns
- Each component has a specific responsibility
- Clean interfaces between modules

🛡️ **Robust**
- Comprehensive error handling
- Input validation at all levels
- Type checking and bounds verification

📖 **Maintainable**
- Clear naming conventions
- Logical file organization
- Consistent coding style

---

## 👨‍💻 Author

## Author

**J Dipayan Rao**

A passionate developer interested in database systems and programming.

- GitHub: [@mrstrange2003](https://github.com/mrstrange2003)
- LinkedIn: [J Dipayan Rao](https://www.linkedin.com/in/dipayan-rao)
---

<div align="center">

**[⭐ Star this repo if you found it helpful!](https://github.com)**

**[🐛 Report Issues](https://github.com)** • **[💡 Suggest Features](https://github.com)**

**Happy querying! 🚀🐱**

Made with ❤️ by a passionate developer

</div>
