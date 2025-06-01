# C++: Copy Assignment vs. Move Assignment with `std::string` and `std::move()`

We have two `std::string` variables: `source_string` and `destination_string`. We want to assign the content of `source_string` to `destination_string`.

```cpp
#include <string>
#include <utility> // For std::move
#include <iostream>

void print_state(const std::string& s_name, const std::string& s_val, 
                 const std::string& d_name, const std::string& d_val) {
    std::cout << "  " << s_name << ": '" << s_val << "' (length: " << s_val.length() << ")\n";
    std::cout << "  " << d_name << ": '" << d_val << "' (length: " << d_val.length() << ")\n";
}
```

## Case A: Copy Assignment (Without std::move())

The `destination_string = source_string;` statement invokes the copy assignment operator.

```cpp
std::string source_string_copy = "A moderately long source string for copy.";
std::string destination_string_copy;

std::cout << "Initial state (Copy):\n";
print_state("source_string_copy", source_string_copy, "destination_string_copy", destination_string_copy);

std::cout << "\nPerforming COPY assignment: destination_string_copy = source_string_copy;\n";
destination_string_copy = source_string_copy;

std::cout << "\nAfter COPY assignment:\n";
print_state("source_string_copy", source_string_copy, "destination_string_copy", destination_string_copy);
```

## Steps for `destination_string_copy = source_string_copy;`:
1. Copy Assignment Operator Called: `std::string::operator=(const std::string& other)` is invoked on `destination_string_copy`.
2. Memory Allocation (Potentially): `destination_string_copy` allocates new memory if its current buffer isn't sufficient for `source_string_copy`'s content.
3. Content Copying: The character data from `source_string_copy` is copied into `destination_string_copy`'s buffer.
4. State Update: `destination_string_copy`'s length/capacity are updated.
5. `source_string_copy` Unchanged: `source_string_copy` retains its original content and resources.

## Output for Case A:

```
Initial state (Copy):
  source_string_copy: 'A moderately long source string for copy.' (length: 40)
  destination_string_copy: '' (length: 0)

Performing COPY assignment: destination_string_copy = source_string_copy;

After COPY assignment:
  source_string_copy: 'A moderately long source string for copy.' (length: 40)
  destination_string_copy: 'A moderately long source string for copy.' (length: 40)
```

Cost: Potentially expensive due to memory allocation and character-by-character copying.


## Case B: Move Assignment (With std::move())

The `destination_string = std::move(source_string);` statement invokes the move assignment operator.

```cpp
std::string source_string_move = "Another source string, this one for moving.";
std::string destination_string_move;

std::cout << "\nInitial state (Move):\n";
print_state("source_string_move", source_string_move, "destination_string_move", destination_string_move);

std::cout << "\nPerforming MOVE assignment: destination_string_move = std::move(source_string_move);\n";
destination_string_move = std::move(source_string_move);

std::cout << "\nAfter MOVE assignment:\n";
print_state("source_string_move (after move)", source_string_move, "destination_string_move", destination_string_move);
```

## Steps for `destination_string_move = std::move(source_string_move);`:
1. `std::move(source_string_move)`: Casts `source_string_move` to an rvalue reference (std::string&&), signaling its resources can be taken.
2. Move Assignment Operator Called: `std::string::operator=(std::string&& other)` is invoked on `destination_string_move`.
3. Resource Transfer: `destination_string_move` takes ownership of the internal data buffer (and other resources like capacity/size) from `source_string_move`. This is typically a shallow copy of pointers/members, not a deep copy of character data.
4. `source_string_move` State Change: `source_string_move` is left in a valid but unspecified state (for std::string, this usually means it becomes empty). It no longer owns the original character data.

## Output for Case B:

```
Initial state (Move):
  source_string_move: 'Another source string, this one for moving.' (length: 42)
  destination_string_move: '' (length: 0)

Performing MOVE assignment: destination_string_move = std::move(source_string_move);

After MOVE assignment:
  source_string_move (after move): '' (length: 0)
  destination_string_move: 'Another source string, this one for moving.' (length: 42)
```

Cost: Very cheap. Involves primarily updating internal pointers and size/capacity members, not copying the string content.


## Summary:

- Copy Assignment (`dest = src`): Creates a deep copy of the source's data. The source object remains unchanged. Can be expensive for large objects.
- Move Assignment (`dest = std::move(src)`): Transfers ownership of the source's resources to the destination. The source object is left in a valid but typically "empty" or unspecified state. Much more efficient for objects that manage heap-allocated resources.
- `std::move()`: Is a cast to an rvalue reference. It enables the selection of move constructors and move assignment operators. It signals that the object being moved from can have its resources pilfered.
