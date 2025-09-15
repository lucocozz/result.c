#include "../result.h"
#include <stdio.h>
#include <string.h>

// === Example for TRY_SOME ===

// Note: OPTIONAL_TYPE(String, ...) is now pre-defined in result.h

Optional(String) get_config_value(const char *key)
{
    if (strcmp(key, "user") == 0)
        return Some(String, "admin");
    return None(String);
}

Optional(String) get_permission_for_user(const char *user)
{
    if (strcmp(user, "admin") == 0)
        return Some(String, "read,write,execute");
    return None(String);
}

Optional(String) get_permissions_from_config_key(const char *key)
{
    const char *user;
    TRY_SOME(String, user, get_config_value(key)); // If None, propagate None

    const char* permissions;
    TRY_SOME(String, permissions, get_permission_for_user(user));

    return Some(String, permissions);
}


// === Example for TRY_OK ===

RESULT_TYPE(FileContent, const char*);

typedef enum { 
    MOCK_ERR_NOT_FOUND = 0, 
    MOCK_ERR_INVALID_CONTENT 
} MockFileError;

DEFINE_ERROR_DOMAIN(FileContent, 3,
    ERROR(MOCK_ERR_NOT_FOUND, 1, "File not found"),
    ERROR(MOCK_ERR_INVALID_CONTENT, 2, "Invalid content format")
);

Result(FileContent) read_mock_file(const char *path)
{
    if (strcmp(path, "/etc/config") == 0)
        return Ok(FileContent, "database_url=postgres://...");
    return Fail(FileContent, FileContent_DOMAIN, MOCK_ERR_NOT_FOUND);
}

Result(FileContent) get_database_url(const char *content)
{
    if (strncmp(content, "database_url=", 13) == 0)
        return Ok(FileContent, content + 13);
    return Fail(FileContent, FileContent_DOMAIN, MOCK_ERR_INVALID_CONTENT);
}

Result(FileContent) find_db_url_from_path(const char *path)
{
    const char *content;
    TRY_OK(FileContent, content, read_mock_file(path)); // If Error, propagate Error

    const char *url;
    TRY_OK(FileContent, url, get_database_url(content));

    return Ok(FileContent, url);
}


int main()
{
    printf("--- Chaining Optionals ---\n");
    Optional(String) perms1 = get_permissions_from_config_key("user");
    if (is_some(perms1))
        printf("Permissions for 'user': %s\n", unwrap_some(perms1));
    else
        printf("Could not get permissions for 'user'\n");

    Optional(String) perms2 = get_permissions_from_config_key("guest");
    if (is_some(perms2))
        printf("Permissions for 'guest': %s\n", unwrap_some(perms2));
    else
        printf("Could not get permissions for 'guest'\n");



    printf("\n--- Chaining Results ---\n");
    Result(FileContent) res1 = find_db_url_from_path("/etc/config");
    if (is_ok(res1))
        printf("Database URL: %s\n", unwrap_ok(res1));
    else
        printf("%s Error: %s\n", error_domain(res1), error_msg(res1));

    Result(FileContent) res2 = find_db_url_from_path("/etc/nonexistent");
    if (is_ok(res2))
        printf("Database URL: %s\n", unwrap_ok(res2));
    else
        printf("%s Error: %s\n", error_domain(res2), error_msg(res2));

    return 0;
}
