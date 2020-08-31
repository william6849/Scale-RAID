#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <inttypes.h>

#include "console.h"
#include "scaleoutmanager.h"

static bool do_adddisk_cmd(int argc,char *argv[]);

static void console_init()
{
    init_cmd();
    add_cmd("adddisk", do_adddisk_cmd,"                | ScaleRAID: Add Data Disk");
}

static bool do_adddisk_cmd(int argc,char *argv[])
{
    if(argc!=2){
        printf("Need argument.\n");
	return false;
    }
    int num = 0;
    if(!get_int(argv[1],&num)){
        printf("Invalid number.\n");
	return false;
    }
    return AddDisk(num);

}
int main(int argc, char *argv[])
{
    console_init();
    bool ok = true;
    ok = start_console(NULL);
    finish_check();
    return ok ? 0 : 1;
}
