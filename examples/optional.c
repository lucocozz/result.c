#include "../result.h"


Optional(Int) find_value_in_array(int value)
{
    static int values[] = {-1, 0, 1,    3, 4, 8};
    //                      ^  ^  ^ values generally used as error codes when a value is not found
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        if (values[i] == value)
            return Some(Int, value);
    }
    return None(Int);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: %s <value>\n", argv[0]);
        return 1;
    }

    int value = atoi(argv[1]);
    Optional(Int) found = find_value_in_array(value);
    if (is_some(found))
        printf("✓ Value found: %d\n", unwrap_some(found));
    else
        printf("✗ Value not found\n");

    return 0;
}
