#include "../result.h"
#include <string.h>

enum AppErrorCode { APP_ERR_INIT_FAILED };
DEFINE_ERROR_DOMAIN(APP, 5,
    ERROR(APP_ERR_INIT_FAILED, 501, "Application initialization failed")
);

enum ConfigErrorCode { CONFIG_ERR_READ_FAILED };
DEFINE_ERROR_DOMAIN(CONFIG, 6,
    ERROR(CONFIG_ERR_READ_FAILED, 601, "Failed to read config file")
);

Result(String) read_file_content(const char *path)
{
    (void)path;
    return Fail(String, IO_DOMAIN, IO_ERR_FILE_NOT_FOUND);
}

Result(String) load_config()
{
    const char *content;
    const char *config_path = "settings.json";

    TRY_FAIL(String, content, read_file_content(config_path), CONFIG_DOMAIN, CONFIG_ERR_READ_FAILED);

    return Ok(String, content);
}

Result(Void) start_application()
{
    const char *config;
    TRY_FAIL_CAST(Void, String, config, load_config(), APP_DOMAIN, APP_ERR_INIT_FAILED);

    printf("Application started successfully with config: %s\n", config);

    return Ok_void();
}

int main()
{
    Result(Void) final_res = start_application();

    if (is_error(final_res))
    {
        print_error_chain(unwrap_error(final_res), PRINT_ORDER_TOP_DOWN, stderr);
        printf("\n---\n\n");
        print_error_chain(unwrap_error(final_res), PRINT_ORDER_BOTTOM_UP, stderr);
    } 
    else {
        printf("Test failed: should have caught an error.\n");
        return 1;
    }

    return 0;
}
