#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <ctype.h>
#include <errno.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

/* SECTION: globals
 * */

static char  *g_licence = "GNU LESSER GENERAL PUBLIC LICENSE Version 3";
static float  g_version = 1.0;  /* numerical representation of the program version */

static size_t g_fcnt = 0;       /* count of files to be processed by the program */
static size_t g_meml = 16;      /* length of the bytes / row (default: 16 bytes / row) */
static size_t g_addrl = 16;     /* length of the displayed memory address (default: 16) */

/* SECTION: function declarations
 * */

extern int hd_procfd(const int);

/* SECTION: main
 * */

int main(int ac, char **av) {
    /* option-processing loop... */
    for (size_t i = 1; i < (size_t) ac; i++) {
        if (*(av[i]) != '-') { g_fcnt++; }
        else {
            char *opt = av[i];

            while (*opt && *opt == '-') { opt++; }
            switch (*opt) {
                case ('v'): {
                    if (strcmp(opt, "v") && strcmp(opt, "version")) {
                        printf("%s: invalid option: %s\n", *av, av[i]); return (1);
                    }
                    
                    printf("%s: %.1f\n", *av, g_version); return (0);
                }

                case ('h'): {
                    if (strcmp(opt, "h") && strcmp(opt, "help")) {
                        printf("%s: invalid option: %s\n", *av, av[i]); return (1);
                    }

                    printf("USAGE: %s [ OPTIONS... ] [ FILE(s)... ]\n\n", *av);
                    printf("%s is a simple utility for inspecting file data on unix-like systems.\n\n", *av);
                    printf("OPTIONS:\n\n");
                    printf("    -h, --help                          display this help message and exit.\n");
                    printf("    -v, --version                       display the version of the program and exit.\n");
                    printf("    -l={arg}, --length={arg}            set the display length of 'Address' and 'Data' sections to {arg} (default: 16).\n");
                    printf("              --length-memory={arg}     set the length of the displayed memory bytes in 'Data' sections to {arg} (default: 16).\n");
                    printf("              --length-address={arg}    set the length of the displayed memory addresses in the 'Address' section to {arg} (default: 16).\n\n");
                    printf("VERSION: %.1f\n", g_version);
                    printf("LICENCE: %s\n", g_licence);
                    return (0);
                }
                
                case ('l'): {
                    char *arg = opt;

                    while (*arg && *arg != '=') { arg++; }
                    if (!*arg) { printf("%s: %s: misused argument option (--opt=arg)\n", *av, av[i]); return (1); }
                    while (*arg && *arg == '=') { *arg = 0, arg++; }
                    if (!*arg) { printf("%s: %s: missing argument value\n", *av, av[i]); return (1); }

                    int value = atoi(arg);
                    if (!strcmp(opt, "length-address"))     { g_addrl = value < 8 ? 8 : value; }
                    else if (!strcmp(opt, "length-memory")) { g_meml  = value < 8 ? 8 : value; }
                    else if (
                        !strcmp(opt, "l") ||
                        !strcmp(opt, "length")
                    ) {
                        g_addrl = g_meml = value < 8 ? 8 : value;
                    }
                    else {
                        printf("%s: invalid option: %s\n", *av, av[i]); return (1);
                    }
                } break;
                
                default: {
                    if (!*opt) { break; }
                    else {
                        printf("%s: invalid option: %s\n", *av, av[i]); return (1);
                    }
                } break;
            }
        }
    }
    
    /* file-processing loop... */
    if (!g_fcnt) { printf("%s: no input provided\n", *av); return (1); }
    for (size_t i = 1, j = g_fcnt; i < (size_t) ac; i++, j--) {
        if (*(av[i]) == '-') { continue; }
        else {
            if (g_fcnt > 1) {
                printf("%s:\n", av[i]);
            }

            int fd = open(av[i], O_RDONLY);
            if (fd == -1) { printf("%s: '%s':%s\n", *av, av[i], strerror(errno)); return (1); }

            struct stat stat;
            if (fstat(fd, &stat) == -1) { return (1); }
            if (S_ISDIR(stat.st_mode)) { printf("%s: '%s': Is a directory\n", *av, av[i]); return (1); }

            if (hd_procfd(fd) == -1) { printf("%s: '%s':%s\n", *av, av[i], strerror(errno)); return (1); }
            if (close(fd) == -1) { printf("%s: '%s':%s\n", *av, av[i], strerror(errno)); return (1); }
            
            if (j > 1) { printf("\n"); }
        }
    }

    return (0);
}

/* SECTION: function definitions
 * */

extern int hd_procfd(const int fd) {
    struct stat stat;
    if (fstat(fd, &stat) == -1) { return (-1); }

    char *data = mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (!data) { return (-1); }

    /* print top frame...
     * */
    putchar('+');
    for (size_t i = 0; i < g_addrl + 2; i++)    { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');
    
    printf("|%-*s|%-*s|%-*s|\n",
           (int) g_addrl + 2, " Address (hex):",
           (int) g_meml * 3 + 1, " Data (hex):",
           (int) g_meml + 2, " Data:"
    );

    putchar('+');
    for (size_t i = 0; i < g_addrl + 2; i++)    { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');

    /* print content frame... 
     * */
    for (size_t i = 0; i < (size_t) stat.st_size; i += g_meml) {
        printf("| ");
        printf("%0*lx", (int) g_addrl ,i);
        printf(" | ");

        for (size_t j = i; j < i + g_meml; j++) {
            printf("%02lx ", j < (size_t) stat.st_size ? (size_t) (data[j] & 0xff) : 0);
        }
        
        printf("| ");
        for (size_t j = i; j < i + g_meml; j++) {
            printf("%c", j < (size_t) stat.st_size ? (isprint(data[j]) ? data[j] : '.') : '.');
        }
        
        printf(" |\n");
    }
    
    /* print bottom frame...
     * */
    putchar('+');
    for (size_t i = 0; i < g_addrl + 2; i++)    { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_meml + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');

    if (munmap(data, stat.st_size) == -1) { return (-1); }
    return (0);
}
