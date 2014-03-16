#include <stdio.h>
#include <math.h>

int main(int argc, char* argv[]) {
    long long n = 0;
    long long inc = 1;
    int direction = 1;
    char ch = '\n';

    printf("Think of a number, and I'll try to guess it!\n");
    while (1) {
        do {
            if (ch == '\n') printf("Is your number %lld\? ('y' for yes, 'n' for no)\n", n);
            ch = getchar();
            while (ch ==32) ch = getchar();
        } while (ch != 'y' && ch != 'n');

        if (ch == 'y') break;

        do {
            if (ch == '\n') printf("Was my guess too high ('h') or too low ('l')?\n");
            ch = getchar();
            while (ch == 32) ch = getchar();
        } while (ch != 'h' && ch != 'l');

        if (ch == 'l') {
            if (direction == -1) inc = 1;
            direction = 1;

            n += inc;
            inc += inc;
        } else if (ch == 'h') {
            if (direction == 1) inc = 1;
            direction = -1;

            n -= inc;
            inc += inc;
        }
    }

    printf("Your number is %lld!\n", n);
    return 0;
}
