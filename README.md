# memory_view

A convenient memory view interface that allows different implementations, including lazy loading from disk, in-memory buffers, and mapped files.

# Documentation

``` cpp
namespace mv
{
    class handler;

    template <typename T>
    class span;

    struct begin_t;

    begin_t begin;

    struct end_t;

    end_t end;

    class memory_view;

    class ptr_memory_source;

    template <typename Container>
    mv::memory_view make_memory_view(Container const& container);

    template <typename T, typename Container>
    mv::memory_view make_memory_view(Container const& container);
}
```

### Class `mv::handler`

``` cpp
class handler
{
};
```

A handler manages the lifetime of a fetched memory `span`.

When dealing with static memory, such as an in-memory array or a memory mapped file, the default empty object can be used. When managing memory by lazy loading or caching, this object defines the lifetime: once the handler object goes out of scope the memory pointed by the span can be invalidated. Effectively, this means that there is no guarantee that the memory pointed to by a span is valid after the span goes out of scope.

-----

### Class `mv::span`

``` cpp
template <typename T>
class span
: public gsl::span<const T>
{
public:
    span(T const* ptr, std::ptrdiff_t count, std::shared_ptr<handler> handler = nullptr);

    friend class memory_view;
};
```

A span object points to a valid contiguous memory area.

-----

### Struct `mv::begin_t`

``` cpp
struct begin_t
{
};
```

Represents the left end of a range when slicing.

-----

### Struct `mv::end_t`

``` cpp
struct end_t
{
};
```

Represents the right end of a range when slicing.

-----

### Class `mv::memory_view`

``` cpp
class memory_view
{
public:
    template <typename source_type>
    memory_view(source_type source);

    memory_view() = default;

    ~memory_view() = default;

    memory_view(mv::memory_view const&) = default;

    memory_view(mv::memory_view&&) noexcept = default;

    mv::memory_view& operator=(mv::memory_view const&) = default;

    mv::memory_view& operator=(mv::memory_view&&) noexcept = default;

    template <typename T>
    T as() const;

    template <typename T>
    span<T> as_span() const;

    template <typename ... T>
    std::tuple<T...> unpack() const;

    template <typename ... T>
    std::tuple<T..., memory_view> unpack_head() const;

    std::size_t size() const;

    bool empty() const;

    mv::memory_view operator()(std::ptrdiff_t first, std::ptrdiff_t last) const;

    mv::memory_view operator()(std::ptrdiff_t first, mv::end_t) const;

    mv::memory_view operator()(mv::begin_t, std::ptrdiff_t last) const;
};
```

A view to an arbitrary memory buffer, such as array, or memory mapped file.

In a lazy memory source, any data access will cause to fetch the underlying data. No data is accessed when slicing.

### Constructor `mv::memory_view::memory_view`

``` cpp
template <typename source_type>
memory_view(source_type source);
```

Creates a memory view from a memory source.

### Template parameter `mv::memory_view::source_type`

``` cpp
typename source_type
```

This type must provide the following methods:

    span<char> as_span(std::ptrdiff_t begin, std::ptrdiff_t end) const;
    std::size_t size() const;

-----

-----

### Function `mv::memory_view::as`

``` cpp
template <typename T>
T as() const;
```

Casts data to the given type, starting at the beginning of the range.

-----

### Function `mv::memory_view::as_span`

``` cpp
template <typename T>
span<T> as_span() const;
```

Returns a span of the given type.

-----

### Function `mv::memory_view::unpack`

``` cpp
template <typename ... T>
std::tuple<T...> unpack() const;
```

Unpacks the memory into several variables, returned as a tuple.

Example:

    auto [n, ch, arr] = mv.unpack<int, char, std::array<char, 5>>();

-----

### Function `mv::memory_view::unpack_head`

``` cpp
template <typename ... T>
std::tuple<T..., memory_view> unpack_head() const;
```

Unpacks leading values and returns them along with the remaining slice.

-----

### Function `mv::memory_view::size`

``` cpp
std::size_t size() const;
```

*Returns:* The size of the view.

-----

### Function `mv::memory_view::empty`

``` cpp
bool empty() const;
```

Equivalent to `size() == 0`

-----

### Function `mv::memory_view::operator()`

``` cpp
mv::memory_view operator()(std::ptrdiff_t first, std::ptrdiff_t last) const;
```

Returns a slice `[first, last)`.

-----

### Function `mv::memory_view::operator()`

``` cpp
mv::memory_view operator()(std::ptrdiff_t first, mv::end_t) const;
```

Returns a slice from `first` to the end of the view.

-----

### Function `mv::memory_view::operator()`

``` cpp
mv::memory_view operator()(mv::begin_t, std::ptrdiff_t last) const;
```

Returns a slice from the beginning of the view to last (exclusive).

-----

-----

### Class `mv::ptr_memory_source`

``` cpp
class ptr_memory_source
{
public:
    ptr_memory_source(char const* ptr, std::size_t size);

    template <int size>
    ptr_memory_source(std::array<char, size> const& arr);

    template <typename ContiguousContainer>
    ptr_memory_source(ContiguousContainer const& container);

    ptr_memory_source() = default;

    ~ptr_memory_source() = default;

    ptr_memory_source(mv::ptr_memory_source const&) = default;

    ptr_memory_source(mv::ptr_memory_source&) = default;

    mv::ptr_memory_source& operator=(mv::ptr_memory_source const&) = default;

    mv::ptr_memory_source& operator=(mv::ptr_memory_source&&) = default;

    span<char> const as_span(std::ptrdiff_t begin, std::ptrdiff_t end) const;

    std::size_t size() const;
};
```

Memory source based on an existing contiguous memory area.

**Warning**: any data passed to the constructors must be valid for the entire lifetime of the memory source. It must be enfoced by the user. In the future, this will be deprecated in favor of a lifetime-aware solution (although most likely this will still be available to use with a warning in place).

-----
