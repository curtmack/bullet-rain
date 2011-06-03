/*
 * bullet rain
 * A bullet hell engine by Curtis Mackie
 *
 * Distributed under the terms of the MIT license
 * See LICENSE.TXT in the svn root directory for more information
 */

/*
 * main.c
 * Includes the main() function, which starts the engine and
 * game loop.
 * Also includes an alternative main() that allows for testing
 * of the various subsystems used by the engine if SYSTEM_TEST
 * is set in config.h
 */

#include "compile.h"

#ifdef SYSTEM_TEST

#include "fixed.h"
#include "resource.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void fixed_test(void);
void hash_test(void);

#define MAIN_MENU_SIZE 6
const char menu[MAIN_MENU_SIZE][32] = {
    "Main menu:",
    " 1. Fixed-point arithmetic test",
	" 2. String hashing test",
    " q. Quit",
    "",
    "Your choice:"
};

int main(int nargs, char *args[])
{
    int finished = 0;
    int i;
    char choice[4];

    srand((int) time(NULL));

    puts("BULLET RAIN ENGINE TEST");
    printf("Engine version %s\n", ENGINE_VERSION);

    while (!finished) {
        for (i=0; i < MAIN_MENU_SIZE; ++i) {
            puts(menu[i]);
        }

		/*
		 * We have to get in too many characters
		 * due to weirdness with the program reading
		 * in the newline
		 */
        fgets(choice, 4, stdin);
        switch (choice[0]) {
            case '1':
                fixed_test();
                break;
            case '2':
				hash_test();
				break;
            case 'q':
                finished = 1;
                break;
            default:
                printf("Invalid choice %c\n", choice[0]);
        }
    }
	return 0;
}

inline void print_fixed(fixed_t a)
{
    printf("%d and %d/65536\n", intpart(a), fracpart(a));
}

#define FIXED_TESTS 20
void fixed_test(void)
{
    fixed_t a,b;
    int i;

    for (i=0; i<FIXED_TESTS; ++i) {
        /* generate random a,b */
        a = tofixed(rand()%1024-512,rand()%65536);
        b = tofixed(rand()%1024-512,rand()%65536);

        printf("  A: ");
        print_fixed(a);
        printf("  B: ");
        print_fixed(b);
        printf("A+B: ");
        print_fixed(a+b);
        printf("A-B: ");
        print_fixed(a-b);
        printf("A*B: ");
        print_fixed(fixmul(a,b));
        printf("A/B: ");
        print_fixed(fixdiv(a,b));
        puts("");
    }
}

void hash_test(void)
{
	char a[64];
	sid_t hash;
	
	while (1) {
		puts("Enter a string to hash (blank line quits):");
		fgets(a, 64, stdin);
		clip_string(a);
		if (a[0] == '\0') {
			puts("Okay, exiting hash test.");
			break;
		}
		
		hash = calculate_sid(a);
		
		/* 
		 * note: this may cause warnings if uint32_t is not vanilla int
		 * but it won't hurt anything, god willing
		 */
		printf("SID of '%s' is %08x\n", a, (uint32_t)hash);
	}
}

#else /* def SYSTEM_TEST */

void main(int nargs, char *args[])
{
    /* TODO: Should probably write something here? */
}

#endif /* def SYSTEM_TEST */
