#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>

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
    else{
        permissions[0] = '?';
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

//Последнее изменение 
void print_last_modified(time_t time) {
    char date[20];
    strftime(date, sizeof(date), "%b %d %H:%M", localtime(&time));
    printf("%s ", date);
}

//Вывод с цветом
void print_colored_name(const char* name, mode_t mode, const char* full_path, int long_format) {
    if (S_ISDIR(mode)) {
        printf(COLOR_DIR "%-8s" COLOR_RESET, name);
    }
    else if (S_ISREG(mode) && (mode & S_IXUSR)) {
        printf(COLOR_EXE "%-8s" COLOR_RESET, name);
    }
    else if (S_ISLNK(mode)) {
        if (long_format) {
            char link_target[PATH_MAX];
            ssize_t len = readlink(full_path, link_target, sizeof(link_target) - 1);
            if (len != -1) {
                link_target[len] = '\0';
                printf(COLOR_LNK "%s" COLOR_RESET " -> %-8s", name, link_target);
            } 
            else {printf(COLOR_LNK "%-8s" COLOR_RESET, name);}
        }
        else{
            printf(COLOR_LNK "%-8s" COLOR_RESET, name);
        }
    }
    else {
        printf("%-8s", name);
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

    const char* dir_path = (argc > optind) ? argv[optind] : ".";

    DIR* dp = opendir(dir_path);
    if (dp == NULL) {
        perror("opendir");
        return 1;
    }

    struct stat file_info;

    struct dirent **namelist;
    int num_files = scandir(dir_path, &namelist, NULL, alphasort);

    // Первый проход по дирректориям для подсчета кол-вa блоков
    for (int i = 0; i < num_files; i++){
        struct dirent *ep = namelist[i];

        if (!show_all && ep->d_name[0] == '.') {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, ep->d_name);

        if (lstat(full_path, &file_info) == 0){
            total_blocks += file_info.st_blocks; 
        }
        else {
            perror("lstat");
        }
    }

    if (long_format) {
        printf("total %lld\n", total_blocks / 2);
    }

    // Второй проход по дирректориями для вывода информации
    for (int i = 0; i < num_files; i++){
        struct dirent *ep = namelist[i];
        if (!show_all && ep->d_name[0] == '.') {
            continue;
        }
        
        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s",dir_path, ep->d_name);
        if (lstat(full_path, &file_info) == 0) {
            if (long_format) {
                print_file_permissions(file_info.st_mode);
                printf("%4ld ", (long)file_info.st_nlink);
                struct passwd *pw = getpwuid(file_info.st_uid);
                struct group  *gr = getgrgid(file_info.st_gid);
                if (pw != NULL) {
                    printf("%-4s ", pw->pw_name);
                } 
                else {
                    printf("%-4d ", file_info.st_uid); 
                } 

                if (gr != NULL) {
                    printf("%-4s ", gr->gr_name);  
                }        
                else{
                    printf("%-4d ", file_info.st_gid);  
                }

                printf("%*ld ", (int)sizeof(file_info.st_size), file_info.st_size);
                print_last_modified(file_info.st_mtime); 
        
            }
            print_colored_name(ep->d_name, file_info.st_mode, full_path, long_format);
            printf("\n");
        }
        else { perror("lstat");}
	}

    // Освобождение памяти
    for (int i = 0; i < num_files; i++) {
        free(namelist[i]);
    }
    free(namelist);

    closedir(dp);
    return 0;
}
