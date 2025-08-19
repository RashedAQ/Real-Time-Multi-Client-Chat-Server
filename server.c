#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>

#define DATA_SIZE 255
#define MESSAGE_SIZE 512
#define MAX_CLIENTS 10
#define MAXUSER 10

struct User {
    socklen_t clilen;
    struct sockaddr_in cli_addr;
    int newsockfd;
    char username[16];
    char session_id[32];  // Increased the size for the session_id
};

struct User users[MAXUSER];
int user_anz = 0;
struct sockaddr_in serv_addr;

FILE *logfile;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

unsigned long crc32(const unsigned char *data, unsigned int len) {
    unsigned long crc = 0;
    for (unsigned int i = 0; i < len; ++i) {
        crc += data[i];
    }
    return crc % 0xFFFFFFFF;
}

int countOnes(unsigned char byte) {
    int count = 0;
    while (byte) {
        count += byte & 1;
        byte >>= 1;
    }
    return count;
}

void create_log_directory() {
    struct stat st = {0};
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0700);
    }
}

void open_log_file(const char *session_id) {
    char filename[100];
    snprintf(filename, sizeof(filename), "logs/session_%s.log", session_id);

    logfile = fopen(filename, "a");
    if (!logfile) {
        perror("Error opening log file");
        exit(1);
    }
}

