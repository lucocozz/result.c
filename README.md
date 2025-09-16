# result.h

A header-only C library implementing `Result` and `Optional` types inspired by Rust, for modern and safe error handling in C.

## Features

- **Result types**: Explicit error handling without exceptions
- **Optional types**: Type-safe optional values
- **Error chaining**: Full error traceability with stack traces
- **Error domains**: Classification and organization of errors
- **Header-only**: Easy integration

## Quick Usage

### Result

```c
#include "result.h"

RESULT_TYPE(Int, int);

Result(Int) divide(int a, int b)
{
    if (b == 0)
        return Fail(Int, MATH_DOMAIN, MATH_ERR_DIV_BY_ZERO);
    return Ok(Int, a / b);
}

int main()
{
    Result(Int) result = divide(10, 2);
    
    if (is_ok(result))
        printf("Result: %d\n", unwrap_ok(result));
    else
        print_error_chain(stderr, unwrap_error(result));

    return 0;
}
```

### Optional

```c
#include "result.h"

OPTIONAL_TYPE(Int, int);

Optional(Int) find_value(int arr[], size_t len, int target)
{
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == target)
            return Some(Int, arr[i]);
    }
    return None(Int);
}

int main()
{
    int arr[] = {1, 2, 3, 4, 5};
    Optional(Int) result = find_value(arr, 5, 3);
    
    int value = or_some(result, -1);
    printf("Found value: %d\n", value);
    
    return 0;
}
```

## Build

```bash
meson setup builddir
meson compile -C builddir

./builddir/result_example filename.txt
./builddir/optional_example 42
./builddir/chaining_example
./builddir/map_example
```

## API Reference

### Types Result

- `Ok(Type, value)` - Creates a successful result
- `Fail(Type, domain, code)` - Creates an error result
- `is_ok(result)` / `is_error(result)` - State checking
- `unwrap_ok(result)` - Extracts value (panics on error)
- `or_ok(result, default)` - Returns value or default
- `TRY(Type, var, expr)` - Error propagation

### Types Optional

- `Some(Type, value)` - Present value
- `None(Type)` - Absent value
- `is_some(opt)` / `is_none(opt)` - Presence checking
- `unwrap_some(opt)` - Extracts value (panics if absent)
- `or_some(opt, default)` - Returns value or default

## Examples

See the `examples/` directory for complete usage examples:

- `result.c` - File error handling
- `optional.c` - Optional values
- `chaining.c` - Error chaining
- `map.c` - Result transformation

## License

MIT License - See [LICENSE](LICENSE) for details.
