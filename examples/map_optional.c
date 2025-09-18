#include <stdio.h>
#include <string.h>
#include "../result.h"


int main()
{
    printf("--- Testing MAP_OPTIONAL ---\n");

    // Test 1: Map a Some to a Some
    Optional(String) some_str = Some(String, "hello");
    Optional(Int) len_some = MAP_OPTIONAL(Int, strlen, String, some_str);
    if (is_some(len_some))
        printf("Mapping Some(\"hello\") -> Some(%d)\n", unwrap_some(len_some));
    else
        printf("ERROR: Mapping a Some resulted in a None\n");

    // Test 2: Map a None to a None
    Optional(String) none_str = None(String);
    Optional(Int) len_none = MAP_OPTIONAL(Int, strlen, String, none_str);
    if (is_none(len_none))
        printf("Mapping None -> None\n");
    else
        printf("ERROR: Mapping a None resulted in a Some\n");

    return 0;
}
