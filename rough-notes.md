# Rough Notes

This is a dump of my thoughts when exploring different alternatives for xyz stuff.

### `INSERT INTO` Feature Implementation Plan

#### **Phase 1: Foundational Storage Layer (`page.h`, `page.cpp`)**

This phase is about creating the building blocks for storing data.

*   **1.1. Define Page Constants and Structures (`page.h`)**
  *   [ ] Create `include/simpledb/page.h`.
  *   [ ] Define `PAGE_SIZE` constant (e.g., `4096`).
  *   [ ] Define the `Page` struct holding the raw byte array (e.g., `std::array<char, PAGE_SIZE>`).
  *   [ ] Define the `Slot` struct (`record_offset`, `record_length`).

*   **1.2. Implement Page Header API (`page.h` / `page.cpp`)**
  *   [ ] Create a `Page` class or a set of free functions to manage the page header.
  *   [ ] Implement getters/setters for header fields: `get_version`, `get_num_records`, `set_num_records`, `get_free_space_ptr`, `set_free_space_ptr`. These will read/write integers from the raw byte array.
  *   [ ] Implement a `Page::initialize()` method to format a new, empty page (set header fields to default values).

*   **1.3. Implement Page Record Management API (`page.h` / `page.cpp`)**
  *   [ ] Implement `Page::get_slot(slot_num)` to retrieve a `Slot` from the slot array.
  *   [ ] Implement `Page::add_record(const std::vector<char>& record_data)`:
    *   Calculates if there's enough free space for the record and a new slot.
    *   If yes, copies the data into the page's heap, adds a new slot, updates the header, and returns `true`.
    *   If no, returns `false`.
  *   [ ] Implement `Page::get_record(slot_num)` which uses a slot to find and return the binary data for a specific record.

#### **Phase 2: Table-Level Storage (`table_heap.h`, `table_heap.cpp`)**

This phase manages the collection of pages for a single table.

*   **2.1. Define `TableHeap` Class (`table_heap.h`)**
  *   [ ] Create `include/simpledb/table_heap.h`.
  *   [ ] The class will manage a table's `.data` file. Its constructor will take a file path.
  *   [ ] Declare the main method: `bool insert_record(const std::vector<char>& record_data);`.

*   **2.2. Implement `TableHeap` Logic (`table_heap.cpp`)**
  *   [ ] Implement the `insert_record` method following the logic we designed:
    *   Open the table's `.data` file.
    *   If the file is empty, create a new page (Page 0), add the record to it, and write it to the file.
    *   If the file is not empty, seek to the last page, read it into memory.
    *   Try to add the record to this page using `Page::add_record`.
    *   If it succeeds, write the modified page back to its original location.
    *   If it fails (page is full), create a new page, add the record to it, and append the new page to the end of the file.

#### **Phase 3: Integration with Parser and Executor**

This phase connects the new storage layer to the existing command processing pipeline.

*   **3.1. Implement Parser for `INSERT` (`parser.cpp`)**
  *   [ ] Fully implement `parse_insert` to extract the table name and a `vector<string>` of values.
  *   [ ] Handle syntax like `INSERT INTO table_name VALUES ('val1', 2, 'val3');`. This will require careful string parsing to handle parentheses, commas, and quotes.

*   **3.2. Implement Row Serialization (`executor.cpp` or a new utility)**
  *   [ ] Create a `serialize_row(const std::vector<std::string>& values)` function that implements the `[len][data]` binary format we designed.

*   **3.3. Update Executor for `INSERT` (`executor.cpp`)**
  *   [ ] In `execute_insert_command`:
    *   Get the table schema from the catalog.
    *   Validate that the number of values from the parsed command matches the number of columns in the schema.
    *   Call `serialize_row` to get the binary record data.
    *   Create a `TableHeap` instance for the table.
    *   Call `table_heap.insert_record` with the serialized data.
    *   Return a success or failure `ExecutionResult`.

#### **Phase 4: Testing**

*   **4.1. Test the `Page` class (`page_test.cpp`)**
  *   [ ] Create a new test file.
  *   [ ] Test that `Page::initialize` works correctly.
  *   [ ] Test that `Page::add_record` correctly adds a record and updates the header.
  *   [ ] Test that it correctly reports failure when the page is full.
  *   [ ] Test that `get_record` retrieves the correct data after insertion.

*   **4.2. Test the `TableHeap` class (`table_heap_test.cpp`)**
  *   [ ] Test that inserting into an empty file creates a new file with one page.
  *   [ ] Test that inserting multiple records fills up the first page and then correctly creates a second page.

*   **4.3. Test the `INSERT` command (`parser_test.cpp`, `executor_test.cpp`)**
  *   [ ] Add tests to `parser_test.cpp` for valid and invalid `INSERT` syntax.
  *   [ ] Add an end-to-end test in `executor_test.cpp` that simulates running an `INSERT` command and verifies that the underlying `.data` file is modified correctly. (This might require a helper function to read and interpret the file for test verification).

## Next main todo: Implementing INSERT INTO command

Page-based approach.

1. Identify the Target Page: The simplest strategy is to always try to insert into the last page of the table's .data file. 
   - How do we know where the last page is? file_size / page_size.
2. Read the Target Page: Read that single page from disk into memory.
3. Check for Space: Check if the new record will fit in the available free space on that page.
   - You'll know this from the page's header, which tracks how much space is used.
4. Handle the Two Cases:
   - Case A: The record fits.
      1. Write the new record's data into the free space on the in-memory page.
     2. Update the page's header (e.g., increment the record count, decrease the amount of free space).
     3. Write the modified page back to the disk at the same location.
     4. Result: The file size does not change. The last page just becomes more full.
   - Case B: The record does NOT fit.
      1. Create a brand new, empty Page object in memory.
     2. Write the new record's data into this new page.
     3. Update this new page's header (record count is 1, etc.).
     4. Append this entire new page to the end of the table's .data file.
     5. Result: The table file grows by exactly one page size. We now have multiple pages.

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