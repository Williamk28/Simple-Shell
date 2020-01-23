#include <stdio.h>
#define STR_LIMIT 512

int main() {
    char str[STR_LIMIT];
    printf("> ");
    fgets(str, STR_LIMIT, stdin);
}