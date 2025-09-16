#include "../result.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// 1. Define the Result type for this file's functions
RESULT_TYPE(File, int);

// 2. Define error codes for the 'File' domain
typedef enum {
    FILE_ERR_NOT_FOUND,
    FILE_ERR_PERMISSION_DENIED,
    FILE_ERR_IO,
} FileErrorCode;

// 3. Define the ErrorDomain object for 'File' errors
DEFINE_ERROR_DOMAIN(FILE, 1,
    ERROR(FILE_ERR_NOT_FOUND, ENOENT, "No such file or directory"),
    ERROR(FILE_ERR_PERMISSION_DENIED, EACCES, "Permission denied"),
    ERROR(FILE_ERR_IO, EIO, "I/O Error")
);


Result(File) open_file(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        return Fail_from_errno(File, FILE_DOMAIN, errno, FILE_ERR_IO);

    return Ok(File, fd);
}

void readfile(int fd)
{
    char buffer[256];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    if (bytes_read == -1)
        perror("Error reading file");
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Result(File) file_res = open_file(argv[1]);
    if (is_error(file_res)) {
        fprintf(stderr, "%s Error: %s\n", error_domain(file_res), error_msg(file_res));
        return 1;
    }

    int fd = unwrap_ok(file_res);
    readfile(fd);
    close(fd);

    return 0;
}