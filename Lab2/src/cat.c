#include <stdio.h>
#include <string.h>
#include <unistd.h>

void show_file(const char *file, short num_all, short num_not_empty, short add_dollar) {
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        printf("cat_utilite: %s: No such file or directory\n", file);
        return;
    }

    char line[256];
    int line_number = 1; 


    while (fgets(line, sizeof(line), fp) != NULL) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        if (num_all && num_not_empty == 0) {
            printf("%6d  ", line_number++);
        }
        else if (num_not_empty && strlen(line) > 0) {
            printf("%6d  ", line_number++);
        } 
        else if (num_not_empty && strlen(line) == 0) {
            printf("        ");
        }

        printf("%s", line);

        if (add_dollar) {
            printf("$");
        }

        printf("\n");
    }

    fclose(fp); 
}

int main(int argc, char **argv) {
    char arg;
    short num_all = 0, num_not_empty = 0, add_dollar = 0;

    while ((arg = getopt(argc, argv, "nbE")) != -1) {
        switch (arg) {
            case 'n':
                num_all = 1;
                break;
            case 'b':
                num_not_empty = 1;
                break;
            case 'E':
                add_dollar = 1;
                break;
            default:
                printf("Unknown option: %c\n", arg);
                return 1;
        }
    }

    if (optind >= argc) {
        printf("Usage: %s [-n] [-b] [-E] <filename>\n", argv[0]);
        return 1;
    }
    show_file(argv[optind], num_all, num_not_empty, add_dollar);

    return 0;
}
