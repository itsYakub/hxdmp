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

static size_t g_fcnt = 0;   /* count of files to be processed by the program */
static size_t g_mlen = 16;  /* length of the bytes / row (default: 16 bytes / row) */

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
                case ('h'): { printf("%s: help\n", *av); return (0); }
                case ('v'): { printf("%s: 1.0\n", *av); return (0); }
                
                case ('l'): {
                    char *arg = opt;

                    while (*arg && *arg != '=') { arg++; }
                    if (!*arg) { printf("%s: %s: misused argument option (--opt=arg)\n", *av, av[i]); return (1); }
                    while (*arg && *arg == '=') { arg++; }
                    if (!*arg) { printf("%s: %s: missing argument value\n", *av, av[i]); return (1); }

                    g_mlen = atoi(arg);
                    if (g_mlen < 8) { g_mlen = 8; }
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
    for (size_t i = 0; i < 18; i++)             { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');
    
    printf("|%-18s|%-*s|%-*s|\n",
           " Address (hex):",
           (int) g_mlen * 3 + 1, " Data (hex):",
           (int) g_mlen + 2, " Data:"
    );

    putchar('+');
    for (size_t i = 0; i < 18; i++)             { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');

    /* print content frame... 
     * */
    for (size_t i = 0; i < (size_t) stat.st_size; i += g_mlen) {
        printf("| ");
        printf("%016lx", i);
        printf(" | ");

        for (size_t j = i; j < i + g_mlen; j++) {
            printf("%02lx ", j < (size_t) stat.st_size ? (size_t) (data[j] & 0xff) : 0);
        }
        
        printf("| ");
        for (size_t j = i; j < i + g_mlen; j++) {
            printf("%c", j < (size_t) stat.st_size ? (isprint(data[j]) ? data[j] : '.') : '.');
        }
        
        printf(" |\n");
    }
    
    /* print bottom frame...
     * */
    putchar('+');
    for (size_t i = 0; i < 18; i++)             { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen * 3 + 1; i++) { putchar('-'); } putchar('+');
    for (size_t i = 0; i < g_mlen + 2; i++)     { putchar('-'); } putchar('+');
    putchar('\n');

    if (munmap(data, stat.st_size) == -1) { return (-1); }
    return (0);
}
