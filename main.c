/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  西元2020年06月05日 13時21分18秒
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

#include <console.h>

int main(int argc, char *argv[])
{
    init_cmd();
    bool ok = true;
    ok = start_console(NULL);
    finish_check();
    return ok ? 0 : 1;
}
