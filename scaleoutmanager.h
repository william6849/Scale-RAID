#include <stddef.h>
#include <stdint.h>

#define DEFAULT_DATA_DISK_NUM 5
#define DEFAULT_PARITY_DISK_NUM 2
#define STRIPE_LBA_NUM (data_disk_num + DEFAULT_PARITY_DISK_NUM)
#define DEFAULT_COLUMN DEFAULT_DATA_DISK_NUM
#define DEFAULT_ROW (DEFAULT_DATA_DISK_NUM-1)
extern size_t data_disk_num;

int32_t AddDisk(uint32_t num);
