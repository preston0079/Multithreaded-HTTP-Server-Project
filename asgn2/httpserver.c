#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "asgn2_helper_funcs.h"

#define BUFFER_SIZE 4096

//HELP REVIEVED FROM CLASSMATE IN LIBRARY - WORK DONE OVER WHITEBOARD, NO CODE WAS SHARED

// Function to read an HTTP request from the client
ssize_t read_command(int socket_fd, char *buffer, size_t max_length) {
    ssize_t total_bytes_read = 0;
    ssize_t bytes_read;

    while (total_bytes_read < max_length) {
        bytes_read = read(socket_fd, buffer + total_bytes_read, max_length - total_bytes_read);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                // Connection closed by the client
                return total_bytes_read;
            } else {
                // Error while reading
                return -1;
            }
        }

        total_bytes_read += bytes_read;

        // Check for the end of the HTTP request (double CRLF)
        if (total_bytes_read >= 4 && buffer[total_bytes_read - 2] == '\r'
            && buffer[total_bytes_read - 1] == '\n' && buffer[total_bytes_read - 4] == '\r'
            && buffer[total_bytes_read - 3] == '\n') {
            // End of the request found
            break;
        }
    }

    return total_bytes_read;
}

// Function to send an HTTP response to the client
void send_response(int client_socket, int status, int content_length) {
    char response[1024];

    // Compose an HTTP response with status, content length, and a message
    char *status_message = "temp";

    if (status == 200) {
        status_message = "OK";
    } else if (status == 201) {
        status_message = "Created";
    } else if (status == 400) {
        status_message = "Bad Request";
    } else if (status == 403) {
        status_message = "Forbidden";
    } else if (status == 404) {
        status_message = "Not Found";
    } else if (status == 500) {
        status_message = "Internal Server Error";
    } else if (status == 501) {
        status_message = "Not Implemented";
    } else if (status == 505) {
        status_message = "Version Not Supported";
    }

    // Put it all together
    sprintf(response, "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s", status, status_message,
        content_length, status_message);

    // Send the response to the client
    write_n_bytes(client_socket, response, strlen(response));
    write_n_bytes(client_socket, "\n", 2);
}

// Function to parse and validate an HTTP request
int test_request(
    const char *request, regex_t *regex, char *method, char *uri, char *version, char *header) {
    regmatch_t matches[5]; // Assuming 4 capturing groups in the regex

    // Use the regex to match and get components of the request
    if (regexec(regex, request, 5, matches, 0) != 0) {
        return 1; // Invalid request
    }

    // Handle the HTTP method (GET, PUT)
    if (matches[1].rm_so != -1) {
        strncpy(method, request + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
        method[matches[1].rm_eo - matches[1].rm_so] = '\0';
    } else {
        return 2; // Method not implemented
    }

    // Handle the URI
    if (matches[2].rm_so != -1) {
        strncpy(uri, request + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
        uri[matches[2].rm_eo - matches[2].rm_so] = '\0';
    } else {
        return 3; // Bad request (missing URI)
    }

    // Handle the HTTP version
    if (matches[3].rm_so != -1) {
        strncpy(version, request + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);
        version[matches[3].rm_eo - matches[3].rm_so] = '\0';
        if (strcmp(version, "HTTP/1.1") != 0) {
            return 4; // HTTP version not supported
        }
    } else {
        return 5; // Bad request (missing version)
    }

    // Handle optional headers
    if (matches[4].rm_so != -1) {
        strncpy(header, request + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);
        header[matches[4].rm_eo - matches[4].rm_so] = '\0';
    }

    return 0; // Request is valid
}

int main(int argc, char **argv) {
    int port;
    int acceptfd;
    int fd;
    char request_buffer[BUFFER_SIZE] = { 0 };
    size_t bytes_read = 0;
    Listener_Socket sock;

    regex_t regex;
    char method[10];
    char uri[65];
    char version[10];
    char header[260];

    const char *pattern = "^([a-zA-Z]{1,8}) (/[^ ]+) (HTTP/[0-9].[0-9])\r\n(([a-zA-Z0-9.-]{1,128}: "
                          "[ -~]{0,128}\r\n)*)\r\n";

    // Compile the regular expression for parsing requests
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex\n");
        exit(1);
    }

    // Get the port number from command line arguments
    if (argc < 2) {
        fprintf(stderr, "Invalid Port Number\n");
        exit(1);
    }

    // Port number from command line
    port = atoi(argv[1]);

    // Check for a valid port number
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid Port Number\n");
        exit(1);
    }

    // Initialize the socket with the given port number
    if (listener_init(&sock, port) == -1) {
        perror("Listener_init Failure");
        exit(1);
    }

    while (1) {
        acceptfd = listener_accept(&sock);
        if (acceptfd == -1) {
            perror("Listener_accept Failure");
            exit(1);
        }

        // Read the request from the socket on the client side
        bytes_read = read_command(acceptfd, request_buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            send_response(acceptfd, 500, 22); // Internal Server Error
            perror("bytes_read < 0");
            exit(1);
        }

        // Process the request line
        int check = test_request(request_buffer, &regex, method, uri, version, header);

        // Check request: GET or PUT
        if (check == 0) {
            if (strcmp(method, "GET") == 0) {
                // Handle GET request
                struct stat st;
                stat(uri + 1, &st);
                if (S_ISDIR(st.st_mode) == 0) {
                    size_t size = st.st_size;
                    fd = open(uri + 1, O_RDONLY);
                    if (fd == -1) {
                        send_response(acceptfd, 404, 10); // Not Found
                    } else {
                        send_response(acceptfd, 200, size); // OK
                        int i = pass_n_bytes(fd, acceptfd, size);
                    }
                    close(fd);
                } else {
                    send_response(acceptfd, 403, 10); // Forbidden
                }
            } else if (strcmp(method, "PUT") == 0) {
                // Handle PUT request

            } else {
                send_response(acceptfd, 501, 16); // Not Implemented
            }
        } else if (check == 1) {
            send_response(acceptfd, 400, 12); // Bad Request
        } else if (check == 2) {
            send_response(acceptfd, 501, 16); // Not Implemented
        } else if (check == 3) {
            send_response(acceptfd, 400, 12); // Bad Request
        } else if (check == 4) {
            send_response(acceptfd, 505, 22); // HTTP Version Not Supported
        } else if (check == 5) {
            send_response(acceptfd, 505, 22); // HTTP Version Not Supported
        }

        // Reset the buffers and close the client connection
        memset(method, 0, 10);
        memset(uri, 0, 65);
        memset(version, 0, 10);
        memset(header, 0, 260);
        memset(request_buffer, 0, BUFFER_SIZE);
        close(acceptfd);
    }

    return 0;
}