void write_log(const char *username, const char *message) {
    if (logfile) {
        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        fprintf(logfile, "[%02d:%02d:%02d] %s: %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, username, message);
        fflush(logfile);
    }
}

void close_log_file_by_session_id(const char *session_id) {
    char filename[100];
    snprintf(filename, sizeof(filename), "logs/session_%s.log", session_id);

    FILE *session_logfile = fopen(filename, "a");
    if (session_logfile) {
        fclose(session_logfile);
    }
}

void generate_session_id(char *session_id, size_t session_id_size) {
    snprintf(session_id, session_id_size, "%ld", time(NULL));
}

void *client_socket_reader(void *usernr) {
    int mynr = *(int *)usernr, n, i = 0;
    char buffer[DATA_SIZE], message_to_send[MESSAGE_SIZE], splitter[DATA_SIZE];

    while (1) {
        bzero(buffer, sizeof(buffer));
        n = read(users[mynr].newsockfd, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            // Log the disconnection event
            printf("Client %s disconnected\n", users[mynr].username);
            write_log(users[mynr].username, "disconnected");

            // Close the log file associated with the current session
            close_log_file_by_session_id(users[mynr].session_id);

            user_anz--;
            close(users[mynr].newsockfd);
            free(usernr);
            return NULL;
        }

        int onesCount = 0;
        for (int i = 0; i < n; ++i) {
            onesCount += countOnes(buffer[i]);
        }

        if (onesCount % 2 == 0) {
            printf("Data received with even parity\n");
        } else {
            printf("Data received with odd parity. Possible error!\n");
        }

        unsigned long crc = crc32((unsigned char *)buffer, n);
        printf("Received Cyclic Redundancy Check: %lu\n", crc);

        // Log messages
        write_log(users[mynr].username, buffer);

        sscanf(buffer, "%s", splitter);

        if (strcmp(splitter, "list") == 0) {
            char userList[MESSAGE_SIZE] = "Connected Clients\n";
            for (i = 0; i < user_anz; i++) {
                strcat(userList, users[i].username);
                strcat(userList, "\n");
            }
            write(users[mynr].newsockfd, userList, strlen(userList));
        } else if (strcmp(splitter, "GONE") == 0) {
            write(users[mynr].newsockfd, "You are disconnected\n", 22);
            user_anz--;
            close(users[mynr].newsockfd);
            free(usernr);
            return NULL;
        } else if (strncmp(buffer, "MESG", 4) == 0) {
            char recipient[16];
            sscanf(buffer, "MESG %15s", recipient);
            char *message_start = strchr(buffer, ' ');
            if (message_start) message_start++;
            else message_start = "";

            bool recipient_found = false;
            for (i = 0; i < user_anz; i++) {
                if (strcmp(users[i].username, recipient) == 0) {
                    snprintf(message_to_send, sizeof(message_to_send), "[Private]%s: %s", users[mynr].username, message_start);
                    n = write(users[i].newsockfd, message_to_send, strlen(message_to_send));
                    if (n < 0) {
                        perror("ERROR writing to socket");
                        recipient_found = true;  // Still set recipient_found to true to avoid logging "Recipient not found"
                    } else {
                        recipient_found = true;
                    }
                    break;
                }
            }

            if (!recipient_found) {
                char errorMsg[MESSAGE_SIZE];
                snprintf(errorMsg, sizeof(errorMsg), "Recipient not found: %s\n", recipient);
                write(users[mynr].newsockfd, errorMsg, strlen(errorMsg));

                // Log the "Recipient not found" message
                printf("Recipient not found: %s\n", recipient);
                write_log(users[mynr].username, errorMsg);
            }
        } else if (strncmp(splitter, "/CONN", 5) == 0) {
            char newUsername[16];
            sscanf(buffer, "/CONN %15s", newUsername);

            // Find the client index
            int clientIndex = -1;
            for (int i = 0; i < user_anz; i++) {
                if (users[i].newsockfd == users[mynr].newsockfd) {
                    clientIndex = i;
                    break;
                }
            }

            if (clientIndex != -1) {
                // Log the disconnection event
                printf("Client %s disconnected\n", users[clientIndex].username);
                write_log(users[clientIndex].username, "disconnected");

                // Close the socket for the existing client
                close(users[clientIndex].newsockfd);

                // Update the username
                strncpy(users[clientIndex].username, newUsername, sizeof(users[clientIndex].username));

                // Log the connection event
                printf("Client %s connected with a new username\n", users[clientIndex].username);
                write_log(users[clientIndex].username, "connected with a new username");

                // Send a message to the client about the username change
                write(users[clientIndex].newsockfd, "You have been connected with a new username\n", 43);
            }
        } else {
            snprintf(message_to_send, sizeof(message_to_send), "%s: %s", users[mynr].username, buffer);
            for (i = 0; i < user_anz; i++) {
                if (i != mynr) {
                    n = write(users[i].newsockfd, message_to_send, strlen(message_to_send));
                    if (n < 0) perror("ERROR writing to socket");
                }
            }
        }
    }
}

void print_server_message(const char *message) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    printf("[%02d:%02d:%02d] %s\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, message);
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    pthread_t pt;

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    create_log_directory();

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    listen(sockfd, MAX_CLIENTS);
    clilen = sizeof(struct sockaddr_in);

    printf("Server started on port %d\n", portno);

    while (1) {
        struct User newUser;
        newsockfd = accept(sockfd, (struct sockaddr *)&newUser.cli_addr, &clilen);
        if (newsockfd < 0) error("ERROR on accept");

        // Receive and set the username for the new client
        bzero(newUser.username, sizeof(newUser.username));
        read(newsockfd, newUser.username, sizeof(newUser.username) - 1);

        // Generate a unique session ID for the user
        generate_session_id(newUser.session_id, sizeof(newUser.session_id));

        // Log the connection event
        printf("Client %s connected\n", newUser.username);
        write_log(newUser.username, "connected");

        // Open a log file for the new session
        open_log_file(newUser.session_id);

        newUser.newsockfd = newsockfd;
        newUser.clilen = clilen;

        users[user_anz] = newUser;

        int *usernr = malloc(sizeof(int));
        *usernr = user_anz;

        // Create a new thread for the client
        if (pthread_create(&pt, NULL, client_socket_reader, usernr) != 0) {
            print_server_message("Error creating thread for a new user");
            continue;
        }

        // Increase the user count
        user_anz++;

        // Send a welcome message to the client
        char welcomeMessage[MESSAGE_SIZE];
        snprintf(welcomeMessage, sizeof(welcomeMessage), "Welcome to the server, %s!\n", newUser.username);
        write(newUser.newsockfd, welcomeMessage, strlen(welcomeMessage));

        // Notify other clients about the new connection
        char notificationMessage[MESSAGE_SIZE];
        snprintf(notificationMessage, sizeof(notificationMessage), "New client connected: %s\n", newUser.username);
        for (int i = 0; i < user_anz; i++) {
            if (i != user_anz - 1) {  // Skip the newly connected client
                write(users[i].newsockfd, notificationMessage, strlen(notificationMessage));
            }
        }
    }

    close(sockfd);
    return 0;
}