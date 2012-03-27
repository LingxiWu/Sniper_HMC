#ifndef __MEM_COMPONENT_H__
#define __MEM_COMPONENT_H__

class MemComponent
{
   public:
      enum component_t
      {
         INVALID_MEM_COMPONENT = 0,
         MIN_MEM_COMPONENT,
         CORE = MIN_MEM_COMPONENT,
         FIRST_LEVEL_CACHE,
         L1_ICACHE = FIRST_LEVEL_CACHE,
         L1_DCACHE,
         L2_CACHE,
         L3_CACHE,
         L4_CACHE,
         /* more, unnamed stuff follows.
            make sure that MAX_MEM_COMPONENT < 32 as pr_l2_cache_block_info.h contains a 32-bit bitfield of these things
         */
         LAST_LEVEL_CACHE = 20,
         DRAM_DIR,
         DRAM,
         MAX_MEM_COMPONENT = DRAM,
         NUM_MEM_COMPONENTS = MAX_MEM_COMPONENT - MIN_MEM_COMPONENT + 1
      };
};

#endif /* __MEM_COMPONENT_H__ */