#ifndef RESULT_H
#define RESULT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ============= Result Handling =============

#define DEF_ERR_ENUM(Name, code, msg) Name##Error_##code,
#define DEF_ERR_MSG(Name, code, msg) [Name##Error_##code] = msg,

#define RESULT_TYPE(Name, SuccessType, ErrorList) \
    typedef enum { \
        Name##Error_NONE = 0, \
        Name##Error_UNKNOWN_ERROR, \
        ErrorList(DEF_ERR_ENUM, Name) \
    } Name##Error; \
    static const char* Name##_messages[] = { \
        [Name##Error_NONE] = "No error", \
        [Name##Error_UNKNOWN_ERROR] = "Unknown error", \
        ErrorList(DEF_ERR_MSG, Name) \
    }; \
    static inline const char* Name##Error_to_string(Name##Error err) { \
        size_t max_idx = sizeof(Name##_messages)/sizeof(Name##_messages[0]); \
        if (err >= 0 && err < max_idx && Name##_messages[err] != NULL) \
            return Name##_messages[err]; \
        return "Unknown error"; \
    } \
    typedef struct { \
        bool is_ok; \
        union { \
            SuccessType ok; \
            Name##Error err; \
        } value; \
    } Name##Result


#define Ok(type, val) ((type##Result){ .is_ok = true, .value.ok = (val) })
#define Error(type, error) ((type##Result){ .is_ok = false, .value.err = (type##Error_##error) })
#define Result(name) name##Result

#define is_ok(result) ((result).is_ok)
#define is_error(result) (!(result).is_ok)
#define unwrap_ok(result) ((result).value.ok)
#define unwrap_error(result) ((result).value.err)
#define error_msg(Type, result) Type##Error_to_string(unwrap_error(result))

#define result_or(result, default_val) \
    (is_ok(result) ? unwrap_ok(result) : (default_val))

#define result_free(Type, result, free_func) \
    do { \
        if (is_ok(result)) { \
            free_func(unwrap_ok(result)); \
        } \
    } while(0)

// ============= Optional Handling =============

#define OPTIONAL_TYPE(Name, Type) \
    typedef struct { \
        bool is_some; \
        Type value; \
    } Name##Optional

#define Some(Name, val) ((Name##Optional){ .is_some = true, .value = (val) })
#define None(Name) ((Name##Optional){ .is_some = false})
#define Optional(name) name##Optional

#define is_some(opt) ((opt).is_some)
#define is_none(opt) (!(opt).is_some)
#define unwrap(opt) ((opt).value)
#define optional_or(opt, default_value) \
    (is_some(opt) ? unwrap(opt) : (default_value))

#define optional_free(Name, optional, free_func) \
    do { \
        if (is_some(optional)) { \
            free_func(unwrap(optional)); \
        } \
    } while(0)


#endif // RESULT_H
