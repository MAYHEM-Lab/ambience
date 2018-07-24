# Mandatory Error Checking

One of the most common sources of bugs in programs is unchecked
errors. Languages that support exceptions solve this issue by
walking up the stack whenever an error happens, and if a suitable
error handler is absent, crash the program.

Even though C++ supports exceptions, the implementations incur
too much space overhead in metadata to be acceptable in most
embedded systems, thus, we disable exceptions in the kernel part
of tos.

Without exceptions, the language doesn't help us with the error
checking mechanisms. We must use return values or output parameters
to denote errors, both of which are ineffective, ugly or both:

```cpp
// returns -1 upon failure
size_t read();

// returns garbage upon failure, but sets did_it_work to false
size_t read(bool& did_it_work);

// returns whether the operation succeeded
bool read(size_t& len);
```

None of the options above enforce the checking of the return value
nor produce a fatal error if the value isn't checked. The act of
ignoring the return value happens implicitly, potentially driving
the application to the realm of undefined behaviour. Obviously,
we can't have that.

## Expected

Fallible operations return instances of the class template 
`tos::expected<T, ErrT>`. An expected return value means that either 
the operation returns an object of `T` in the case of success, or it
returns an object of type `ErrT`, in the case of an error.

The trick is in the interface of this type. The public interface 
prevents the user code from accessing either the return value or the
error value directly. The user must use the `with` function in order 
to be able to access them:

```
with(fallible_func(), [&](auto& success_val){
    // use success
}, [](auto& error_val){
    // use error
});
```

The user cannot access the value without writing handlers for both 
the _expected_ flow, and the erroneous flow. The user can still ignore
the error value, but the act is now explicit. Ignoring one of the 
cases might be a common use case, and writing an empty lambda just to
ignore it may be too much to type, so tos provides a default function
object that ignores anything, called `tos::ignore`:

```
with(fallible_func(), [&](auto& success){
    // use success
}, tos::ignore);
```

The same behaviour, but the ignoring is still explicit!

The use of with blocks will obviously result in extreme nesting:

```
with(fallible1(), [&](auto& success1){
    with(fallible2(success1), [&](auto& success2){
        with(fallible3(success2), [&](auto& success3){
            // ... and so on ...
        }, tos::ignore);
    }, tos::ignore);
}, tos::ignore);
```

To help alleviate this, `with` acts like a visitor in variant based 
setups, that is it will also return the value returned by the handlers
from the with call:

```
auto res = with(fallible1(), [](auto& success){
    return success;
}, [](auto&){
    return some_default_value;
});

// use res
```

This is obviously only useful if there's always a valid, fallback value
to use instead of the success value. For such cases, an interface called
with_or is also useful:

```
auto res = with_or(fallible1(), some_default_value);
```

## Unexpected

That is the user part of the story. How do we use this type in our 
libraries?

The type is pretty simple to use. Just declare the function to return
an object of type `expected<T, ErrT>` where `T` is the expected type and
`ErrT` is the error type.

In the implementation, regular return values will denote a successful
operation, and the `expected` type will try to return a `T` from the 
return expression:

```
enum class foo_errors
{
    unknown
};

expected<int, foo_errors> foo()
{
    return 42; // denotes success
}
```

Returning errors is a bit more ceremonious. You might expect returning an
object of type `ErrT` should just denote an error, but since the expected 
type and the error type may well be the same type, in the general case,
looking at the type of the return value would be ambiguous:

```
expected<int, int> foo()
{
    if (errno == ERR_OK)
    {
        return 42;
    }
    
    return errno; // I want this to be an error!
}
```

The proper way is just to let the `expected<T, ErrT>` type know that 
we are returning something unexpected:

```
expected<int, int> foo()
{
    if (errno == ERR_OK)
    {
        return 42;
    }
    
    return unexpected(errno); // This is an error!
}
```

For consistency, even if the types of the expected and the error values
are different, we expect the user to use the `unexpected` function to
return error values.

## Disclaimer

The template `tos::expected<T, ErrT>` is inspired by the proposal to add 
`std::expected<T, ErrT>` to the C++ standard, but deviates heavily in the 
interface and behaviour.