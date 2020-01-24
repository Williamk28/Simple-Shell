#include <stdio.h>
#include <string.h>
#define STR_LIMIT 512
#define PROMPT "> "
#define DELIMITER " \t|><&;"

int main() {
    char str[STR_LIMIT];
    printf(PROMPT);
    fgets(str, STR_LIMIT, stdin);
    char* token = strtok(str, DELIMITER);

    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, DELIMITER);
    }
}