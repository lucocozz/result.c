#include <stdio.h>
#include <stdlib.h>
#define RESULT_FEATURE_COLOR
#include "../result.h"


int square(int x) {
    return x * x;
}

Result(Int) divide(int a, int b)
{
    if (b == 0)
        return Fail(Int, MATH_DOMAIN, MATH_ERR_DIV_BY_ZERO);
    return Ok(Int, a / b);
}

int main(void)
{
    printf("Test Case 1: divide(10, 2) and map square\n");
    printf("-----------------------------------------\n");
    Result(Int) res_div_ok = divide(10, 2);
    Result(Int) squared_result_ok = MAP_RESULT(Int, Int, res_div_ok, square);

    if (is_ok(squared_result_ok)) {
        printf("Original division result: %d\n", unwrap_ok(res_div_ok));
        printf("Mapped squared result: %d\n", unwrap_ok(squared_result_ok));
    }
    else
        print_error_chain(stderr, unwrap_error(squared_result_ok));


    printf("\nTest Case 2: divide(10, 0) and map square\n");
    printf("------------------------------------------\n");
    Result(Int) res_div_err = divide(10, 0);
    Result(Int) squared_result_err = MAP_RESULT(Int, Int, res_div_err, square);

    if (is_ok(squared_result_err)) {
        printf("Original division result: %d\n", unwrap_ok(res_div_err));
        printf("Mapped squared result: %d\n", unwrap_ok(squared_result_err));
    }
    else
        print_error_chain(stderr, unwrap_error(squared_result_err));

    return 0;
}
