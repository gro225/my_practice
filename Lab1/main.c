#include <stdio.h>
#include <unistd.h>

#include <sys/stat.h> 
// #include <sys/types.h>
#include <dirent.h>

#include <errno.h>
#include <string.h>
#include <time.h>

#include <pwd.h>
#include <grp.h>

//Цвета
#define COLOR_RESET "\033[0m"
#define COLOR_DIR "\033[1;34m"
#define COLOR_EXE "\033[1;32m"
#define COLOR_LNK "\033[1;36m"

void print_file_permissions(mode_t mode) {
    char permissions[11];

    // Первый символ указывает тип файла
    if (S_ISDIR(mode)) {
        permissions[0] = 'd';
    } else if (S_ISREG(mode)) {
        permissions[0] = '-';
    } else if (S_ISLNK(mode)) {
        permissions[0] = 'l';
    }

    // Права пользователя
    permissions[1] = (mode & S_IRUSR) ? 'r' : '-';
    permissions[2] = (mode & S_IWUSR) ? 'w' : '-';
    permissions[3] = (mode & S_IXUSR) ? 'x' : '-';

    // Права группы
    permissions[4] = (mode & S_IRGRP) ? 'r' : '-';
    permissions[5] = (mode & S_IWGRP) ? 'w' : '-';
    permissions[6] = (mode & S_IXGRP) ? 'x' : '-';

    // Права для всех остальных
    permissions[7] = (mode & S_IROTH) ? 'r' : '-';
    permissions[8] = (mode & S_IWOTH) ? 'w' : '-';
    permissions[9] = (mode & S_IXOTH) ? 'x' : '-';

    permissions[10] = '\0';

    printf("%s ", permissions);
}

void print_last_modified(time_t time){
    char* date = ctime(&time);
    date[strlen(date) - 1] = '\0';
    //Проиводим к виду ls 
    char* space_pos = strstr(date, " ");
    int new_length = strlen(space_pos) - 8;
    char new_date[14];
    strncpy(new_date, space_pos, new_length);
    printf("%s ", new_date);
}

void print_colored_name(const char* name, mode_t mode) {
    if (S_ISDIR(mode)) {
        printf(COLOR_DIR "%s" COLOR_RESET, name);
    }
    else if (S_ISREG(mode) && (mode & S_IXUSR)) {
        printf(COLOR_EXE "%s" COLOR_RESET, name);
    }
    else if (S_ISLNK(mode)) {
        printf(COLOR_LNK "%s" COLOR_RESET, name);
    }
    else {
        printf("%s", name);
    }
}


int main(int argc, char** argv) {

    char arg;
    int show_all = 0, long_format = 0;
    long long total_blocks = 0;

    while ((arg = getopt(argc, argv, "al")) != -1) {
        switch (arg) {
            case 'a':
            show_all = 1;
                break;
            case 'l': 
            long_format = 1;
                break;

            default:
                printf("Unknown option: %c\n", arg);
                break;
        }
    }

	DIR* dp = opendir("./");
	struct dirent* ep;

    // Первый проход по дирректориям для подсчета их кол-во
    while ((ep = readdir(dp)) != NULL) {
        if (!show_all && ep->d_name[0] == '.') {
            continue;
        }

        struct stat file_info;
        if (stat(ep->d_name, &file_info) == 0){
            total_blocks += file_info.st_blocks; 
        }
    }
     if (long_format) {
         printf("total %lld\n", total_blocks / 2);
     }

    // Сбрасываем позицию 
    rewinddir(dp);

    // Второй проход по дирректориями для вывода информации
    while ((ep = readdir(dp)) != NULL){
        if (!show_all && ep->d_name[0] == '.') {
            continue;
        }

        if (long_format) {
            struct stat file_info;
            if (stat(ep->d_name, &file_info) == 0) {
                struct passwd *pw = getpwuid(file_info.st_uid);
                struct group  *gr = getgrgid(file_info.st_gid);

                print_file_permissions(file_info.st_mode);
                printf("%ld ", (long)file_info.st_nlink); 
                printf("%s %s ", pw->pw_name, gr->gr_name);  
                printf("%ld ", file_info.st_size);
                print_last_modified(file_info.st_mtime); 
            }
        }

        struct stat file_info;
        if (stat(ep->d_name, &file_info) == 0){
             print_colored_name(ep->d_name, file_info.st_mode);
        }
        else{
            printf("%s\n", ep->d_name);
        }
        printf("\n");

	}
    closedir(dp);
    return 0;
}
