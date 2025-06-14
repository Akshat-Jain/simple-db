# V1 Storage Engine: A Page-Based Approach

To implement `INSERT`, we must decide how to store table data on disk. Instead of a simple text file (like a CSV), we are adopting a more robust, page-based binary format. This design is a simplified version of the architecture used in production databases like PostgreSQL and SQLite.

## The Core Concepts

### Pages
A **page** is the fundamental unit of storage and I/O in the database. It is a fixed-size block of data (e.g., 4096 bytes) that is read from and written to disk in a single operation. This is efficient because it aligns with how operating systems and physical disks work. Think of a page as a single sheet of paper in a binder that holds our records.

### Slotted Page Layout
To manage variable-length records within a fixed-size page, we use a classic **Slotted Page** layout. This structure allows us to pack records efficiently without wasting space.

**Visual Diagram:**
```
<---------------------------------- 4096 bytes -------------------------------------->
+---------------+-----------------+-...-+----------------------+---------------------+
|  Page Header  | Slot 1 | Slot 2 | ... |      Free Space      | Record 2 | Record 1 |
+---------------+-----------------+-...-+----------------------+---------------------+
^               ^                 ^     ^                      ^                     ^
0             HeaderEnd         SlotEnd FreeSpaceStart       Record2Start          4095
```

The page is organized into three main sections that grow towards each other:
1.  **Page Header:** A fixed-size area at the beginning of the page containing metadata *about the page*.
2.  **Slot Array:** An array of pointers that grows from the end of the header forward. Each slot points to a specific record on the page.
3.  **Record Data:** The actual binary data for each row, packed at the end of the page and growing backward.

The space between the Slot Array and the Record Data is the **Free Space** available for new records.

## V1 Page and Record Design

### Page Header Layout
Our initial page header design is simple but extensible.

| Field | Offset (Bytes) | Size (Bytes) | Type | Rationale |
| :--- | :--- | :--- | :--- | :--- |
| `page_version` | 0 | 1 | `uint8_t` | **Crucial for future evolution.** Initialized to `1`. |
| `num_records` | 1 | 2 | `uint16_t` | The number of records currently on this page. |
| `free_space_ptr`| 3 | 2 | `uint16_t` | Pointer to the start of the record data heap (grows from the end). |
| *Padding/Reserved*| 5 | 3 | `char[3]` | Reserved for future use and aligns the slot array. |
| **Total Size** | | **8 bytes** | | |

### Slot Structure
Each slot in the slot array provides the location and size of a record.

| Field | Size (Bytes) | Type | Rationale |
| :--- | :--- | :--- | :--- |
| `record_offset` | 2 | `uint16_t` | The byte offset of the record from the start of the page. |
| `record_length` | 2 | `uint16_t` | The length of the record data in bytes. |
| **Total Size** | **4 bytes** | | |

### Record Serialization
For now, a record will be a simple binary format. For a row with N columns, the format will be:
`[len1 (4 bytes)][col1_data][len2 (4 bytes)][col2_data]...[lenN (4 bytes)][colN_data]`

**Note on Column Order:** 

The storage engine treats each record as an opaque binary blob. 
The responsibility for correctly ordering the columns during serialization (for `INSERT`) and interpreting them during deserialization (for `SELECT`) lies with the `Executor`, which uses the ordered list of columns from the table's schema in the `Catalog`.

## `INSERT` Logic
1.  **Identify Target Page:** Always attempt to insert into the last page of the table's `.data` file.
2.  **Read Page:** Read the target page from disk into an in-memory buffer.
3.  **Check Space:** Check if the page has enough free space for the new serialized record data plus a new 4-byte slot.
4.  **Handle Two Cases:**
    *   **Case A (Record Fits):** Write the record data into the free space area (from the end, backwards). Add a new slot to the slot array. Update the page header (`num_records`, `free_space_ptr`). Write the modified page back to disk at the same location.
    *   **Case B (Record Doesn't Fit):** Create a brand new, empty page in memory. Place the record there. Append this new page to the end of the table's `.data` file, causing the file to grow.

---

## FAQ & Design Decisions

#### Q: Is this "slotted page" model a standard?
**A:** Yes, it's a classic and fundamental database design pattern, heavily inspired by academic work and used in various forms by major databases.
*   **PostgreSQL:** Uses a nearly identical model with a page header, an array of "line pointers" (their name for slots), and record data called "tuples".
*   **Oracle:** Also employs a very similar slotted page architecture.
*   **MySQL (InnoDB) & SQLite:** Use a more specialized version of this model within their B-Tree leaf pages, where the data is stored.
By implementing this, we are building a simplified but authentic version of a real database storage page.

#### Q: How do indexes relate to the slot array?
**A:** They are separate concepts that work together. The slot array's order is purely physical (the order records were inserted onto the page), not logical. An index provides the logical ordering.
*   **The Index:** A separate data structure (like a B-Tree) that maps a key value (e.g., a user's name) to a **Row ID (RID)**.
*   **The Row ID (RID):** A pointer to the exact physical location of a record. It's a pair of numbers: `<PageID, SlotNum>`.
*   **The Workflow:** To find a record via an index, the engine searches the index to get the RID. Then, it uses the `PageID` to fetch the correct page and the `SlotNum` to find the record's exact location on that page via the slot array. This separation allows a single table's data (a "heap file" of slotted pages) to have many different indexes.

#### Q: Once a record's slot is assigned, does it ever move?
**A:** The **slot number is stable**, but the **data it points to can move**. This is key for handling updates. If a record is updated and its size changes (e.g., `UPDATE users SET name = 'Robert' WHERE name = 'Bob'`), the new, larger data won't fit in the old space. The engine will find a new location on the page for the updated record and change the `record_offset` in the original slot to point to this new location. The old data space becomes a "hole". This can lead to page fragmentation, which real databases manage with a `VACUUM` or compaction process to reclaim space.

#### Q: What happens when we delete a record?
**A:** We must not change the slot numbers of other records, as this would invalidate all indexes. The correct approach is to **"tombstone" the slot**. We can mark the slot as empty and reusable by setting its `record_offset` and `record_length` to a special value (like 0). The space on the page that the record's data occupied is now considered free and can be reclaimed by a future `INSERT` or by a compaction process.

#### Q: How can we change the page structure in the future?
**A:** This is a very difficult problem called "format evolution," and the key is to plan for it from the start. This is why we include a **`page_version` number in the header**.
*   **V1 (Now):** We ship with `page_version = 1`.
*   **V2 (Future):** If we need to change the header (e.g., add a new flag), we create a V2 format.
*   **Backward Compatibility:** The database code must be updated to handle both versions. When it reads a page, it first checks the version number and then uses the appropriate logic (V1 or V2) to parse the rest of the page.
*   **Upgrade Path:** Pages can be upgraded from V1 to V2 on-the-fly (when they are next written to) or via a dedicated offline `upgrade` utility for major changes. Without a version number from the beginning, this becomes nearly impossible to do safely.