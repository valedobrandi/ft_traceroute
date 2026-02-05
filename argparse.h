#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0

#define MAX_INTERVAL 2147483647.0

typedef struct s_arg42
{
    char *cursive;
    char alias;
    char *arg;
    const char *helper;
} t_arg42;

typedef struct s_arg42_args
{
    int argc;
    char **argv;
} t_arg42_args;

t_arg42 * find_option(t_arg42 *opts, const char type);

int ft_argp(
    t_arg42_args *main_args,
    t_arg42 *opts,
    int (*handler)(int, char *, void *),
    void *user
);

void print_help(t_arg42 *opts);
size_t parse_number(const char *arg, size_t max, int allow_zero);

#endif