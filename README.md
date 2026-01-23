# KittyDB 🐱

A lightweight, file-based relational database engine built from scratch in C++. KittyDB implements a subset of SQL functionality with a custom query parser, providing a complete experience of database internals.

## Features

### Core Database Operations
- **CREATE DATABASE** - Create new database directories
- **USE DATABASE** - Switch between databases
- **CREATE TABLE** - Define tables with multiple data types
- **SHOW TABLES** - List all tables in current database
- **DESCRIBE TABLE** - View table schema and columns
- **TRUNCATE TABLE** - Clear all rows from a table

### Data Manipulation
- **INSERT INTO** - Add rows with type validation
- **SELECT** - Query with powerful filtering and aggregation
- **UPDATE** - Modify rows matching conditions
- **DELETE FROM** - Remove rows based on WHERE clause

### Query Features
- **WHERE Clause** - Filter with `=`, `!=`, `<`, `>`, `<=`, `>=`, `BETWEEN`, `IN`, `LIKE`, `IS NULL`
- **Logical Operators** - Combine conditions with `AND`/`OR`
- **ORDER BY** - Sort results ascending or descending
- **LIMIT / OFFSET** - Pagination support
- **DISTINCT** - Remove duplicate rows
- **Aggregates** - `COUNT()`, `SUM()`, `AVG()`, `MIN()`, `MAX()`

### Supported Data Types
- `INT` - Integer values
- `FLOAT` - Decimal numbers
- `TEXT` / `VARCHAR` - String data
- `BOOL` - Boolean values (true/false, 1/0)
- `CHAR` - Single character
- `DATE` - Date in YYYY-MM-DD format

## Architecture

```
KittyDB/
├── parser.cpp/h       - SQL command parsing and tokenization
├── where.cpp/h        - WHERE clause evaluation and condition matching
├── select.cpp/h       - SELECT query execution with filtering/sorting
├── insert.cpp/h       - INSERT row operations
├── update.cpp/h       - UPDATE row operations
├── delete.cpp/h       - DELETE row operations
├── aggregate.cpp/h    - Aggregate function calculations
├── table.cpp/h        - Table creation and schema management
├── db.cpp/h           - Database creation and directory handling
├── schema.cpp/h       - Schema parsing and column management
├── types.cpp/h        - Data type validation
├── main.cpp           - Command-line interface and REPL
└── databases/         - Data storage directory (auto-created)
```

## Getting Started

### Compilation

```bash
cd src
g++ main.cpp parser.cpp where.cpp schema.cpp types.cpp table.cpp db.cpp insert.cpp select.cpp delete.cpp update.cpp aggregate.cpp -o kittydb
```

### Running

```bash
./kittydb or kittydb
```

You'll see the KittyDB prompt:
```
Welcome to KittyDB
Type 'help' to show supported commands
Type 'exit' to quit
KittyDB >
```

## Usage Examples

### Creating a Database and Table

```sql
create database company
use company
create table employees (name text, age int, salary float)
```

### Inserting Data

```sql
insert into employees values ("Alice Johnson", 28, 75000.00)
insert into employees values ("Bob Smith", 35, 85000.50)
insert into employees values ("Carol White", 32, 92000.00)
insert into employees values ("David Brown", 29, 68000.75)
insert into employees values ("Eve Davis", 31, 88000.00)
```

### Basic Queries

```sql
-- Select all rows
select * from employees

-- Select specific columns
select name, salary from employees

-- Filter with WHERE
select * from employees where age > 30

-- Complex conditions
select * from employees where salary >= 80000 and age < 35
select * from employees where name like "%Smith%"
select * from employees where age between 28 and 32
```

### Sorting and Pagination

```sql
-- Sort by salary descending
select * from employees order by salary desc

-- Get top 3 highest paid employees
select * from employees order by salary desc limit 3

-- Pagination: Get 2 rows per page, page 2
select * from employees limit 2 offset 2
```

### Aggregation

```sql
-- Count total employees
select count(*) from employees

-- Average salary
select avg(salary) from employees

-- Min and max age
select min(age), max(age) from employees

-- Sum of all salaries
select sum(salary) from employees
```

### Advanced Queries

```sql
-- Remove duplicates
select distinct age from employees

-- Multiple aggregates
select count(*), avg(salary), max(salary) from employees

-- Filter with ORDER BY and LIMIT
select * from employees where age >= 30 order by name asc limit 10
```

### Updating Data

```sql
-- Update single column
update employees set salary = 90000 where name = "Alice Johnson"

-- Update multiple columns
update employees set age = 33, salary = 95000 where name = "Bob Smith"
```

### Deleting Data

```sql
-- Delete specific rows
delete from employees where age < 25

-- Delete rows matching complex condition
delete from employees where salary < 65000 and age > 40
```

## Supported SQL Syntax

### Commands (Case-Insensitive)

```
CREATE DATABASE <name>
USE <database>
CREATE TABLE <table> (col TYPE, col TYPE, ...)
SHOW TABLES
DESCRIBE <table>
TRUNCATE <table>

INSERT INTO <table> VALUES (val1, val2, ...)
SELECT [DISTINCT] col1, col2, ... FROM <table>
       [WHERE condition]
       [ORDER BY col ASC|DESC]
       [LIMIT n OFFSET m]
UPDATE <table> SET col=val [, col=val] WHERE condition
DELETE FROM <table> WHERE condition
```

