#include "scaleoutmanager.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "chunk.h"


#define abs(x) ((x) < 0 ? -(x) : (x))
uint32_t g_sl = 0;
uint32_t g_vd = 0;

size_t source_region;
size_t target_region;
size_t supply_region;
double supply_ratio;
size_t full_regions;

size_t data_disk_num = DEFAULT_DATA_DISK_NUM;  // PDs

static int32_t ReOrganizer(uint32_t);
static int32_t ReConstructor();
static uint32_t GetScaleLevel();
static void ScaleOutManager();
static chunk_set_p VictimFullRegions(const size_t row,
                                     const size_t column,
                                     const uint32_t sl);
static chunk_set_p VictimSupplyRegions(const size_t row,const size_t column);

/* Cale */
#define lcm(a, b) (a * b) / gcd(a, b);
static int32_t gcd(int32_t x, int32_t y)
{
    if (y == 0) {
        return x;
    } else {
        return gcd(y, x % y);
    }
}

static int32_t mod(int32_t a, int32_t b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

/* impl */
static int32_t ReOrganizer(uint32_t oldsl)
{
    uint32_t old_row = DEFAULT_ROW + (oldsl + 1) * oldsl;
    uint32_t new_row = DEFAULT_ROW + (g_sl + 1) * g_sl;
    int32_t m = lcm(old_row, new_row);
    source_region = m / old_row;
    target_region = m / new_row;
    supply_region = source_region - target_region;
    uint32_t hFR = floor(supply_region/target_region) * target_region;
    supply_ratio = (double)(supply_region - hFR) / (double)target_region;
    printf("Old Row:%u\n",old_row);
    printf("New Row:%u\n",new_row);
    printf("Supply Region:%zu\n",supply_region);
    printf("Supply Ratio:%f\n",supply_ratio);
}

static void calculate(uint32_t frs,uint32_t sps,uint32_t xort)
{

}
static int32_t ReConstructor() {}

static uint32_t GetScaleLevel()
{
    // onlt when ad>vd
    int32_t surples = (data_disk_num - DEFAULT_DATA_DISK_NUM);
    while (true) {
        surples -= (2 * g_sl);  // add 0,2,4,6... , if add 6 disk, sl will be 2
        if (surples <= 0) {
            return g_sl;
        }
        g_sl++;
    }
}

static chunk_set_p VictimFullRegions(const size_t row,
                                     const size_t column,
                                     const uint32_t sl)
{
    uint32_t vics=0;
    chunk_set_p frset = NULL;
    int32_t victim_length = sl + 2;
    for (uint32_t i = 1; i <= row; i++) {
        for (uint32_t j = 1; j <= abs(victim_length); j++) {
            chunk_set_p ncs = (chunk_set_p) malloc(sizeof(chunk_set));
            if (!ncs) {
                //-NOMEM
                exit(-ENOMEM);
            }
            if (victim_length > 0) {
                CHUNK_SET(ncs, i, (DEFAULT_COLUMN - j + 1));
                printf("FrSet:(%u,%u) Q%d\n", i, (DEFAULT_COLUMN - j + 1),(mod((DEFAULT_COLUMN-j+1-i),column)+1));
            } else if (victim_length < 0) {
                CHUNK_SET(ncs, i, j);
                printf("FrSet:(%u,%u) Q%d\n", i, j,(mod((j-i),column)+1));
            }
            CHUNK_ADD(frset, ncs);
            vics++;
        }
        victim_length--;
    }
    printf("FRSET:VIC:%d\n", vics);
    return frset;
}
static chunk_set_p VictimSupplyRegions(const size_t row,const size_t column)
{
    uint32_t vics=0;
    chunk_set_p spset = NULL;
    uint32_t row_count = row * supply_ratio;
    for (uint32_t i = (row*column*supply_region); i < (row*column*supply_region)+row_count*column;i++){
        chunk_set_p ncs = (chunk_set_p) malloc(sizeof(chunk_set));
        if(!ncs){
            exit(-ENOMEM);
        }
        uint32_t sub_set_index = (i / column)+1;
        CHUNK_SET(ncs, sub_set_index, (mod((i-sub_set_index+1),DEFAULT_COLUMN+(g_sl+1)*g_sl)+1));
        CHUNK_ADD(spset, ncs);
        printf("SpSet:(%d,%llu) Q%d\n", sub_set_index, (i%column) + 1,mod(((i%column)-sub_set_index+1),DEFAULT_COLUMN+(g_sl+1)*g_sl)+1);
        vics++;
    }
    printf("SPSET:VIC:%d\n", vics);
    return spset;
}

static void ScaleOutManager(const uint32_t n)
{
    const uint32_t ad = n;
    printf("Current virtual disk:%u\n",g_vd);
    if (g_vd >= ad) {
	printf("Only made virtual disk from %u to %u\n",g_vd, (g_vd-ad));
        g_vd -= ad;  // replace vd to pd
    } else {
        uint32_t oldsl = g_sl;
        printf("Old Scale Level: %u\n",oldsl);
        g_sl = GetScaleLevel();
        printf("New Scale Level: %u\n",g_sl);
        g_vd = (DEFAULT_DATA_DISK_NUM + (g_sl + 1) * g_sl) -
               data_disk_num;  // request physical device
        ReOrganizer(oldsl);
        chunk_set_p vfr = VictimFullRegions((DEFAULT_ROW+ (oldsl + 1) * oldsl),(DEFAULT_COLUMN+ (oldsl + 1) * oldsl),oldsl);
        chunk_set_p vsr = VictimSupplyRegions((DEFAULT_ROW+ (oldsl + 1) * oldsl),(DEFAULT_COLUMN+ (oldsl + 1) * oldsl));
        ReConstructor();
        free(vfr);
        free(vsr);
    }
    printf("Virtual disk after scaling:%u\n",g_vd);
}

int32_t AddDisk(uint32_t num)
{
    printf("Add %hd disk\n",num);
    data_disk_num += num;
    printf("Total data disk :%lld\n",data_disk_num);
    ScaleOutManager(num);
    return 1;
}
