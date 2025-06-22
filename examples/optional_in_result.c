#include "../maybe.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

OPTIONAL_TYPE(Page, char *);

#define API_ERRORS(ErrDef, Name) \
    ErrDef(Name, 500_INTERNAL, "500 Internal server") \

RESULT_TYPE(API, Optional(Page), API_ERRORS);

Result(API) fetch_page()
{
    int api_state = rand() % 3;  // Simulate different API states
    if (api_state == 0)
        return Error(API, 500_INTERNAL);
    else if (api_state == 1)
        return Ok(API, Some(Page, "https://example.com"));
    return Ok(API, None(Page));
}

int main()
{
    srand(time(NULL));  // Initialize random number generator
    printf("Fetching page...\n");
    Result(API) result = fetch_page();
    if (is_error(result)) {
        printf("Error fetching page: %s\n", error_msg(API, result));
        return 1;
    }
    Optional(Page) page = unwrap_ok(result);
    if (is_some(page))
        printf("Page fetched successfully: %s\n", page.value);
    else
        printf("No page found.\n");
}
