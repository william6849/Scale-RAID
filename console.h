/*
 * =====================================================================================
 *
 *       Filename:  console.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  西元2020年06月05日 14時18分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Lai Liang-Wei (), william68497@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>

typedef bool (*cmd_function)(int argc, char *argv[]);

typedef struct _CMD cmd_obj, *cmd_ptr;
struct _CMD {
    char *name;
    cmd_function operation;
    char *documentation;
    cmd_ptr next;
};

typedef bool (*param_function)(int val);
typedef struct _PARAM param_obj, *param_ptr;
struct _PARAM {
    char *name;
    param_function setter;
    char *documentation;
    param_ptr next;
};
void init_cmd();

void add_cmd(char *name, cmd_function operation, char *documentation);

void add_param(char *name, int *pval, param_function, char *documentation);

void quit_helper(cmd_function qfunc);

bool finish_check();

bool get_int(char *vname, int *loc);

bool start_console(char *readin_file_name);

#endif