### WHERE Operators

| Operator | Example | Description |
|----------|---------|-------------|
| `=` | `age = 30` | Equals |
| `!=` | `age != 25` | Not equals |
| `<` | `salary < 50000` | Less than |
| `>` | `age > 30` | Greater than |
| `<=` | `salary <= 100000` | Less than or equal |
| `>=` | `age >= 25` | Greater than or equal |
| `BETWEEN` | `age between 25 and 35` | Within range |
| `IN` | `status in (1, 2, 3)` | In list |
| `LIKE` | `name like "%Smith"` | Pattern matching (% = any chars, _ = single char) |
| `IS NULL` | `phone is null` | Null check |
| `IS NOT NULL` | `email is not null` | Not null check |

### Logical Operators

- `AND` - Both conditions must be true
- `OR` - At least one condition must be true

```sql
select * from employees where age > 30 and salary > 80000
select * from employees where department = "HR" or department = "IT"
```

## Technical Details

### Data Storage

- Databases are stored as directories in `databases/`
- Tables use three files:
  - `.tbl` - Actual row data (pipe-delimited format)
  - `.meta` - Schema metadata
  - `.seq` - Auto-increment sequence counter

Example `.tbl` file:
```
1|Alice Johnson|28|75000.00
2|Bob Smith|35|85000.50
3|Carol White|32|92000.00
```

### Parser Implementation

The parser uses a hand-written recursive descent approach that:
- Converts SQL keywords to uppercase for comparison
- Preserves original casing for identifiers (database/table/column names)
- Handles whitespace-tolerant command parsing
- Supports complex WHERE clauses with AND/OR logic

### Query Execution Order

1. **WHERE filtering** - Apply conditions to rows
2. **ORDER BY** - Sort filtered rows
3. **DISTINCT** - Remove duplicates
4. **Aggregates** - Calculate functions (returns early)
5. **LIMIT/OFFSET** - Apply pagination

## Limitations

- **No JOINs** - Cannot query across multiple tables
- **No GROUP BY** - Cannot group results
- **No transactions** - No COMMIT/ROLLBACK support
- **No indexes** - All queries perform full table scans
- **No constraints** - No PRIMARY KEY, FOREIGN KEY, or UNIQUE constraints
- **Single-user** - Not designed for concurrent access
- **No ALTER TABLE** - Cannot modify table schema after creation
- **No arithmetic expressions** - WHERE clauses don't support math operations
- **Windows-only paths** - Uses Windows-style path separators (can be adapted)

## System Columns

Every table automatically includes a `__id` column:
- **Type:** INT
- **Auto-increment:** Increments with each new row
- **Read-only:** Cannot be modified by users
- **Purpose:** Serves as implicit primary key

Example:
```sql
create table users (name text)
insert into users values ("Alice")
insert into users values ("Bob")
-- Internally stored as: 1|Alice and 2|Bob
```

## Error Handling

KittyDB provides clear error messages for:
- Missing database or table
- Invalid data types
- Unknown columns
- Malformed WHERE conditions
- Type mismatches in INSERT/UPDATE
- Modifying system columns

## Performance Notes

- **Full table scans** - Every SELECT scans entire table (no indexes)
- **In-memory sorting** - ORDER BY loads all rows into memory
- **String parsing** - WHERE conditions use string parsing (no compilation)

For small datasets (< 100,000 rows), performance is acceptable.

## Future Enhancements

Potential improvements for future versions:
- [ ] JOIN operations (INNER, LEFT, RIGHT, FULL)
- [ ] GROUP BY and HAVING clauses
- [ ] Index support for faster queries
- [ ] Transaction support (COMMIT/ROLLBACK)
- [ ] Constraints (PRIMARY KEY, FOREIGN KEY, UNIQUE)
- [ ] ALTER TABLE operations
- [ ] Cross-platform path handling
- [ ] Multi-user concurrent access
- [ ] Query optimization and execution planning

## Code Quality

- **Modular design** - Separated concerns (parsing, execution, storage)
- **Error handling** - Comprehensive error messages and validation
- **Memory safety** - Proper resource management and cleanup
- **Input validation** - Type checking and bounds verification

## Learning Outcomes

Building KittyDB demonstrates understanding of:
- **SQL fundamentals** - Query syntax and semantics
- **Parsing** - Custom command-line parsing and tokenization
- **Data structures** - Vectors, sets, maps for efficient data management
- **File I/O** - Persistent data storage and retrieval
- **Algorithm implementation** - Sorting, searching, filtering
- **Software design** - Modular architecture and separation of concerns

## Author

**J Dipayan Rao**

A passionate developer interested in database systems and programming.

- GitHub: [@mrstrange2003](https://github.com/mrstrange2003)
- LinkedIn: [J Dipayan Rao](https://www.linkedin.com/in/dipayan-rao)
  
---

**Happy querying! 🚀**
