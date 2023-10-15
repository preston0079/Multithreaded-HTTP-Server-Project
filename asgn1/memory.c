#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096

// Function to handle the "get" operation
int get(int fd) {
    int bytes_read = 0;
    char *buf = (char *)malloc(BUFFER_SIZE * sizeof(char));

    if (buf == NULL) {
        fprintf(stderr, "Operation Failed\n");
        exit(1);
    }

    // Read from the fd and write to standard output
    do {
        bytes_read = read(fd, buf, BUFFER_SIZE);

        // Check for read errors
        if (bytes_read < 0) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        } else if (bytes_read > 0) {
            int bytes_write = 0;
            
            // Write data to standard output
            do {
                int bytes = write(STDOUT_FILENO, buf + bytes_write, bytes_read - bytes_write);
                
                // Check for write errors
                if (bytes <= 0) {
                    fprintf(stderr, "Operation Failed\n");
                    exit(1);
                }
                bytes_write += bytes;
            } while (bytes_write < bytes_read);
        }
    } while (bytes_read > 0);

    // Free allocated memory
    free(buf);
    return 0;
}

// Function to handle the "set" operation
int set(int fd, size_t content_length) {
    int bytes_read = 0;
    char *buf = (char *)malloc(BUFFER_SIZE * sizeof(char));
    
    if (buf == NULL) {
        fprintf(stderr, "Operation Failed\n");
        exit(1);
    }

    ssize_t total_bytes_read = 0;

    // Read from standard input and write to the specified file descriptor
    do {
        bytes_read = read(STDIN_FILENO, buf, BUFFER_SIZE);

        // Check for read errors
        if (bytes_read < 0) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        } else if (bytes_read > 0) {
            ssize_t bytes_written = 0;
            
            // Write data to the file descriptor
            do {
                ssize_t bytes = write(fd, buf + bytes_written, bytes_read - bytes_written);

                // Check for write errors
                if (bytes <= 0) {
                    fprintf(stderr, "Operation Failed\n");
                    exit(1);
                }
                bytes_written += bytes;
            } while (bytes_written < bytes_read);

            // Update the total_bytes_read
            total_bytes_read += bytes_written;
            
            // Check if content length is specified and if it has reached the specified length
            if ((size_t)content_length > 0 && total_bytes_read >= (ssize_t)content_length) {
                break;
            }
        }
    } while (bytes_read > 0);

    // Free allocated memory
    free(buf);
    return 0;
}

// Help recieved from classmate - work shown on whiteboard
int main(void) {
    int fd = 1; // stdout
    int new_line = 0;
    int buffer_index = 0;
    int buffer_index2 = 0;

    char first_command[BUFFER_SIZE] = {0};
    char location[BUFFER_SIZE] = {0};
    char content_length_str[BUFFER_SIZE] = {0};
    char temp;
    char first_byte;
    char buffer;

    ssize_t bytes_read = 0;
    size_t total_bytes_read = 0;

    // Check if the file exceeds PATH_MAX
    if (strlen(location) > PATH_MAX) {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    // Read the first two lines
    while (new_line < 2) {
        bytes_read = read(STDIN_FILENO, &temp, 1);
        if (bytes_read <= 0) {
            fprintf(stderr, "Invalid Command\n");
            exit(1);
        }

        if (temp == '\n') {
            new_line++;
            buffer_index = 0;

            // Break out after the second new line
            if (new_line == 2) {
                break;
            }
            continue;
        }

        // Get command / location
        if (new_line == 0) {
            first_command[buffer_index++] = temp;
        } else if (new_line == 1) {
            location[buffer_index++] = temp;
        }
    }

    // Check the command and perform either get or set function
    if (strcmp(first_command, "get") == 0) {

        // Make sure the input is formatted correctly (only a new line)
        while ((bytes_read = read(STDIN_FILENO, &buffer, 1)) > 0) {
            if (buffer != '\n') {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }
        }

        // Open the file for reading and call the get function
        fd = open(location, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        }
        get(fd);
        close(fd);
    } else if (strcmp(first_command, "set") == 0) {
        
        // Open the file for writing and call the set function
        fd = open(location, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd < 0) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        }

        // Read first byte
        if (read(STDIN_FILENO, &first_byte, 1) <= 0) {
            fprintf(stderr, "Operation Failed\n");
            exit(1);
        }

        // Check if the content length is specified (starts with a digit)
        if (isdigit((unsigned char) first_byte)) {

            // Store the first_byte in the content_length_str and increment the buffer
            content_length_str[buffer_index2++] = first_byte;

            // Read until hit new line
            while (
                (total_bytes_read = read(STDIN_FILENO, &first_byte, 1)) > 0 && first_byte != '\n') {
                content_length_str[buffer_index2++] = first_byte;
            }

            // Make into C-string with null terminator
            content_length_str[buffer_index2] = '\0';

            // For strtoul
            char *end_ptr;

            // Convert content_length_str into size_t
            size_t content_length = strtoul(content_length_str, &end_ptr, 10);

            // Check for invalid content length - end should be on null terminator
            if (*end_ptr != '\0') {
                fprintf(stderr, "Invalid Command\n");
                exit(1);
            }
            set(fd, content_length);
            fprintf(stdout, "OK\n");
            close(fd);

        // Else: first_byte is not a digit so no content length is specified
        } else {

            // Write the first byte to the file
            write(fd, &first_byte, 1);
            set(fd, 0); // Pass 0 to indicate no specified content length
            fprintf(stdout, "OK\n");
            close(fd);
        }

    // Not get or set
    } else {
        fprintf(stderr, "Invalid Command\n");
        exit(1);
    }

    return 0;
}


