 In computer science, a “database” is an organized collection of data that allows for easy access,
 management, and updating. It’s essentially a storage system where you can keep large amounts of
 information, such as student records, product inventories, or customer information. The most
 popular type of databases is relational, where the data is organized as tables.
 Your Task. In this project, you will implement a mini-database management system (i.e.,
 miniDB) that supports creating tables, updating records, querying tables, and more. Your system
 should be able to process commands in our simplified version of SQL (i.e., miniSQL) and
 output the query results.
 What is SQL? SQL (Structured Query Language) is a standard language for managing and
 manipulating relational databases, enabling users to create, read, update, and delete data, as well
 as define and modify database structures. Here, you’ll work with a simplified version of SQL
 designed specifically for our miniDB.
 How a table in our miniDB looks like? Table 1 shows a table used in our mini database.
 Each row is a “record,” and each column is a “field.” Your database needs to manage multiple
 tables.
 
 Table 1: A table showing student information
 ID Name GPA Major
 1000 Jay Chou 3.0  Microelectronics
 1001 Taylor Swift 3.2  Data Science
 1002 Bob Dylan 3.5 Financial Technology
 
 Usage of minidb Compile your program into a single executable named minidb, which accepts
 two command-line arguments: the first is the input SQL file, and the second is the output file.
 The basic usage format of the program is as follows:
 ./minidb input.sql output.txt
