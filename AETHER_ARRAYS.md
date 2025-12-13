# Aether Array Implementation Plan

## Goal
Add array support to aether forms using `std::aligned_storage` for proper alignment with placement new.

## Implementation Tasks

### 1. Add bool_c to forms.hpp
```cpp
class bool_c : public monad_c<bool>, public numeric_if<bool> {
public:
    bool_c() = delete;
    constexpr bool_c(bool value) : monad_c<bool>(value) {}
    constexpr ~bool_c() = default;
    
    constexpr bool value() const noexcept { return this->data(); }
    
    constexpr bool_c operator&&(const bool_c& other) const noexcept;
    constexpr bool_c operator||(const bool_c& other) const noexcept;
    constexpr bool_c operator!() const noexcept;
    constexpr bool operator==(const bool_c& other) const noexcept;
};
```

### 2. Add array_c template to forms.hpp
```cpp
template<typename T>
concept array_element = (std::derived_from<T, numeric_if<std::uint8_t>> ||
                         std::derived_from<T, numeric_if<std::uint16_t>> ||
                         std::derived_from<T, numeric_if<std::uint32_t>> ||
                         std::derived_from<T, numeric_if<std::uint64_t>> ||
                         std::derived_from<T, numeric_if<float>> ||
                         std::derived_from<T, numeric_if<double>> ||
                         std::derived_from<T, numeric_if<bool>>);

template<array_element T>
class array_c {
private:
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    std::vector<storage_t> _storage;
    std::size_t _length;
    
public:
    array_c(std::size_t length);
    ~array_c();
    
    T& operator[](std::size_t idx);
    const T& operator[](std::size_t idx) const;
    std::size_t length() const noexcept;
};
```

### 3. Implement array_c methods
- Constructor: allocate storage, placement new all elements
- Destructor: explicitly call destructor for each element
- operator[]: launder and reinterpret_cast to access elements
- length(): return _length

### 4. Add tests to test_forms.cpp

**Test group: ArrayTests**
- CanConstructEmptyArray
- CanConstructArrayWithSize
- CanAccessArrayElements
- CanModifyArrayElements
- ArrayOfI8
- ArrayOfI16
- ArrayOfI32
- ArrayOfI64
- ArrayOfR32
- ArrayOfR64
- ArrayOfBool
- ArrayBoundsCheck (if implementing bounds checking)
- ArrayDestructorCalled (verify cleanup)

**Test group: StructTests**
- Define test struct: `test_point_t` with x, y coordinates (i32 x, i32 y)
- Implement `point_c : public struct_c<test_point_t>`
- CanConstructStruct
- CanAccessStructMembers
- CanModifyStructMembers
- StructSizeAndAlignment
- ArrayOfStructs (array_c<point_c> with multiple points)
- ArrayOfStructsAccessElements (verify each struct in array)
- ArrayOfStructsModifyElements (modify structs in array)
- StructInStruct (nested structs: test_rect_t with two test_point_t)
- ArrayOfNestedStructs (array of rectangles)

### 5. Add struct_if interface for user-defined types
```cpp
class struct_if {
public:
    virtual ~struct_if() = default;
    virtual std::size_t size_bytes() const noexcept = 0;
    virtual std::size_t alignment() const noexcept = 0;
    virtual void* data_ptr() noexcept = 0;
    virtual const void* data_ptr() const noexcept = 0;
};
```

### 6. Add struct_c wrapper for user-defined structs
```cpp
template<typename T>
class struct_c : public struct_if {
private:
    using storage_t = std::aligned_storage_t<sizeof(T), alignof(T)>;
    storage_t _storage;
    bool _constructed;
    
public:
    struct_c();
    template<typename... Args>
    explicit struct_c(Args&&... args);
    ~struct_c();
    
    T& get();
    const T& get() const;
    
    std::size_t size_bytes() const noexcept override;
    std::size_t alignment() const noexcept override;
    void* data_ptr() noexcept override;
    const void* data_ptr() const noexcept override;
};
```

### 7. Add array_c specialization for struct_if types
```cpp
template<std::derived_from<struct_if> T>
class array_c<T> {
private:
    std::vector<std::unique_ptr<T>> _elements;
    std::size_t _length;
    
public:
    array_c(std::size_t length);
    T& operator[](std::size_t idx);
    const T& operator[](std::size_t idx) const;
    std::size_t length() const noexcept;
};
```

### 8. Optional: Add array_2d_c for multi-dimensional arrays
Similar structure but with `_rows` and `_cols`, `at(row, col)` accessor.

## Key Implementation Details

- Use `std::launder` when accessing elements via reinterpret_cast
- Explicitly call destructors in ~array_c() using `std::destroy_at`
- Storage vector handles alignment automatically
- Each element constructed with placement new at construction
- Default initialize elements to zero/false
- struct_if provides runtime polymorphism for user-defined types
- struct_c uses aligned_storage to wrap any user type
- Array of structs uses unique_ptr for proper lifetime management

## Files to Modify

1. `libs/aether/include/truk/aether/forms.hpp` - Add bool_c, struct_if, struct_c, array_c, array_2d_c
2. `libs/aether/tests/test_forms.cpp` - Add ArrayTests and StructTests groups with ~20 tests total
3. Update CMakeLists if needed (should auto-detect)

## Testing Strategy

Build and run: `cmake --build build --target test_forms && ./build/libs/aether/tests/test_forms`

Verify all tests pass including new array tests.
