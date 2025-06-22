#include "../maybe.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define FILE_ERRORS(ErrDef, Name) \
    ErrDef(Name, NOT_FOUND, "File not found") \
    ErrDef(Name, PERMISSION_DENIED, "Permission denied to access the file")

RESULT_TYPE(File, int, FILE_ERRORS);

Result(File) open_file(char *filename)
{
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT)
            return Error(File, NOT_FOUND);
        else if (errno == EACCES)
            return Error(File, PERMISSION_DENIED);
        else
            return Error(File, UNKNOWN_ERROR);
    }
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

    Result(File) file = open_file(argv[1]);
    if (is_error(file)) {
        fprintf(stderr, "Error opening file '%s': %s\n", argv[1], error_msg(File, file));
        return 1;
    }

    int fd = unwrap_ok(file);
    readfile(fd);
    close(fd);

    return 0;
}
