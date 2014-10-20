#include <platform.h>
#include "printf.h"

#include <stdint.h>
#include "n25xxx.h"


/**
 * nvram writes are per page, 1 page = 256kB (8 bits boundary)
 * erase is apparently required before writes, otherwize 0 is stored
 * erase is per-subsector, 1 subsector = 16 pages (12 bits boundary)
 * or per sector, using ``erase_sector`` (1 sector = 16 subsectors).
 *
 * IoT-LAB M3 nodes have 128Mbit nvram, i.e. 16MB, or 64 pages.
 */
static void nvram_rw_example()
{
	static uint8_t buf[256];
	uint32_t addr = 0x3F00;

	printf("nvram read/write example\n");

	/* read current data */
	n25xxx_read(addr, buf, 256);
	printf("addr=0x%0x, data=%d\n", addr, buf[0]);

	/* erase before writing */
	n25xxx_write_enable();
	n25xxx_erase_subsector(addr & 0xF000);

	buf[0] += 1;

	/* perform write */
	n25xxx_write_enable();
	n25xxx_write_page(addr, buf);
}

int main()
{
	platform_init();
	nvram_rw_example();
	platform_run();
	return 0;
}
