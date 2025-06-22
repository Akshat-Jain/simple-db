# C++ Smart Pointers: `std::unique_ptr` and the Operator Pipeline

`std::unique_ptr` is a modern C++ tool for managing ownership of dynamically allocated memory (memory from `new`). It guarantees that objects are always destroyed correctly, preventing memory leaks and crashes. This is essential for building our Volcano operator pipeline.

## The Core Problem: Object Lifetime and Dangling Pointers

In our database, the query planner creates a pipeline of operators. A `ProjectionOperator` needs to get rows from a `TableScanOperator`. The `ProjectionOperator` needs to be sure that its child, the `TableScanOperator`, stays alive as long as it does.

### The Wrong Way: Using a Raw Pointer or Reference

Let's see what happens if the `ProjectionOperator` just holds a reference to its child.

```cpp
// In ProjectionOperator.h
class ProjectionOperator : public Operator {
    Operator& child_; // Holds a reference, does NOT own the child.
public:
    ProjectionOperator(Operator& child, ...) : child_(child) {}
    // ...
};

// In the planner function (e.g., executor.cpp)
std::unique_ptr<Operator> plan_select(...) {
    // 1. A TableScanOperator is created. It is owned by the
    //    local unique_ptr 'scan_op'.
    auto scan_op = std::make_unique<TableScanOperator>(...);

    // 2. A ProjectionOperator is created. It is given a reference
    //    to the TableScanOperator object. It does not own it.
    auto proj_op = std::make_unique<ProjectionOperator>(*scan_op, ...);

    return proj_op;
} // 3. The function ends. The local variable 'scan_op' is destroyed.
  //    Because it's a unique_ptr, it DELETES the TableScanOperator.
  //    The 'proj_op' we returned is now holding a DANGLING reference
  //    to memory that has been freed. This will crash the program.
```

This is a classic lifetime bug. The child operator died before its parent was done with it.

## The Solution: `std::unique_ptr` for an Ownership Chain

`std::unique_ptr` solves this by creating a clear chain of ownership. The parent operator will **own** its child.

### Key Properties of `std::unique_ptr`

1.  **Exclusive Ownership:** Only one `unique_ptr` can own an object. You cannot copy them. This prevents two operators from trying to delete the same child.
2.  **Automatic Cleanup (RAII):** When a `unique_ptr` is destroyed, it automatically `delete`s the object it owns.
3.  **Transferable Ownership:** You can move ownership from one `unique_ptr` to another using `std::move`. This is the key to building our pipeline.

### The Right Way: Building the Pipeline with `std::move`

We design the `ProjectionOperator` to take ownership of its child.

```cpp
// In ProjectionOperator.h
class ProjectionOperator : public Operator {
    // This unique_ptr OWNS the child operator.
    std::unique_ptr<Operator> child_;
public:
    // The constructor takes ownership by moving the child pointer.
    ProjectionOperator(std::unique_ptr<Operator> child, ...)
        : child_(std::move(child)) {}
    // ...
};
```

Now, the planner function works correctly and safely.

```cpp
// In the planner function (e.g., executor.cpp)
std::unique_ptr<Operator> plan_select(...) {
    // 1. A TableScanOperator is created, owned by 'scan_op'.
    auto scan_op = std::make_unique<TableScanOperator>(...);

    // 2. A ProjectionOperator is created. We TRANSFER OWNERSHIP of the
    //    TableScanOperator from 'scan_op' to the new operator
    //    using std::move.
    auto proj_op = std::make_unique<ProjectionOperator>(std::move(scan_op), ...);

    // 3. After the move, 'scan_op' is now empty (nullptr).
    //    The 'proj_op' now officially owns the TableScanOperator.

    return proj_op;
} // 4. The function ends. The local variable 'scan_op' is destroyed,
  //    but since it's empty, nothing happens. The TableScanOperator
  //    is NOT deleted because its ownership was transferred.
```

When the `main` loop is finished with the query, the `unique_ptr` holding the `proj_op` will be destroyed. This will trigger a cascade:
1.  The `ProjectionOperator` is destroyed.
2.  Its destructor destroys its `child_` member (`unique_ptr`).
3.  The `child_` unique_ptr deletes the `TableScanOperator`.

Everything is cleaned up automatically, in the correct order, with no leaks or dangling pointers. This is the safe, modern C++ way to manage object hierarchies like our operator pipeline.
