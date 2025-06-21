# The Volcano (Iterator) Model of Query Execution

The Volcano model, also known as the Iterator model, is a standard and elegant architecture for processing queries in a database system. It provides a clean, composable, and efficient way to execute even the most complex SQL queries.

## What is the Volcano Model?

The core idea is to decompose a query execution plan into a tree of **Operators**. Each operator is a small, independent component that performs one specific task (e.g., scanning a table, filtering rows, sorting data).

Every operator implements the same simple interface: a 'next()' method.

When 'next()' is called on an operator, it does two things:
1.  It pulls one or more rows from its child operator(s) by calling *their* 'next()' methods.
2.  It performs its specific processing on those rows and returns a single resulting row to its parent.

This creates a pipeline where data flows from the bottom of the tree (the data source) up to the top (the client), one row at a time.

For a query like 'SELECT name FROM users WHERE age > 30;', the operator tree would look like this:

```
    [Client]
       ^
       | .next() -> gets a row like {"name": "Alice"}
       |
 [ProjectionOperator]
       ^
       | .next() -> pulls a row, extracts the 'name' column, and passes it up
       |
  [FilterOperator]
       ^
       | .next() -> pulls rows from the scan until one has age > 30, then passes it up
       |
 [TableScanOperator]
       ^
       | .next() -> reads the next row from the physical table file on disk
       |
    [Disk]
```

## Why is it Called 'Volcano'?

The model gets its name from the visual metaphor of data "erupting" from the top of the operator tree, one tuple (row) at a time, just as lava flows from the top of a volcano. The client at the top keeps calling 'next()', and each call causes a cascade of 'next()' calls down the tree, eventually pulling a single row up from the disk and through the pipeline.

```
    Client
      /|\
       |  <-- Data "erupts" one row at a time
      \|/
+--------------------+
| ProjectionOperator |
+--------------------+
         |
+--------------------+
|   FilterOperator   |
+--------------------+
         |
+--------------------+
|  TableScanOperator |
+--------------------+
         |
    Data Source (Disk)
```

## Key Concepts and Terminology

*   **Operator (or Node):** A single processing unit in the pipeline. In our code, this will be a class like 'TableScanOperator' or 'FilterOperator'.
*   **Iterator Interface:** The common interface that all operators must implement. The key method is 'next()', which retrieves the next processed row. It must also have a way to signal that there are no more rows (e.g., by returning 'std::nullopt' or a null pointer).
*   **Pull-based Execution:** The client at the top *pulls* data from the pipeline. No work is done until a row is requested. This is in contrast to a push-based model where the source pushes data up as fast as it can.
*   **On-Demand Execution:** A direct benefit of the pull-based model. The pipeline only performs the minimum work necessary to produce the next row. If a user runs 'SELECT * FROM large_table LIMIT 5;', the 'TableScanOperator' will only read the first 5 rows from disk because the 'LimitOperator' at the top will stop asking for more rows after it has received 5.

## How Other Databases Refer to It (e.g., PostgreSQL)

The concepts are universal, but the terminology can vary slightly.

*   **PostgreSQL:**
    *   **Operator:** They are called **Plan Nodes** or simply **Nodes**.
    *   **Implementation:** The C structs that represent these nodes are named after their function, e.g., 'SeqScan' (Sequential Scan, src/backend/executor/nodeSeqscan.c), 'IndexScan', 'Sort', 'HashJoin'.
    *   **'next()' method:** The actual execution logic for each node is typically in a function called 'Exec<NodeName>', for example, 'ExecSeqScan'.

## Summary Table

| Our Terminology | PostgreSQL Terminology | Core Concept |
| :--- | :--- | :--- |
| **Operator** | Plan Node / Node | A single step in the execution plan (e.g., Scan, Filter). |
| **'next()'** | 'Exec<NodeName>()' | The function that gets the next row from an operator. |
| **Execution Plan** | Plan Tree | The tree of operators that will be executed to satisfy a query. |
