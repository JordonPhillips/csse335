//Request the user to type positive numbers until either a zero or a negative is typed, and then show the user how many positives were typed in.
#include <stdio.h>
 
int main() {
    int posCount=-1; 
    int input;

    do  {
        printf("Enter a positive number (0 or less to quit): \n");
        scanf("%d", &input);
        posCount++;
    } while (input > 0);

    printf("Total number of positives: %d\n", posCount);
    return(0);
}
