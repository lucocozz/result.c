#ifndef RESULT_H
#define RESULT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <errno.h>
#include <stdatomic.h>

// ============= Configuration =============

#ifndef RESULT_ERROR_POOL_SIZE
#define RESULT_ERROR_POOL_SIZE 256
#endif

// ============= Panic Handling =============

#ifndef PANIC
_Noreturn static inline void _panic_internal(const char *msg, const char *file, int line) {
    fprintf(stderr, "PANIC: %s (%s:%d)\n", msg, file, line);
    abort();
}
#define PANIC(msg) _panic_internal(msg, __FILE__, __LINE__)
#endif


// ============= Error Handling =============

typedef struct Error {
    int         domain_id;
    const char *domain_name;
    int         raw_code;
    int         type_code;
    const char *message;

    const struct Error *cause;
    const char         *file;
    int                 line;
    const char         *func;
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

static Error result_error_pool[RESULT_ERROR_POOL_SIZE];
static _Atomic size_t result_error_pool_index = 0;

static inline const Error *_result_error_new(const Error *cause, const ErrorDomain *domain, int err_code, const char *file, int line, const char *func) {
    size_t index = atomic_fetch_add(&result_error_pool_index, 1);
    Error *new_err = &result_error_pool[index % RESULT_ERROR_POOL_SIZE];

    new_err->domain_id = domain->domain_id;
    new_err->domain_name = domain->domain_name;
    new_err->raw_code = domain->errors[err_code].raw_code;
    new_err->type_code = err_code;
    new_err->message = domain->errors[err_code].message;
    new_err->cause = cause;
    new_err->file = file;
    new_err->line = line;
    new_err->func = func;

    return new_err;
}

typedef enum {
    PRINT_ORDER_TOP_DOWN,
    PRINT_ORDER_BOTTOM_UP
} CResultPrintOrder;

static inline void print_error_chain(const Error *error, CResultPrintOrder order, FILE *stream)
{
    if (order == PRINT_ORDER_TOP_DOWN)
    {
        fprintf(stream, "Error trace (Top-Down):\n");
        bool is_cause = false;
        while (error != NULL)
        {
            if (is_cause)
                fprintf(stream, "Caused by: ");
            else
                fprintf(stream, "Error: ");

            fprintf(stream, "[%s] '%s' (%d)\n",
                    error->domain_name, error->message, error->raw_code);
            fprintf(stream, "    at %s() in %s:%d\n",
                    error->func, error->file, error->line);

            error = error->cause;
            is_cause = true;
        }
    }
    else
    {
        fprintf(stream, "Error trace (Bottom-Up):\n");
        const Error *chain[RESULT_ERROR_POOL_SIZE];
        int depth = 0;

        while (error != NULL && depth < RESULT_ERROR_POOL_SIZE) {
            chain[depth++] = error;
            error = error->cause;
        }

        for (int i = depth - 1; i >= 0; --i)
        {
            const Error *current = chain[i];
            if (i < depth - 1)
                fprintf(stream, "... which was wrapped by ...\n");
            fprintf(stream, "Error: [%s] '%s' (%d)\n",
                    current->domain_name, current->message, current->raw_code);
            fprintf(stream, "    at %s() in %s:%d\n",
                    current->func, current->file, current->line);
        }
    }
}


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
            const Error *error; \
        }; \
    } Typename##Result

#define Ok(Typename, Value) ((Typename##Result){ ._is_ok = true, .value = (Value) })

