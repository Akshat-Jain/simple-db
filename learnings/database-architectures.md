# Database Architectures: Embedded vs. Client-Server

Our `simple-db` project is currently a **single-process, single-threaded** application. This means it can only do one thing at a time and is run as a standalone executable. This is great for development, but how would it be used in a real-world application? This document explores the two primary architectural models for databases.

## Model 1: Embedded Database (e.g., SQLite)

This model is the most direct evolution from our current project. The database engine is not a standalone server but a **library** that is linked directly into a host application.



*   **How It's Used:**
    *   You would compile `simple-db` into a library file (e.g., `libsimpledb.so` or `libsimpledb.a`).
    *   An application (like a web server, desktop software, or mobile app) includes your header files and links against your library.
    *   The host application calls functions from your library directly, like `simpledb_open("my_db.data")` and `simpledb_execute("SELECT * FROM users;")`. The database runs inside the host application's process.

*   **API:**
    *   You must define and expose a stable Application Programming Interface (API), typically as a set of C-style functions in a header file. This API is the "product" you ship.

*   **Concurrency:**
    *   The **host application** is responsible for managing threads. If it wants to handle multiple requests at once, it will call your library's functions from multiple threads.
    *   Therefore, your library code **must be thread-safe**. You would need to add internal locking mechanisms (like mutexes) to protect shared data structures (like the page cache or the catalog) from being corrupted by simultaneous access.

## Model 2: Client-Server Database (e.g., PostgreSQL, MySQL)

This is the more traditional model for shared, multi-user databases. The database is a completely separate program that runs as a background service or "daemon".



*   **How It's Used:**
    *   **The Server (`simpledbd`):** A dedicated, long-running process that "owns" the database files. It listens for connections on a network port (e.g., TCP port 5432).
    *   **The Client (`simple-db` REPL, application, etc.):** A separate program that connects to the server over the network. It sends SQL queries as messages and receives results back as messages. The client **never** touches the database files directly.

*   **API:**
    *   The "API" is the **network protocol**. You must define a strict set of rules for how bytes are exchanged between the client and server to represent queries, results, errors, etc.

*   **Concurrency:**
    *   The **server process** is responsible for managing concurrency.
    *   When a new client connects, the server typically spawns a new **worker thread** or process dedicated to that client.
    *   With multiple worker threads running inside the server, the engine code still needs to be thread-safe, using locks or other concurrency control mechanisms (like MVCC) to manage access to the data files and internal structures.

## Summary for `simple-db`

| Aspect | Current `simple-db` | Embedded Model (Next Step) | Client-Server Model (Future) |
| :--- | :--- | :--- | :--- |
| **Form Factor** | Executable REPL | Library (`.so`, `.a`) | Server Daemon + Client App |
| **API** | None (internal calls) | C/C++ Function API | Network Protocol |
| **Concurrency** | Single-threaded | Handled by host app; library must be thread-safe (locks). | Handled by server; engine must be thread-safe (locks). |
| **Data Access** | Direct file access | Direct file access (by library) | Indirect (via network) |

Our current path of building the core engine (parser, executor, storage manager) is the necessary foundation for **both** models. The choice between them and the implementation of a concurrency control layer are major architectural decisions for the future.