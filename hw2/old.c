#include <stdlib.h>
#include <stdio.h>

#define MY_BIRTH_YEAR 1993
#define BUFFSIZE 256

int main(int argc, char* argv[]) {
  char buffer[BUFFSIZE];
  char *end = NULL;
  int year;
  int isInt;

  printf("What's your name?\n");
  fgets(buffer, BUFFSIZE, stdin);

  while (1) {
    printf("What year were you born in?\n");
    fflush(stdout);
    fgets(buffer, BUFFSIZE, stdin);
    isInt = sscanf(buffer, "%d", &year);
    if (isInt) break;
  }

  if (year < MY_BIRTH_YEAR) printf("Old.\n");
  return 0;
}
