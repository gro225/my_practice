#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

void usage(const char *prog_name) {
    printf("Usage: %s [mode] filename\n", prog_name);
}

mode_t octal_mode(const char *mode_str) {
    return strtol(mode_str, NULL, 8);
}

mode_t makeNewMode(char *fileName, char *ugoa, char action, char *change) {
    struct stat st;

    if (lstat(fileName, &st) != 0) {
        printf("chmod: impossible acces to  '%s': %s\n", fileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    bool userMode = false, groupMode = false, otherMode = false;

    
    if (strlen(ugoa) == 0) {
        userMode = groupMode = otherMode = true;  // Все пользователи
    } else {
        for (size_t i = 0; i < strlen(ugoa); i++) {
            if (ugoa[i] == 'u') { userMode = true; }
            else if (ugoa[i] == 'g') { groupMode = true; }
            else if (ugoa[i] == 'o') { otherMode = true; }
            else if (ugoa[i] == 'a') { userMode = groupMode = otherMode = true; }
            else {
                fprintf(stderr, "Unsuppoted value!\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    mode_t newMode = st.st_mode;
    for (size_t i = 0; i < strlen(change); i++) {
        mode_t bitMask = 0;

        if (change[i] == 'r') {
            if (userMode) { bitMask |= S_IRUSR; }
            if (groupMode) { bitMask |= S_IRGRP; }
            if (otherMode) { bitMask |= S_IROTH; }
        } else if (change[i] == 'w') {
            if (userMode) { bitMask |= S_IWUSR; }
            if (groupMode) { bitMask |= S_IWGRP; }
            if (otherMode) { bitMask |= S_IWOTH; }
        } else if (change[i] == 'x') {
            if (userMode) { bitMask |= S_IXUSR; }
            if (groupMode) { bitMask |= S_IXGRP; }
            if (otherMode) { bitMask |= S_IXOTH; }
        } else {
            fprintf(stderr, "Unsuppoted value!\n");
            exit(EXIT_FAILURE);
        }

        switch (action) {
            case '+':
                newMode |= bitMask;
                break;
            case '-':
                newMode &= ~bitMask;
                break;
            case '=':
                if (userMode) newMode &= ~(S_IRUSR | S_IWUSR | S_IXUSR);
                if (groupMode) newMode &= ~(S_IRGRP | S_IWGRP | S_IXGRP);
                if (otherMode) newMode &= ~(S_IROTH | S_IWOTH | S_IXOTH);
                newMode |= bitMask;
                break;
            default:
                fprintf(stderr, "Unsuppoted value!\n");
                exit(EXIT_FAILURE);
        }
    }

    return newMode;
}

mode_t symbolic_mode(const char *mode_str, const char *filename) {
    char ugoa[4] = {0};
    char action;
    const char *change;

    if (mode_str[1] == '+' || mode_str[1] == '-' || mode_str[1] == '=') {
        strncpy(ugoa, mode_str, 1);
        action = mode_str[1];
        change = mode_str + 2;
    } else {
        action = mode_str[0];
        change = mode_str + 1;
    }

    return makeNewMode((char*)filename, ugoa, action, (char*)change);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    const char *mode_str = argv[1];
    const char *filename = argv[2];
    struct stat file_stat;

    if (stat(filename, &file_stat) == -1) {
        perror("Error getting file status");
        return 1;
    }

    mode_t new_mode;
    if (isdigit(mode_str[0])) {
        new_mode = octal_mode(mode_str);
    } else {
        new_mode = symbolic_mode(mode_str, filename);
        if (new_mode == (mode_t)-1) return 1;
    }

    FILE *src_file = fopen(filename, "rb");
    if (src_file == NULL) {
        perror("Error opening source file");
        return 1;
    }

    fseek(src_file, 0, SEEK_END);
    size_t file_size = (size_t) ftell(src_file);
    rewind(src_file);

    char *buffer = malloc(file_size);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(src_file);
        return 1;
    }

    if (fread(buffer, 1, file_size, src_file) != file_size) {
        perror("Error reading file into buffer");
        free(buffer);
        fclose(src_file);
        return 1;
    }
    fclose(src_file);

    char tmp_filename[] = "tmp_fileXXXXXX";
    int tmp_fd = mkstemp(tmp_filename);
    if (tmp_fd == -1) {
        perror("Error creating temporary file");
        free(buffer);
        return 1;
    }

    fchmod(tmp_fd, new_mode);
    FILE *tmp_file = fdopen(tmp_fd, "wb");

    if (fwrite(buffer, 1, file_size, tmp_file) != file_size) {
        perror("Error writing buffer to temporary file");
        fclose(tmp_file);
        free(buffer);
        unlink(tmp_filename);
        return 1;
    }

    free(buffer);
    fclose(tmp_file);

    if (rename(tmp_filename, filename) == -1) {
        perror("Error renaming temporary file");
        unlink(tmp_filename);
        return 1;
    }

    return 0;
}
