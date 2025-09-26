# Rough Notes

This is a dump of my thoughts when working on a new feature, or exploring different alternatives for xyz stuff, etc.

Next tasks:
1. WHERE clause support (strings/numbers only for now, no expressions) ✅ COMPLETED
2. Data generation utilities (1K, 100K, 1M row datasets)
3. Benchmarking framework (select queries, document results in a `benchmarks/` directory)
4. Index implementation (B+ tree for single columns)
5. Performance measurement (before/after index comparison)

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

#### B+ Tree Implementation Plan

**Implementation Phases:**
1. Create BTreePage structure and file management
2. Implement basic B+ tree operations (insert, search, split)
3. Build IndexScanOperator following Volcano model
4. Extend planner with index awareness and cost-based decisions
5. Add comprehensive benchmarks demonstrating O(n) → O(log n) improvements

## Minor Todos:
1. Update application.log filepath to also use ENV_DATA_DIR.
