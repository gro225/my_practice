#include <stdio.h>
#include <string.h>
#include <regex.h>

#define RED "\033[31m"
#define RESET "\033[0m"

int main(int argc, char **argv) {
	if (argc < 3) {
        printf("Usage: %s <pattern> <file1> [file2 ...]\n", argv[0]);
        return 1;
    }

	char line[256];
	regex_t regex;

	int ret = regcomp(&regex, argv[1], REG_EXTENDED);
    if (ret) {
        printf("Could not compile regex\n");
        return 1;
    }

	for(int i = 2; i < argc; i++) {
		FILE *fp = fopen(argv[i], "r");
    	if (fp == NULL) {
        	printf("grep_utilite: %s: No such file or directory\n", argv[i]);
            continue;
		}

		while (fgets(line, sizeof(line), fp) != NULL){
            const char *ptr = line;
            regmatch_t match;
            if (regexec(&regex, ptr, 1, &match, 0) == 0){
                while (regexec(&regex, ptr, 1, &match, 0) == 0){
                    fwrite(ptr, 1, match.rm_so, stdout);

                    printf(RED);
                    fwrite(ptr + match.rm_so, 1, match.rm_eo - match.rm_so, stdout);
                    printf(RESET);

                    ptr += match.rm_eo;
                }

                printf("%s", ptr);
            }
        }
		fclose(fp);
	}
    regfree(&regex);
    return 0;
}
  
