#include <stdio.h>
#define Key 5

void encrypt(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = str[i] ^ Key;
    }
}

int main() {
    char pass[50];
    char id[50];
	printf("Enter Admin ID: ");
    scanf("%s", id);
    printf("Enter password: ");
    scanf("%s", pass);

    encrypt(pass);
	FILE *fp;
	fp=fopen("admin.txt", "w");
	fprintf(fp, "%s\n%s", id, pass);
	fclose(fp);

    return 0;
}
