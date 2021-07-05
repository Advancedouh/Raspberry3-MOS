#include "mmu.h"

#pragma once

#define SZ_16K			0x4000
#define SZ_64K                  0x10000

#define TEXT_OFFSET		0x80000

// Physical memory address space: 0-1G
#define PHYSMEM_START	(0x0UL)
#define PERIPHERAL_BASE (0x3F000000UL)
#define PHYSMEM_END	(0x40000000UL)
#define PAGE_COUNT  (PHYSMEM_END >> BY2PG)
