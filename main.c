#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

FILE* pointer_name;

void exit_handler(){
    fclose(pointer_name);
    exit(0);
}

char* combine_strings(const char* str, char* str2){
    char *combinedString;
    int totalSize = strlen(str2) + 1;
    combinedString = (char *)malloc(totalSize * sizeof(char));
    sprintf(combinedString, str, str2);
    return combinedString;
}

int main(int argc, char **argv) {
    char* path, log_path = "";

    if (argc > 1) {
        path = argv[1];
    } else if (argc > 2){
        log_path = argv[1];
    }

    if (log_path == ""){
        pointer_name = fopen("log.txt", "a+");
    } else {
        pointer_name = fopen(log_path, "a+");
    }

    if (pointer_name == NULL) {
        perror("Log file isn't opened.");
    }

    signal(SIGINT, exit_handler);

    int length, i = 0;
    int fd, wd;
    char buffer[BUF_LEN];

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
    }

    if (path == ""){
        wd = inotify_add_watch(fd, ".", IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM);
    } else {
        wd = inotify_add_watch(fd, path, IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_TO | IN_MOVED_FROM);
    }

    for (;;) {
        i = 0;
        int total_read = read(fd, buffer, BUF_LEN);

        if (total_read < 0) {
            perror("read");
        }

        while (i < total_read) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];

            if (event->len) {
                if (event->mask & IN_CREATE) {
                    if ( event->mask & IN_ISDIR ) {
                        char *combinedString = combine_strings("The dir %s was created.\n", event->name);
                        fputs(combinedString, pointer_name);
                    } else {
                        char *combinedString = combine_strings("The file %s was created.\n", event->name);
                        fputs(combinedString, pointer_name);
                    }
                }
                else if (event->mask & IN_MODIFY) {
                    if ( event->mask & IN_ISDIR ) {
                        char *combinedString = combine_strings("The dir %s was modified.\n", event->name);
                        fputs(combinedString, pointer_name);
                    } else {
                        char *combinedString = combine_strings("The file %s was modified.\n", event->name);
                        fputs(combinedString, pointer_name);
                    }
                }
                else if (event->mask & IN_DELETE) {
                    if ( event->mask & IN_ISDIR ) {
                        char *combinedString = combine_strings("The dir %s was deleted.\n", event->name);
                        fputs(combinedString, pointer_name);
                    } else {
                        char *combinedString = combine_strings("The file %s was deleted.\n", event->name);
                        fputs(combinedString, pointer_name);
                    }
                }
                else if (event->mask & IN_MOVED_TO){
                    if ( event->mask & IN_ISDIR ) {
                        char *combinedString = combine_strings("The dir %s was moved.\n", event->name);
                        fputs(combinedString, pointer_name);
                    } else {
                        char *combinedString = combine_strings("The file %s was moved.\n", event->name);
                        fputs(combinedString, pointer_name);
                    }
                }
                else if (event->mask & IN_MOVED_FROM){
                    if ( event->mask & IN_ISDIR ) {
                        char *combinedString = combine_strings("The dir %s was moved.\n", event->name);
                        fputs(combinedString, pointer_name);
                    } else {
                        char *combinedString = combine_strings("The file %s was moved.\n", event->name);
                        fputs(combinedString, pointer_name);
                    }
                }
            }

            i += EVENT_SIZE + event->len;
        }
    }

    inotify_rm_watch(fd, wd);

    close(fd);

    return 0;
}