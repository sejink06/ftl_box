#include "ftl_box.h"
#include "time.h"

/*
 * Number of blocks: 16K
 * Number of pages per block: 256
 * Number of pages: 4M (=16K*256) (Phyiscal key space)
 * Page size: 4 Byte (integer)
 * Key range: 0 ~ 3.6M (=3774874) (Logical key space)
 */

/*
 * Implement your own FTL (Page FTL or Anything else)
 */

int main(int argc, char *argv[])
{
	int i;
	int req_type;
	int32_t requested_value;
	int32_t returned_value;
	int32_t valid_page_copies;
	
	box_create();

	srand(time(NULL));

	// Generate 16M random write requests	
	for (i = 0; i < 16 * 1024 * 1024; ++i)
	{
		requested_value = i % NUMKEY;
		flash_page_write(requested_value);			
	}

	for (i = 0; i < NUMKEY; ++i)
	{
		returned_value = flash_page_read(i);
		//if (returned_value == -1)
			//printf("Value %d cannot be read.\n", i);
		//else
			//printf("Value %d read.\n", returned_value);
	}
	
	valid_page_copies = get_valid_page_copies();
	printf("number of valid page copies: %d\n", valid_page_copies);

	box_destroy();

	return 0;
}
