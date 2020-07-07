#include "ftl_box.h"

_flash_block *flash_blocks;
_mpte *mapping_table;
int32_t curr_free_block;
int32_t next_free_block;
int32_t num_free_blocks;
int32_t valid_page_copies;

int32_t box_create() {
	int i, j;
	flash_blocks = (_flash_block*)malloc(sizeof(_flash_block) * NOB);
	for (i=0; i<NOB; i++) {
		flash_blocks[i].pages = (_page*)malloc(sizeof(_page) * PPB);
		for (j = 0; j < PPB; j++)
		{
			flash_blocks[i].pages[j].valid = 0;
			flash_blocks[i].pages[j].value = -1;
		}
		flash_blocks[i].valid_pages = 0;
		flash_blocks[i].curr_free_page = 0;
	}
	
	curr_free_block = 0;
	next_free_block = 1;
	num_free_blocks = NOB;

	mapping_table = (_mpte*)malloc(sizeof(_mpte) * NUMKEY);
	memset(mapping_table, 0xff, sizeof(_mpte) * NUMKEY);
}
	
int32_t box_destroy() {
	for (int i=0; i<NOB; i++) {
		free(flash_blocks[i].pages);
	}
	free(flash_blocks);
	free(mapping_table);
	return 1;
}

int32_t select_victim()
{
	int i;
	int32_t min_valid_pages;
	int32_t victim;

	min_valid_pages = PPB;

	for (i = 0; i < NOB; i++)
	{
		if (i == curr_free_block)
			continue;
		if (flash_blocks[i].valid_pages < min_valid_pages)
		{
			min_valid_pages = flash_blocks[i].valid_pages;
			victim = i;
			if (min_valid_pages == 0)
				break;
		}
	}
	return victim;
}

void copy_valid_pages(int32_t from)
{
	int i;
	int32_t new_block_num;
	int32_t new_page_num;
	_flash_block *new_block;
	int32_t value;
	
	new_block_num = curr_free_block;
	new_block = &flash_blocks[new_block_num];
	// Nothing to copy
	if (flash_blocks[from].valid_pages == 0)
		return;

	for (i = 0; i < PPB; ++i)
	{
		if (flash_blocks[from].pages[i].valid)
		{
			// Copy valid page
			new_page_num = new_block->curr_free_page++;
			new_block->pages[new_page_num] = flash_blocks[from].pages[i];
			++new_block->valid_pages;
			++valid_page_copies;
			
			// Update mapping table
			value = new_block->pages[new_page_num].value;
			mapping_table[value].block = new_block_num;
			mapping_table[value].page = new_page_num;
		}
	}
}

void flash_block_erase(int32_t victim)
{
	flash_blocks[victim].valid_pages = 0;
	flash_blocks[victim].curr_free_page = 0;
}

void garbage_collect()
{
	int32_t victim;

	// Select victim block
	victim = select_victim();
	// Copy valid pages in the victim block to curr_free_block
	copy_valid_pages(victim);
	// Erase victim block
	flash_block_erase(victim);
	++num_free_blocks;
	next_free_block = victim;
}

int32_t flash_page_read(int32_t value)
{
	int32_t block;
	int32_t page;

	if (mapping_table[value].block == -1)
		return -1;
		
	else
	{
		block = mapping_table[value].block;
		page = mapping_table[value].page;
		return flash_blocks[block].pages[page].value;
	}
}

void flash_page_write(int32_t value)
{
	_flash_block *block; 
	int32_t block_num;
	int32_t page_num; 
	int32_t old_block_num;
	int32_t old_page_num;

	// Wrote this value before, invalidation required
	if (mapping_table[value].block != -1)
	{
		// Invalidate the page with the old value
		old_block_num = mapping_table[value].block;
		old_page_num = mapping_table[value].page;
		flash_blocks[old_block_num].pages[old_page_num].valid = 0;
		--flash_blocks[old_block_num].valid_pages;
	}

	// If only one block is a whole free block, perform GC
	if (num_free_blocks == 1)
		garbage_collect();
	
	// Write the value in flash
	block_num = curr_free_block;
	block = &flash_blocks[curr_free_block];
	page_num = block->curr_free_page;
	block->pages[page_num].valid = 1;
	block->pages[page_num].value = value;
	++block->valid_pages;
	
	// Update the mapping table
	mapping_table[value].block = block_num;
	mapping_table[value].page = page_num;
	
	// Update curr_free_page, curr_free_block, next_free_block
	if (++block->curr_free_page == PPB)
	{
		--num_free_blocks;
		//printf("num_free_blocks: %d\n", num_free_blocks);
		curr_free_block = next_free_block;
		if (num_free_blocks == 1)
			next_free_block = -1;
		else
			++next_free_block;
	}	
}

int32_t get_valid_page_copies()
{
	return valid_page_copies;
}


