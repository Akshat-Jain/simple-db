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

Based on PostgreSQL's approach, here's our minimal, fast B+ tree design:

**Core Page Structure:**
```cpp
struct BTreePageHeader {
    uint16_t page_type;        // 0=leaf, 1=internal
    uint16_t num_entries;      // Number of key-value pairs
    uint32_t prev_index_page;  // Left sibling index page (0 if none)
    uint32_t next_index_page;  // Right sibling index page (0 if none)  
};
// Total: 12 bytes

struct InternalEntry {
    int32_t separator_key;      // Boundary value
    uint32_t child_index_page;  // Child index page ID
};
// 8 bytes per entry

struct LeafEntry {
    int32_t key;                    // The indexed value
    uint32_t table_page_id;         // Which page in the TABLE file
    uint16_t table_row_offset;      // Offset within that TABLE page
};
// 10 bytes per entry
```

**Page Capacity (4KB pages):**
- Internal pages: 510 entries (can reference 511 child pages)
- Leaf pages: 408 key-value pairs
- Header overhead: only 12 bytes

**Architecture Integration:**
- `IndexScanOperator`: New Volcano-model operator for indexed access
- Planner chooses between `TableScanOperator` vs `IndexScanOperator` based on available indexes
- Index files: `table_name_column.idx` alongside `table_name.data`
- Clear separation: index pages reference other index pages OR table pages (with table_ prefix)

**Search Example (WHERE user_id = 502):**
1. Read index_page_0 (root): 502 >= 500 → go to child_index_page=6
2. Read index_page_6 (leaf): Find key=502 → {table_page=1, table_offset=64}
3. Read table_page_1 at offset 64: Get actual user row
**Result: 3 page reads instead of full table scan**

**Expected Performance Impact:**
- 100K row SELECT: 277ms → 1-5ms (logarithmic vs linear)
- INSERT overhead: slight increase for index maintenance
- Perfect foundation for benchmarking indexing benefits

**Implementation Phases:**
1. Create BTreePage structure and file management
2. Implement basic B+ tree operations (insert, search, split)
3. Build IndexScanOperator following Volcano model
4. Extend planner with index awareness and cost-based decisions
5. Add comprehensive benchmarks demonstrating O(n) → O(log n) improvements

## Minor Todos:
1. Update application.log filepath to also use ENV_DATA_DIR.
