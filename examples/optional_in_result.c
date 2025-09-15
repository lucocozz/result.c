#include "../result.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define the optional type for a page
OPTIONAL_TYPE(Page, char*);

// Define the Result type for the API call
RESULT_TYPE(API, Optional(Page));

// Define error codes for the 'API' domain
typedef enum {
    API_ERR_INTERNAL_SERVER = 0,
    API_ERR_NOT_FOUND = 1,
    API_ERR_UNAUTHORIZED = 2,
} ApiErrorCode;

// Define the ErrorDomain object for 'API' errors
DEFINE_ERROR_DOMAIN(API, 2,
    ERROR(API_ERR_INTERNAL_SERVER, 500, "500 Internal Server Error"),
    ERROR(API_ERR_NOT_FOUND, 404, "404 Not Found"),
    ERROR(API_ERR_UNAUTHORIZED, 401, "401 Unauthorized")
);

Result(API) fetch_page()
{
    int api_state = rand() % 3;
    if (api_state == 0)
        return Fail(API, API_DOMAIN, API_ERR_INTERNAL_SERVER);
    else if (api_state == 1)
        return Ok(API, Some(Page, "https://example.com"));
    return Ok(API, None(Page));
}

int main()
{
    srand(time(NULL));
    printf("Fetching page...\n");

    Result(API) result = fetch_page();
    if (is_error(result)) {
        printf("%s Error: %s\n", error_domain(result), error_msg(result));
        return 1;
    }

    Optional(Page) page = unwrap_ok(result);
    if (is_some(page))
        printf("Page fetched successfully: %s\n", unwrap_some(page));
    else
        printf("No page found.\n");

    return 0;
}