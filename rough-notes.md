# Rough Notes

This is a dump of my thoughts when working on a new feature, or exploring different alternatives for xyz stuff, etc.

Next task is to add support for SELECT queries.

#### Future todo #1: Buffer Pool Manager

A "Pager" (also called a Buffer Pool Manager) is an in-memory cache for disk pages. 
Real databases never operate directly on the disk file for every single read or write because disk I/O is thousands of times slower than memory access.

What a real Pager does:
- Maintains a pool of page-sized memory buffers (e.g., 1000 buffers of 4KB each).
- When the executor needs to read Page #5, the Pager checks if it's already in one of its memory buffers.
- If it is (a "cache hit"), it just returns a pointer to the memory, which is super fast.
- If it's not (a "cache miss"), the Pager must:
  1. Find an empty buffer.
  2. If no buffer is empty, it must choose a page to evict (e.g., the "Least Recently Used" one). 
  3. If the page being evicted was modified (is "dirty"), it must be written back to disk before its buffer can be reused. 
  4. Finally, it reads the requested Page #5 from disk into the now-free buffer.
- It also handles "pinning" pages in memory so they can't be evicted while a transaction is actively using them.

What our simplified "No Pager Cache" approach does:
- When the executor needs to read or modify a page, it will always go directly to the disk.
  1. The logic would look like this:
  2. Executor: "I need to insert a record into the last page of users.data."
  3. Open users.data file. 
  4. seek() to the position of the last page. 
  5. read() the entire 4KB page from disk into a single, temporary Page object in memory. 
  6. Modify that one Page object in memory. 
  7. seek() back to the same position in the file. 
  8. write() the entire modified 4KB page back to disk. 
  9. Close the file.

#### Future todo #2: Indexes

An index is a separate data structure that allows for fast lookups. 
Without an index, the only way to find a specific row is to scan the entire table from beginning to end.

What a real Index (like a B-Tree) does:
- It's a separate structure (often in its own file) that maps column values to the physical location of the rows.
- To find the user with id = 123, the database doesn't read the whole table. It searches the B-Tree index for the key 123.
- The B-Tree search is very fast (logarithmic complexity), quickly leading to a leaf node that says "the row for id 123 is on Page #57 at offset 820".
- The database then uses the Pager to fetch only Page #57 and reads the record from that specific offset.

What our simplified "No Indexes" approach does:
- Our TableHeap is just a collection of pages with no specific order.
- When we eventually implement SELECT * FROM users WHERE id = 123, the logic will be:
  1. Go to Page #1 of users.data. Read it into memory. 
  2. Scan every record on that page. Deserialize it and check if its id is 123. 
  3. Go to Page #2. Read it. Scan every record. 
  4. ...continue this for every single page in the table file until the record is found or the end of the file is reached.

This is called a Full Table Scan. 
It's very slow for large tables but is simple to implement because you don't need to build or maintain any complex index data structures. You are deferring the entire concept of data structures for fast lookups.

## Other handy things:
1. `files-to-prompt . | pbcopy`
2. Older chat: https://aistudio.google.com/prompts/17DRNPhY1QuXOOECcAXZGttiH5vi1Qd8g
3. Newer chat: https://aistudio.google.com/prompts/1olmlfe1X55D7MrO6T1V4RE8JPyqr4ggu

## Minor Todos:
1. Update application.log filepath to also use ENV_DATA_DIR.
2. Update ci.yml to use --parallel flag for cmake.
