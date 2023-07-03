#include <netdb.h>
#include <netinet/in.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#define MAX_MEM_BUFFER_SIZE 8192
#define PORT_DEFAULT 8765

bool is_exit_command(std::string &cmd) {
    return cmd == "exit" || cmd == "exit;" || cmd == "bye" || cmd == "bye;";
}
bool is_source_command(std::string &cmd) {
    return strcmp(cmd.c_str(), "source") >= 0;
}

int init_unix_sock(const char *unix_sock_path) {
    int sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "failed to create unix socket. %s", strerror(errno));
        return -1;
    }

    struct sockaddr_un sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sun_family = PF_UNIX;
    snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "%s",
             unix_sock_path);

    if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
        fprintf(stderr,
                "failed to connect to server. unix socket path '%s'. error %s",
                sockaddr.sun_path, strerror(errno));
        close(sockfd);
        return -1;
    }
    return sockfd;
}

int init_tcp_sock(const char *server_host, int server_port) {
    struct hostent *host;
    struct sockaddr_in serv_addr;

    if ((host = gethostbyname(server_host)) == NULL) {
        fprintf(stderr, "gethostbyname failed. errmsg=%d:%s\n", errno,
                strerror(errno));
        return -1;
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "create socket error. errmsg=%d:%s\n", errno,
                strerror(errno));
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);
    serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(serv_addr.sin_zero), 8);

    if (connect(sockfd, (struct sockaddr *)&serv_addr,
                sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "Failed to connect. errmsg=%d:%s\n", errno,
                strerror(errno));
        close(sockfd);
        return -1;
    }
    return sockfd;
}
int handle_source_command(std::string &command, int sockfd, char *recv_buf) {
    int ret = 0;
    std::stringstream ss(command);
    std::string word;
    std::vector<std::string> words;
    while (getline(ss, word, ' ')) {
        if(!word.empty()){
            words.push_back(word);
        }
    }
    
    if (words.size() != 2) {
        printf("usage: source <filename>\n");
        return 0;
    }
    printf("get source %s\n",words[1].c_str());
    return ret;
}
int handle_normal_command(std::string &command, int sockfd, char *recv_buf) {
    int send_bytes, ret = 0;
    if ((send_bytes = write(sockfd, command.c_str(), command.length() + 1)) ==
        -1) {
        // fprintf(stderr, "send error: %d:%s \n", errno, strerror(errno));
        std::cerr << "send error: " << errno << ":" << strerror(errno) << " \n"
                  << std::endl;
        exit(1);
    }

    int len = recv(sockfd, recv_buf, MAX_MEM_BUFFER_SIZE, 0);
    if (len < 0) {
        fprintf(stderr, "Connection was broken: %s\n", strerror(errno));
        ret = -1;
    } else if (len == 0) {
        printf("Connection has been closed\n");
        ret = -1;
    } else {
        for (int i = 0; i <= len; i++) {
            if (recv_buf[i] == '\0') {
                break;
            } else {
                printf("%c", recv_buf[i]);
            }
        }
        memset(recv_buf, 0, MAX_MEM_BUFFER_SIZE);
    }
    return ret;
}

int main(int argc, char *argv[]) {
    int ret = 0;  // set_terminal_noncanonical();
                  //    if (ret < 0) {
                  //        printf("Warning: failed to set terminal non
                  //        canonical. Long command may be "
                  //               "handled incorrect\n");
                  //    }

    const char *unix_socket_path = nullptr;
    const char *server_host = "127.0.0.1";  // 127.0.0.1 192.168.31.25
    int server_port = PORT_DEFAULT;
    int opt;

    while ((opt = getopt(argc, argv, "s:h:p:")) > 0) {
        switch (opt) {
            case 's':
                unix_socket_path = optarg;
                break;
            case 'p':
                char *ptr;
                server_port = (int)strtol(optarg, &ptr, 10);
                break;
            case 'h':
                server_host = optarg;
                break;
            default:
                break;
        }
    }

    // const char *prompt_str = "RucBase > ";

    int sockfd, send_bytes;
    // char send[MAXLINE];

    if (unix_socket_path != nullptr) {
        sockfd = init_unix_sock(unix_socket_path);
    } else {
        sockfd = init_tcp_sock(server_host, server_port);
    }
    if (sockfd < 0) {
        return 1;
    }

    char recv_buf[MAX_MEM_BUFFER_SIZE];

    while (1) {
        char *line_read = readline("Rucbase> ");
        if (line_read == nullptr) {
            // EOF encountered
            break;
        }
        std::string command = line_read;
        free(line_read);

        if (!command.empty()) {
            add_history(command.c_str());
            if (is_exit_command(command)) {
                printf("The client will be closed.\n");
                break;
            }
            if (is_source_command(command)) {
                if (handle_source_command(command, sockfd, recv_buf)) {
                    break;
                }

            } else {
                if (handle_normal_command(command, sockfd, recv_buf) < 0) {
                    break;
                }
            }

            // if ((send_bytes = write(sockfd, command.c_str(),
            //                         command.length() + 1)) == -1) {
            //     // fprintf(stderr, "send error: %d:%s \n", errno,
            //     // strerror(errno));
            //     std::cerr << "send error: " << errno << ":"
            //               << strerror(errno) << " \n"
            //               << std::endl;
            //     exit(1);
            // }
            // int len = recv(sockfd, recv_buf, MAX_MEM_BUFFER_SIZE, 0);
            // if (len < 0) {
            //     fprintf(stderr, "Connection was broken: %s\n",
            //             strerror(errno));
            //     break;
            // } else if (len == 0) {
            //     printf("Connection has been closed\n");
            //     break;
            // } else {
            //     for (int i = 0; i <= len; i++) {
            //         if (recv_buf[i] == '\0') {
            //             break;
            //         } else {
            //             printf("%c", recv_buf[i]);
            //         }
            //     }
            //     memset(recv_buf, 0, MAX_MEM_BUFFER_SIZE);
            // }
        }
    }
    close(sockfd);
    printf("Bye.\n");
    return 0;
}
