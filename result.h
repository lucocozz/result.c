#ifndef RESULT_H
#define RESULT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <errno.h>

// ============= Panic Handling =============

#ifndef PANIC
_Noreturn static inline void _panic_internal(const char *msg, const char *file, int line) {
    fprintf(stderr, "PANIC: %s (%s:%d)\n", msg, file, line);
    abort();
}
#define PANIC(msg) _panic_internal(msg, __FILE__, __LINE__)
#endif


// ============= Error Handling =============

typedef struct {
    int         domain_id;
    const char *domain_name;
    int         raw_code;
    int         type_code;
    const char *message;
} Error;

typedef struct {
    int         raw_code;
    int         type_code;
    const char *message;
} ErrorInfo;

typedef struct {
    int              domain_id;
    const char      *domain_name;
    const ErrorInfo *errors;
} ErrorDomain;


#define ERROR(name, _code, _message) [name] = {.raw_code = _code, .type_code = name, .message = _message}

#define DEFINE_ERROR_DOMAIN(name, id, ...) \
    static const ErrorInfo name##_ERRORS[] = { __VA_ARGS__ }; \
    static const ErrorDomain name##_DOMAIN = { \
        .domain_id = id, \
        .domain_name = #name, \
        .errors = name##_ERRORS \
    }


// ============= Result Handling =============

#define RESULT_TYPE(Typename, Type) \
    typedef struct { \
        bool _is_ok;     \
        union { \
            Type value; \
            Error error; \
        }; \
    } Typename##Result

#define Ok(Typename, Value) ((Typename##Result){ ._is_ok = true, .value = (Value) })

#define Fail(ResultType, DomainObject, ErrCode) \
    ((ResultType##Result){ ._is_ok = false, .error = { \
        .domain_id = (DomainObject).domain_id, \
        .domain_name = (DomainObject).domain_name, \
        .raw_code = (DomainObject).errors[ErrCode].raw_code, \
        .type_code = (DomainObject).errors[ErrCode].type_code, \
        .message = (DomainObject).errors[ErrCode].message \
    } })

#define Propagate(Typename, ErrStruct) ((Typename##Result){ ._is_ok = false, .error = (ErrStruct) })

#define Result(Typename) Typename##Result

#define is_ok(result) ((result)._is_ok)
#define is_error(result) (!(result)._is_ok)

#define unwrap_ok(result) \
    (is_ok(result) ? (result).value : (PANIC("Called unwrap_ok() on an Error value"), (result).value))

#define expect_ok(result, message) \
    (is_ok(result) ? (result).value : (PANIC(message), (result).value))

#define unwrap_error(result) \
    (is_error(result) ? (result).error : (PANIC("Called unwrap_error() on an Ok value"), (result).error))

#define error_msg(result) (unwrap_error(result).message)
#define error_domain(result) (unwrap_error(result).domain_name)

#define or_ok(result, default_val) \
    (is_ok(result) ? unwrap_ok(result) : (default_val))

#define free_result(result_var, free_func) \
    do { \
        if (is_ok(result_var)) \
            free_func(unwrap_ok(result_var)); \
    } while(0)

#define TRY_OK(Typename, target_var, res_expr) \
    do { \
        Result(Typename) res = (res_expr); \
        if (is_error(res)) \
            return Propagate(Typename, unwrap_error(res)); \
        target_var = unwrap_ok(res); \
    } while(0)

// ============= Optional Handling =============

#define OPTIONAL_TYPE(Typename, Type) \
    typedef struct { \
        bool _is_some;     \
        Type value; \
    } Typename##Optional

#define Some(Typename, Value) ((Typename##Optional){ ._is_some = true, .value = (Value) })
#define None(Typename) ((Typename##Optional){ ._is_some = false})
#define Optional(Typename) Typename##Optional

#define is_some(Varname) ((Varname)._is_some)
#define is_none(Varname) (!(Varname)._is_some)

#define unwrap_some(Varname) \
    (is_some(Varname) ? (Varname).value : (PANIC("Called unwrap_some() on a None value"), (Varname).value))

#define expect_some(Varname, message) \
    (is_some(Varname) ? (Varname).value : (PANIC(message), (Varname).value))

#define or_some(Varname, default_value) \
    (is_some(Varname) ? unwrap_some(Varname) : (default_value))

#define free_optional(optional_var, free_func) \
    do { \
        if (is_some(optional_var)) \
            free_func(unwrap_some(optional_var)); \
    } while(0)

#define TRY_SOME(Typename, target_var, opt_expr) \
    do { \
        Optional(Typename) opt = (opt_expr); \
        if (is_none(opt)) \
            return None(Typename); \
        target_var = unwrap_some(opt); \
    } while(0)

// ============= Pre-defined Types =============

OPTIONAL_TYPE(Char, char);
OPTIONAL_TYPE(UChar, unsigned char);
OPTIONAL_TYPE(Short, short);
OPTIONAL_TYPE(UShort, unsigned short);
OPTIONAL_TYPE(Int, int);
OPTIONAL_TYPE(UInt, unsigned int);
OPTIONAL_TYPE(Long, long);
OPTIONAL_TYPE(ULong, unsigned long);
OPTIONAL_TYPE(LongLong, long long);
OPTIONAL_TYPE(ULongLong, unsigned long long);
OPTIONAL_TYPE(Float, float);
OPTIONAL_TYPE(Double, double);
OPTIONAL_TYPE(String, const char*);
OPTIONAL_TYPE(VoidPtr, void*);

RESULT_TYPE(Char, char);
RESULT_TYPE(UChar, unsigned char);
RESULT_TYPE(Short, short);
RESULT_TYPE(UShort, unsigned short);
RESULT_TYPE(Int, int);
RESULT_TYPE(UInt, unsigned int);
RESULT_TYPE(Long, long);
RESULT_TYPE(ULong, unsigned long);
RESULT_TYPE(LongLong, long long);
RESULT_TYPE(ULongLong, unsigned long long);
RESULT_TYPE(Float, float);
RESULT_TYPE(Double, double);
RESULT_TYPE(String, const char*);
RESULT_TYPE(VoidPtr, void*);

RESULT_TYPE(CharOptional, CharOptional);
RESULT_TYPE(UCharOptional, UCharOptional);
RESULT_TYPE(ShortOptional, ShortOptional);
RESULT_TYPE(UShortOptional, UShortOptional);
RESULT_TYPE(IntOptional, IntOptional);
RESULT_TYPE(UIntOptional, UIntOptional);
RESULT_TYPE(LongOptional, LongOptional);
RESULT_TYPE(ULongOptional, ULongOptional);
RESULT_TYPE(LongLongOptional, LongLongOptional);
RESULT_TYPE(ULongLongOptional, ULongLongOptional);
RESULT_TYPE(FloatOptional, FloatOptional);
RESULT_TYPE(DoubleOptional, DoubleOptional);
RESULT_TYPE(StringOptional, StringOptional);
RESULT_TYPE(VoidPtrOptional, VoidPtrOptional);

#endif // RESULT_H