#define Fail(ResultType, DomainObject, ErrCode) \
    ((ResultType##Result){ ._is_ok = false, .error = _result_error_new(NULL, &(DomainObject), ErrCode, __FILE__, __LINE__, __func__) })

#define Propagate(Typename, ErrStructPtr) \
    ((Typename##Result){ ._is_ok = false, .error = _result_error_new(ErrStructPtr, &STANDARD_DOMAIN, STD_ERR_PROPAGATED, __FILE__, __LINE__, __func__) })

#define Result(Typename) Typename##Result

#define is_ok(result) ((result)._is_ok)
#define is_error(result) (!(result)._is_ok)

#define unwrap_ok(result) \
    (is_ok(result) ? (result).value : (PANIC("Called unwrap_ok() on an Error value"), (result).value))

#define expect_ok(result, message) \
    (is_ok(result) ? (result).value : (PANIC(message), (result).value))

#define unwrap_error(result) \
    (is_error(result) ? (result).error : (PANIC("Called unwrap_error() on an Ok value"), (result).error))

#define error_msg(result) (unwrap_error(result)->message)
#define error_domain(result) (unwrap_error(result)->domain_name)

#define or_ok(result, default_val) \
    (is_ok(result) ? unwrap_ok(result) : (default_val))

#define free_result(result_var, free_func) \
    do { \
        if (is_ok(result_var)) \
            free_func(unwrap_ok(result_var)); \
    } while(0)

#define TRY_CAST(EnclosingTypename, ExprTypename, target_var, res_expr) \
    do { \
        Result(ExprTypename) res = (res_expr); \
        if (is_error(res)) \
            return Propagate(EnclosingTypename, unwrap_error(res)); \
        target_var = unwrap_ok(res); \
    } while(0)

#define TRY(Typename, target_var, res_expr) \
    TRY_CAST(Typename, Typename, target_var, res_expr)

#define TRY_FAIL_CAST(EnclosingTypename, ExprTypename, target_var, res_expr, FailDomain, FailCode) \
    do { \
        Result(ExprTypename) res = (res_expr); \
        if (is_error(res)) { \
            const Error *cause = unwrap_error(res); \
            const Error *new_err = _result_error_new(cause, &(FailDomain), FailCode, __FILE__, __LINE__, __func__); \
            return ((EnclosingTypename##Result){ ._is_ok = false, .error = new_err }); \
        } \
        target_var = unwrap_ok(res); \
    } while(0)

#define TRY_FAIL(Typename, target_var, res_expr, FailDomain, FailCode) \
    TRY_FAIL_CAST(Typename, Typename, target_var, res_expr, FailDomain, FailCode)

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

// ============= Predefined Error Domains =============

enum StandardErrorCodes {
    STD_ERR_GENERIC = 0,
    STD_ERR_OUT_OF_MEMORY,
    STD_ERR_INVALID_ARGUMENT,
    STD_ERR_NULL_POINTER,
    STD_ERR_BUFFER_OVERFLOW,
    STD_ERR_NOT_FOUND,
    STD_ERR_ACCESS_DENIED,
    STD_ERR_TIMEOUT,
    STD_ERR_NOT_IMPLEMENTED,
    STD_ERR_PROPAGATED
};

DEFINE_ERROR_DOMAIN(STANDARD, 1,
    ERROR(STD_ERR_GENERIC, 0, "Generic error"),
    ERROR(STD_ERR_OUT_OF_MEMORY, ENOMEM, "Out of memory"),
    ERROR(STD_ERR_INVALID_ARGUMENT, EINVAL, "Invalid argument"),
    ERROR(STD_ERR_NULL_POINTER, EFAULT, "Null pointer"),
    ERROR(STD_ERR_BUFFER_OVERFLOW, EOVERFLOW, "Buffer overflow"),
    ERROR(STD_ERR_NOT_FOUND, ENOENT, "Not found"),
    ERROR(STD_ERR_ACCESS_DENIED, EACCES, "Access denied"),
    ERROR(STD_ERR_TIMEOUT, ETIMEDOUT, "Timeout"),
    ERROR(STD_ERR_NOT_IMPLEMENTED, ENOSYS, "Not implemented"),
    ERROR(STD_ERR_PROPAGATED, -1, "Error propagated")
);

enum IoErrorCodes {
    IO_ERR_FILE_NOT_FOUND = 0,
    IO_ERR_PERMISSION_DENIED,
    IO_ERR_FILE_EXISTS,
    IO_ERR_READ_FAILED,
    IO_ERR_WRITE_FAILED,
    IO_ERR_SEEK_FAILED,
    IO_ERR_DISK_FULL,
    IO_ERR_INVALID_PATH,
    IO_ERR_DEVICE_NOT_READY
};

DEFINE_ERROR_DOMAIN(IO, 2,
    ERROR(IO_ERR_FILE_NOT_FOUND, ENOENT, "File not found"),
    ERROR(IO_ERR_PERMISSION_DENIED, EACCES, "Permission denied"),
    ERROR(IO_ERR_FILE_EXISTS, EEXIST, "File already exists"),
    ERROR(IO_ERR_READ_FAILED, EIO, "Read operation failed"),
    ERROR(IO_ERR_WRITE_FAILED, EIO, "Write operation failed"),
    ERROR(IO_ERR_SEEK_FAILED, ESPIPE, "Seek operation failed"),
    ERROR(IO_ERR_DISK_FULL, ENOSPC, "Disk full"),
    ERROR(IO_ERR_INVALID_PATH, ENOTDIR, "Invalid path"),
    ERROR(IO_ERR_DEVICE_NOT_READY, ENODEV, "Device not ready")
);

enum NetworkErrorCodes {
    NET_ERR_CONNECTION_FAILED = 0,
    NET_ERR_CONNECTION_REFUSED,
    NET_ERR_CONNECTION_TIMEOUT,
    NET_ERR_HOST_NOT_FOUND,
    NET_ERR_NETWORK_UNREACHABLE,
    NET_ERR_PROTOCOL_ERROR,
    NET_ERR_INVALID_URL,
    NET_ERR_SSL_ERROR,
    NET_ERR_AUTHENTICATION_FAILED
};

DEFINE_ERROR_DOMAIN(NETWORK, 3,
    ERROR(NET_ERR_CONNECTION_FAILED, ECONNREFUSED, "Connection failed"),
    ERROR(NET_ERR_CONNECTION_REFUSED, ECONNREFUSED, "Connection refused"),
    ERROR(NET_ERR_CONNECTION_TIMEOUT, ETIMEDOUT, "Connection timeout"),
    ERROR(NET_ERR_HOST_NOT_FOUND, EHOSTUNREACH, "Host not found"),
    ERROR(NET_ERR_NETWORK_UNREACHABLE, ENETUNREACH, "Network unreachable"),
    ERROR(NET_ERR_PROTOCOL_ERROR, EPROTO, "Protocol error"),
    ERROR(NET_ERR_INVALID_URL, EINVAL, "Invalid URL"),
    ERROR(NET_ERR_SSL_ERROR, EPROTO, "SSL/TLS error"),
    ERROR(NET_ERR_AUTHENTICATION_FAILED, EACCES, "Authentication failed")
);

enum ParseErrorCodes {
    PARSE_ERR_INVALID_FORMAT = 0,
    PARSE_ERR_UNEXPECTED_CHARACTER,
    PARSE_ERR_UNEXPECTED_END,
    PARSE_ERR_NUMBER_TOO_LARGE,
    PARSE_ERR_NUMBER_TOO_SMALL,
    PARSE_ERR_INVALID_ESCAPE,
    PARSE_ERR_SYNTAX_ERROR,
    PARSE_ERR_ENCODING_ERROR
};

DEFINE_ERROR_DOMAIN(PARSE, 4,
    ERROR(PARSE_ERR_INVALID_FORMAT, EINVAL, "Invalid format"),
    ERROR(PARSE_ERR_UNEXPECTED_CHARACTER, EILSEQ, "Unexpected character"),
    ERROR(PARSE_ERR_UNEXPECTED_END, ENODATA, "Unexpected end of input"),
    ERROR(PARSE_ERR_NUMBER_TOO_LARGE, ERANGE, "Number too large"),
    ERROR(PARSE_ERR_NUMBER_TOO_SMALL, ERANGE, "Number too small"),
    ERROR(PARSE_ERR_INVALID_ESCAPE, EILSEQ, "Invalid escape sequence"),
    ERROR(PARSE_ERR_SYNTAX_ERROR, EINVAL, "Syntax error"),
    ERROR(PARSE_ERR_ENCODING_ERROR, EILSEQ, "Encoding error")
);

#endif // RESULT_H
