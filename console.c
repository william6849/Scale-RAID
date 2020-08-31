/*
 * =====================================================================================
 *
 *       Filename:  console.c
 *
 *    Description:  Command console with interpreter
 *
 *        Version:  1.0
 *        Created:  西元2020年06月05日 14時04分41秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lai Liang-Wei (), william68497@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#include <ctype.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <console.h>

#include "scaleoutmanager.h"
/* global variable */
static cmd_ptr cmd_list = NULL;
static param_ptr param_list = NULL;
static bool quit_flag = false;
static char *prompt = "CMD> ";
static bool prompt_flag = true;
static bool block_flag = false;
/* Implement RIO package */
#define RIO_BUFSIZE 8192 /* Allow file size */

static char linebuf[RIO_BUFSIZE];

typedef struct _RIO rio_obj, *rio_ptr;
struct _RIO {
    int fd;                /* File descriptor */
    int cnt;               /* Unread bytes in internal buffer */
    char *bufptr;          /* Next unread byte in internal buffer */
    char buf[RIO_BUFSIZE]; /* Internal buffer */
    rio_ptr prev;          /* Next element in stack */
};

static rio_ptr buf_stack = NULL;
static char linebuf[RIO_BUFSIZE];

int fd_max = 0;
bool push_file(char *filename)
{
    int fd = filename ? open(filename, O_RDONLY) : STDIN_FILENO;
    if (fd < 0)
        return false;

    if (fd > fd_max)
        fd_max = fd;

    rio_ptr riop = malloc(sizeof(rio_obj));
    if (!riop)
        return false;
    riop->fd = fd;
    riop->cnt = 0;
    riop->bufptr = riop->buf;
    riop->prev = buf_stack;
    buf_stack = riop;

    return true;
}
void pop_file()
{
    if (buf_stack) {
        rio_ptr file = buf_stack;
        buf_stack = buf_stack->prev;
        close(file->fd);
        free(file);
    }
}

static bool do_quit_cmd(int argc, char *argv[]);
static bool do_help_cmd(int argc, char *argv[]);

void init_cmd()
{
    cmd_list = NULL;
    param_list = NULL;
    buf_stack = NULL;

    add_cmd("quit", do_quit_cmd, "                | Exit program");
    add_cmd("help", do_help_cmd, "                | Show documentation");
}

void add_cmd(char *name, cmd_function operation, char *documentation)
{
    cmd_ptr next_cmd = cmd_list;
    cmd_ptr *last_cmd = &cmd_list;

    while (next_cmd && strcmp(name, next_cmd->name) > 0) {
        last_cmd = &next_cmd->next;
        next_cmd = next_cmd->next;
    }
    cmd_ptr new = (cmd_ptr) malloc(sizeof(cmd_obj));
    if (new == NULL)
        abort;
    new->name = name;
    new->operation = operation;
    new->documentation = documentation;
    new->next = next_cmd;
    *last_cmd = new;
}

void add_param(char *name,
               int *pval,
               param_function setter,
               char *documentation)
{
    param_ptr next_param = param_list;
    param_ptr *last_param = &param_list;
    while (next_param && strcmp(name, next_param->name) > 0) {
        last_param = &next_param->next;
        next_param = next_param->next;
    }
    param_ptr new = malloc(sizeof(param_obj));
    if (new == NULL)
        abort();
    new->name = name;
    new->setter = setter;
    new->documentation = documentation;
    new->next = next_param;
    *last_param = new;
}

static char **parse_args(char *line, int *argcp)
{
    char *scan = line;
    char c = 0;
    int argc = 0;
    bool skipping = true;
    int len = strlen(line);
    char *buf = malloc(len + 1);
    char *current = buf;
    /* Split line into sevral componien */
    while ((c = *scan++) != '\0') {
        if (isspace(c)) {
            if (!skipping) {
                *current++ = '\0';
                skipping = true;
            }
        } else {
            if (skipping) {
                argc++;
                skipping = false;
            }
            *current++ = c;
        }
    }
    char **argv = calloc(argc, sizeof(char *));
    current = buf;
    for (int i = 0; i < argc; i++) {
        argv[i] = malloc(strlen(current) + 1);
        if (argv[i] == NULL)
            return NULL;
        strncpy(argv[i], current, strlen(current) + 1);
        current += strlen(argv[i]) + 1;
    }
    free(buf);
    *argcp = argc;
    return argv;
}

static bool interpret_cmda(int argc, char *argv[])
{
    bool ok = true;
    if (argc == 0)
        return true;
    cmd_ptr cmdp = cmd_list;

    while (cmdp && strcmp(cmdp->name, argv[0]) != 0)
        cmdp = cmdp->next;
    if (cmdp != NULL) {
        ok = cmdp->operation(argc, argv);
        if (!ok) {
            printf("cmd:%s operfail", cmdp->name);
        }
    } else {
        printf("Unknown command \'%s\'\n", argv[0]);
        ok = false;
    }
    return ok;
}

