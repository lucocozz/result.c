#include <stdio.h>
#include <string.h>
#define RESULT_FEATURE_COLOR
#include "../result.h"

enum AppErrorCodes {
    APP_ERR_INVALID_USER,
    APP_ERR_INSUFFICIENT_PERMS
};
DEFINE_ERROR_DOMAIN(APP, 7,
    ERROR(APP_ERR_INVALID_USER, 100, "Invalid user"),
    ERROR(APP_ERR_INSUFFICIENT_PERMS, 101, "Insufficient permissions")
);

Result(Int) get_user_score(const char *username, const char *requested_by)
{
    if (strcmp(username, "luc") == 0)
    {
        if (strcmp(requested_by, "admin") != 0) {
            // 2. Using Fail_fmt to add context to a specific error
            return Fail_fmt(Int, APP_DOMAIN, APP_ERR_INSUFFICIENT_PERMS,
                "User '%s' does not have permission to access scores.", requested_by);
        }
        return Ok(Int, 100);
    }
    // 3. Using Fail_fmt with a more generic error
    return Fail_fmt(Int, STANDARD_DOMAIN, STD_ERR_NOT_FOUND, "User with name '%s' was not found in the system.", username);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <username> <requested_by>\n", argv[0]);
        return 1;
    }

    printf("Attempting to get score for user '%s' as requested by '%s'...\n", argv[1], argv[2]);
    Result(Int) score_res = get_user_score(argv[1], argv[2]);

    if (is_error(score_res)) {
        print_error_chain(stderr, unwrap_error(score_res));
        return 1;
    }

    printf("\nSuccess! Score is: %d\n", unwrap_ok(score_res));

    // 4. Demonstrate truncation
    printf("\n--- Demonstrating message truncation ---\n\n");

    const char *long_string = ("This is a very long string designed to be over "
    "128 characters to properly test the truncation logic that we have "
    "implemented. By making it excessively long, we ensure that vsnprintf will "
    "report a required length greater than the buffer and our truncation "
    "indicator will be appended correctly.");

    Result(Void) truncation_res = Fail_fmt(Void, STANDARD_DOMAIN, STD_ERR_GENERIC, "Error with a long message: %s", long_string);
    print_error_chain(stderr, unwrap_error(truncation_res));

    return 0;
}
