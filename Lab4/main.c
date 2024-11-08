#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

void print_usage() {
    printf("Usage: ./mychmod [permissions] filename\n");
}

// Функция для обновления прав доступа 
int apply_symbolic_mode(char *mode, mode_t *permissions) {
    while (*mode) {
        mode_t who_mask = 0;
        // Определяем категории: u, g, o, a (all)
        while (*mode == 'u' || *mode == 'g' || *mode == 'o' || *mode == 'a') {
            if (*mode == 'u') who_mask |= S_IRWXU;
            else if (*mode == 'g') who_mask |= S_IRWXG;
            else if (*mode == 'o') who_mask |= S_IRWXO;
            else if (*mode == 'a') who_mask |= S_IRWXU | S_IRWXG | S_IRWXO;
            mode++;
        }

        if (!who_mask) who_mask = S_IRWXU | S_IRWXG | S_IRWXO; 

        char operation = *mode++;  // +, -, или =

        mode_t rights = 0;

        // Присваиваем права 
        while (*mode == 'r' || *mode == 'w' || *mode == 'x') {
            if (*mode == 'r') rights |= (who_mask & (S_IRUSR | S_IRGRP | S_IROTH));
            else if (*mode == 'w') rights |= (who_mask & (S_IWUSR | S_IWGRP | S_IWOTH));
            else if (*mode == 'x') rights |= (who_mask & (S_IXUSR | S_IXGRP | S_IXOTH));
            mode++;
        }

        if (operation == '+') {
            *permissions |= rights;
        } else if (operation == '-') {
            *permissions &= ~rights;
        } else if (operation == '=') {
            *permissions = (*permissions & ~who_mask) | rights;
        }
    }
    return 0;
}



int mychmod(const char *file, char *mode) {
    struct stat fileStat;
    
    if (stat(file, &fileStat) == -1) {
        perror("stat");
        return -1;
    }

    mode_t permissions = fileStat.st_mode;

    if (mode[0] >= '0' && mode[0] <= '7') {  // Числовой режим
        permissions = strtol(mode, NULL, 8);
    } 
    else {  // Символьный режим
        if (apply_symbolic_mode(mode, &permissions) == -1) {
            fprintf(stderr, "Invalid symbolic mode\n");
            return -1;
        }
    }

    if (chmod(file, permissions) == -1) {
        perror("mychmod");
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage();
        return 1;
    }

    const char *mode = argv[1];
    const char *file = argv[2];

    if (mychmod(file, (char *)mode) == -1) {
        fprintf(stderr, "Failed to change permissions\n");
        return 1;
    }

    return 0;
}
