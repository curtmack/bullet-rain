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

#include "debug.h"
#include "fixed.h"
#include "geometry.h"
#include "resource.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void fixed_test(void);
void geom_test(void);
void hash_test(void);
void res_test(void);

#define MAIN_MENU_SIZE 8
const char menu[MAIN_MENU_SIZE][32] = {
    "Main menu:",
    " 1. Fixed-point arithmetic test",
    " 2. Geometry test",
    " 3. String hashing test",
    " 4. Resource loading test",
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
    
    init_resources();

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
                geom_test();
                break;
            case '3':
                hash_test();
                break;
            case '4':
                res_test();
                break;
            case 'q':
                finished = 1;
                break;
            default:
                printf("Invalid choice %c\n", choice[0]);
        }
    }
    
    stop_resources();
    
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

#define DIAMETER 760
#define WIDTH 800
void geom_test(void)
{
    char img[WIDTH][WIDTH];
    int i,j,x,y;
    double mx, my, dangle;
    angle_t currang;
    polar_point p;
    rect_point r;
    FILE *out;
    
    const int center = WIDTH/2;
    const fixed_t radius = tofixed(DIAMETER/2,0);
    const double dradius = intpart(radius) + (fracpart(radius)/65536.0);
    const angle_t step = makeangle(0,4096); /* so 4096 steps */
    
    puts("Drawing a big circle and outputting it to geometry.txt");
    
    debug("Filling memory buffer with spaces");
    for (i = 0; i < WIDTH; ++i) {
        for (j = 0; j < WIDTH; ++j) {
            img[i][j] = ' ';
        }
    }
    
    debug("Beginning to draw circle");
    p.r = radius;
    for (currang = zeroangle; currang < maxangle; currang += step) {
        p.t = currang;
        r = polar_to_rect(p);
        x = intpart(r.x + tofixed(center,0));
        y = intpart(r.y + tofixed(center,0));
        img[y][x] = '#';
        
        dangle = intpart(currang) + (fracpart(currang)/65536.0);
        mx = dradius * cos(dangle);
        my = dradius * sin(dangle);
        x = (int)(mx + center);
        y = (int)(my + center);
        img[y][x] = 'C';
    }
    
    out = fopen("geometry.txt", "w");
    for (i = 0; i < WIDTH; i++) {
        for (j = 0; j < WIDTH; j++) {
            fputc(img[i][j], out);
        }
        fputc('\n', out);
    }
    fclose(out);
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

void print_res(resource *res)
{
    /*
     * We're going to assume we'll never call this unless we know
     * it's actually printable data
     */
    char *a = (char*) res->data;
    int64_t num = res->size;
    int i, r;
    
    puts("-------BEGIN-------");
    for (i = 0; i <= num; ++i) {
        r = (a[i] <= 127);
        warnn(r, "Character found that was not printable:", (int)a[i]);
        warnn(r, "It was at location:", i);
        if (r) putchar(a[i]);
    }
    puts("\n--------END--------");
}

void res_test(void)
{
    arclist *arc;
    resource *res;
    
    puts("Loading res/test.tgz");
    arc = load_arc("res/test.tgz");
    puts("Loading successful!");
    
    puts("Printing contents of file chocolat.txt");
    res = get_res("res/test.tgz", "chocolat.txt");
    print_res(res);
    
    puts("Freeing archive");
    free_arc("res/test.tgz");
    puts("Archive freed!");
    
    puts("Getting resource at pocky.txt in a way that should cause a warning");
    res = get_res("res/test.tgz", "pocky.txt");
    
    puts("Printing contents of file pocky.txt");
    print_res(res);
    
    puts("Test done!");
    puts("Leaving archive in memory.");
}

#else /* def SYSTEM_TEST */

void main(int nargs, char *args[])
{
    /* TODO: Should probably write something here? */
}

#endif /* def SYSTEM_TEST */