static bool interpret_cmd(char *cmdline)
{
    if (!cmdline)
        return true;
    int argc = 0;
    char **argv = parse_args(cmdline, &argc);
    bool ok = interpret_cmda(argc, argv);
    for (int i = 0; i < argc; i++)
        free(argv[i]);
    free(argv);
    return ok;
}

void quit_helper(cmd_function qfunc) {}
/* Built-in commands */
static bool do_quit_cmd(int argc, char *argv[])
{
    bool ok = true;
    cmd_ptr cmdp = cmd_list;
    while (cmdp) {
        cmd_ptr old = cmdp;
        cmdp = cmdp->next;
        free(old);
    }

    param_ptr paramp = param_list;
    while (paramp) {
        param_ptr old = paramp;
        paramp = paramp->next;
        free(old);
    }

    while (buf_stack)
        pop_file();
    quit_flag = true;
    return ok;
}

static bool do_help_cmd(int argc, char *argv[])
{
    if (argc == 1) {
        cmd_ptr cmdp = cmd_list;
        printf("Commands:\n");
        while (cmdp) {
            printf("\t%s\t%s\n", cmdp->name, cmdp->documentation);
            cmdp = cmdp->next;
        }

        param_ptr paramp = param_list;
        printf("Options:\n");
        while (paramp) {
            printf("\t%d\t%s\n", paramp->name, paramp->documentation);
            paramp = paramp->next;
        }
    }
    return true;
}

bool finish_check() {}

static char *readline()
{
    int cnt = 0;
    char *lbufp = linebuf;
    char c;
    if (!buf_stack)
        return NULL;

    for (cnt = 0; cnt < RIO_BUFSIZE - 2; cnt++) {
        if (buf_stack->cnt <= 0) {
            buf_stack->cnt = read(buf_stack->fd, buf_stack->buf,
                                  RIO_BUFSIZE); /* Read will save offset */
            buf_stack->bufptr = buf_stack->buf;
            if (buf_stack->cnt <= 0) {
                /* got EOF */
                pop_file();  // why?
                if (cnt > 0) {
                    *lbufp++ = '\n';
                    *lbufp++ = '\0';
                    return linebuf;
                }
                return NULL;
            }
        }
        c = *buf_stack->bufptr++;
        *lbufp++ = c;
        buf_stack->cnt--;
        if (c == '\n')
            break;
    }
    if (c != '\n')  // Hit buffer limit
        *lbufp++ = '\n';
    *lbufp++ = '\0';
    return linebuf;
}

bool readReady()
{
    for (int i = 0; buf_stack && i < buf_stack->cnt; i++) {
        if (buf_stack->bufptr[i] == '\n') {
            return true;
        }
    }
    return false;
}

static bool cmd_done()
{
    return !buf_stack || quit_flag;
}

static int cmd_select(int nfds,
                      fd_set *readfds,
                      fd_set *writefds,
                      fd_set *exceptfds,
                      struct timeval *timeout)
{
    char *cmdline;

    while (!block_flag && readReady()) {
        cmdline = readline();
        interpret_cmd(cmdline);
    }

    if (cmd_done())
        return 0;

    int infd;
    fd_set local_readset;
    if (!block_flag) {
        if (!readfds)
            readfds = &local_readset;
        infd = buf_stack->fd;
        FD_SET(infd, readfds);
        if (infd == STDIN_FILENO && prompt_flag) {
            printf("%s", prompt);
            fflush(stdout);
        }
        if (infd >= nfds)
            nfds = infd + 1;
    }
    if (nfds == 0)
        return 0;

    int result = select(nfds, readfds, writefds, exceptfds, timeout);
    if (result <= 0)
        return result;

    infd = buf_stack->fd;
    if (readfds && FD_ISSET(infd, readfds)) {
        /* Commandline input available */
        FD_CLR(infd, readfds);
        result--;
        cmdline = readline();
        if (cmdline)
            interpret_cmd(cmdline);
    }
    return result;
}

bool start_console(char *infile_name)
{
    if (!push_file(infile_name)) {
        printf("faild to push \'%s\' at start", infile_name);
        return false;
    }
    while (!cmd_done())
        cmd_select(0, NULL, NULL, NULL, NULL);
    return true;
}

bool get_int(char *vname, int *loc)
{
    char *end = NULL;
    long int v = strtol(vname, &end, 0);
    if (v == LONG_MIN || *end != '\0')
        return false;

    *loc = (int) v;
    return true;
}

