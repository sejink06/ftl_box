#ifndef __H_FTL_BOX__
#define __H_FTL_BOX__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NOB (16 * 1024)
//#define NOB (5)
//#define PPB (4)
#define PPB (256)
#define NOP ((NOB)*(PPB)) // 4M
#define PAGESIZE (4)

#define NUMKEY 3774874

#define NIL -1
#define PAGE_READ 0
#define PAGE_WRITE 1
#define BLOCK_ERASE 2

typedef struct page {
	unsigned valid : 1;
	int32_t value;
} _page;

typedef struct flash_block {
	_page *pages;
	//int32_t erase_cnt;
	int32_t valid_pages;
	int32_t curr_free_page;
} _flash_block;

typedef struct mpte {
	int32_t block;
	int32_t page;
} _mpte;

int32_t box_create();
int32_t box_destroy();
int32_t select_victim();
void copy_valid_pages(int32_t from);
void flash_block_erase(int32_t victim);
void garbage_collect();
void flash_page_write(int32_t value);
int32_t flash_page_read(int32_t value);
int32_t get_valid_page_copies();

#endif // !__H_FTL_BOX__
