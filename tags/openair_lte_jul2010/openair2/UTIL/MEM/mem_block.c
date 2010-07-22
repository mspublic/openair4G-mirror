/***************************************************************************
                          mem_block.c  -  description
                             -------------------
  AUTHOR  : Lionel GAUTHIER
  COMPANY : EURECOM
  EMAIL   : Lionel.Gauthier@eurecom.fr


 ***************************************************************************/
#define MEM_BLOCK_C
#include "rtos_header.h"
#include "mem_block.h"
#include "mem_pool.h"
#include "list.h"
#include "LAYER2/MAC/extern.h"

//-----------------------------------------------------------------------------
//#define DEBUG_MEM_MNGT_FREE
//#define DEBUG_MEM_MNGT_ALLOC_SIZE
//#define DEBUG_MEM_MNGT_ALLOC
#ifdef DEBUG_MEM_MNGT_ADDR
#    define   PRINT_MEM_MNGT_ADDR msg
#else
#    define   PRINT_MEM_MNGT_ADDR
                                //
#endif
//-----------------------------------------------------------------------------
#ifdef USER_MODE
u32_t             counters[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
//-----------------------------------------------------------------------------
/*
 * initialize all ures
 */
void           *
pool_buffer_init ()
{
//-----------------------------------------------------------------------------

  u32_t             index, mb_index, pool_index;
  mem_pool       *memory = (mem_pool *) &mem_block_var;
  int             pool_sizes[11] = { MEM_MNGT_MB0_NB_BLOCKS, MEM_MNGT_MB1_NB_BLOCKS,
    MEM_MNGT_MB2_NB_BLOCKS, MEM_MNGT_MB3_NB_BLOCKS,
    MEM_MNGT_MB4_NB_BLOCKS, MEM_MNGT_MB5_NB_BLOCKS,
    MEM_MNGT_MB6_NB_BLOCKS, MEM_MNGT_MB7_NB_BLOCKS,
    MEM_MNGT_MB8_NB_BLOCKS, MEM_MNGT_MB9_NB_BLOCKS,
    MEM_MNGT_MBCOPY_NB_BLOCKS
  };

  memset (memory, 0, sizeof (mem_pool));
  mb_index = 0;
  // LG_TEST
  for (pool_index = 0; pool_index <= MEM_MNGT_POOL_ID_COPY; pool_index++) {
    list_init (&memory->mem_lists[pool_index], "POOL");
    for (index = 0; index < pool_sizes[pool_index]; index++) {
      //memory->mem_blocks[mb_index + index].previous = NULL; -> done in memset 0
      //memory->mem_blocks[mb_index + index].next     = NULL; -> done in memset 0
      switch (pool_index) {
          case 0:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool0[index][0]);
            break;
          case 1:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool1[index][0]);
            break;
          case 2:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool2[index][0]);
            break;
          case 3:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool3[index][0]);
            break;
          case 4:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool4[index][0]);
            break;
          case 5:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool5[index][0]);
            break;
          case 6:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool6[index][0]);
            break;
          case 7:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool7[index][0]);
            break;
          case 8:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool8[index][0]);
            break;
          case 9:
            memory->mem_blocks[mb_index + index].data = &(memory->mem_pool9[index][0]);
            break;
          default:;
            memory->mem_blocks[mb_index + index].data = NULL;   // pool copy

      }
      memory->mem_blocks[mb_index + index].pool_id = pool_index;
      list_add_tail_eurecom (&memory->mem_blocks[mb_index + index], &memory->mem_lists[pool_index]);
    }
    mb_index += pool_sizes[pool_index];
  }

  /*for (pool_index = 0; pool_index < 6; pool_index++) {
     init_list(&memory->mem_raw_lists[pool_index], "RAW");
     for (index=0; index < pool_raw_sizes[pool_index]; index++) {
     switch (pool_index) {
     case 0:
     (( mem_block_t*)(&memory->mem_pool_raw0[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw0[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     case 1:
     (( mem_block_t*)(&memory->mem_pool_raw1[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw1[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     case 2:
     (( mem_block_t*)(&memory->mem_pool_raw2[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw2[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     case 3:
     (( mem_block_t*)(&memory->mem_pool_raw3[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw3[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     case 4:
     (( mem_block_t*)(&memory->mem_pool_raw4[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw4[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     case 5:
     (( mem_block_t*)(&memory->mem_pool_raw5[index][0]))->pool_id = pool_index;
     add_tail((( mem_block_t*)(&memory->mem_pool_raw5[index][0])), &memory->mem_raw_lists[pool_index]);
     break;
     default: msg("[WCDMA][MEM][INIT] out of bounds\n");
     }
     }
     } */
  return 0;
}

//-----------------------------------------------------------------------------
void           *
pool_buffer_clean (void *arg)
{
//-----------------------------------------------------------------------------
  return 0;
}

/*
//-----------------------------------------------------------------------------
void rt_free(void *blockP) {
//-----------------------------------------------------------------------------
   mem_block_t *mb;

  mb = ( mem_block_t*)((u32_t)blockP - sizeof( mem_block_t) + sizeof(void*));
#ifdef DEBUG_MEM_MNGT_FREE
  msg("[MEM_MNGT][FREE] rt_free() %p pool: %d\n", blockP, mb->pool_id);
#endif
  if (mb->pool_id <= 6)  {
    add_tail(mb,  &mem_block_var->mem_raw_lists[mb->pool_id]);
  } else {
    msg("[MEM_MNGT][FREE] ERROR rt_free() unknown pool_id : %d\n", mb->pool_id);
  }
}*/
//-----------------------------------------------------------------------------
void
free_mem_block (mem_block_t * leP)
{
//-----------------------------------------------------------------------------

  if (!(leP)) {
    msg ("[MEM_MNGT][FREE] WARNING FREE NULL MEM_BLOCK\n");
    return;
  }
#ifdef DEBUG_MEM_MNGT_FREE
  msg ("[MEM_MNGT][FREE] free_mem_block() %p pool: %d\n", leP, leP->pool_id);
#endif
#ifdef DEBUG_MEM_MNGT_ALLOC
  check_free_mem_block (leP);
#endif

  if (leP->pool_id <= MEM_MNGT_POOL_ID_COPY) {
    list_add_tail_eurecom (leP, &mem_block_var.mem_lists[leP->pool_id]);
#ifdef DEBUG_MEM_MNGT_ALLOC
    counters[leP->pool_id] -= 1;
#endif
    leP = NULL;                 // this prevent from freeing the block twice
  } else {
    msg ("[MEM_MNGT][FREE] ERROR free_mem_block() unknown pool_id : %d\n", leP->pool_id);
  }
}

/*
//-----------------------------------------------------------------------------
// danger : the free function is not really real time (primitive algorithm, poor performance)
void* rt_alloc(u16_t sizeP) {
//-----------------------------------------------------------------------------
   mem_block_t* le = NULL;
  int               pool_selected;

#ifdef DEBUG_MEM_MNGT_ALLOC_SIZE
  msg("[MEM_MNGT] ALLOC BLOCK SIZE %d bytes\n",sizeP);
#endif
  sizeP = sizeP >> 6;
  pool_selected = 0;

  while ((sizeP)) {
    pool_selected +=1;
    sizeP = sizeP >> 1;
  }

  // pool is selected according to the size requested, now get a block
  // if no block is available pick one in an other pool
  do {
    if ((le=remove_head( &mem_block_var->mem_raw_lists[pool_selected]))) {
      return (void*)(&(le->data));
    }
#ifdef DEBUG_MEM_MNGT_ALLOC
    msg("[MEM_MNGT][ERROR][MINOR] memory pool %d is empty trying next pool\n",pool_selected);
    #ifdef USER_MODE
    display_mem_load();
    check_mem_area((void *)&mem_block_var);
//    break_point();
    #endif
#endif
  } while (pool_selected++ <= 5);
  msg("[MEM_MNGT][ERROR][FATAL] size requested out of bounds or memory pools empty\n");
  return NULL;
}*/
//-----------------------------------------------------------------------------
mem_block_t      *
get_free_mem_block (u16_t sizeP)
{
//-----------------------------------------------------------------------------
  mem_block_t      *le = NULL;
  int             pool_selected;
  int             size;

  if (sizeP > MEM_MNGT_MB9_BLOCK_SIZE) {
    msg ("[MEM_MNGT][ERROR][FATAL] size requested out of bounds\n");


    display_mem_load ();
    mac_xface->macphy_exit("");

    //wcdma_handle_error (WCDMA_ERROR_OUT_OF_MEM_BLOCK);
    return NULL;
  }
  size = sizeP >> 6;
  pool_selected = 0;

  while ((size)) {
    pool_selected += 1;
    size = size >> 1;
  }

  // pool is selected according to the size requested, now get a block
  // if no block is available pick one in an other pool
  do {
    if ((le = list_remove_head (&mem_block_var.mem_lists[pool_selected]))) {
#ifdef DEBUG_MEM_MNGT_ALLOC
      counters[pool_selected] += 1;
#endif
#ifdef DEBUG_MEM_MNGT_ALLOC_SIZE
      msg ("[MEM_MNGT][INFO] ALLOC MEM_BLOCK SIZE %d bytes pool %d (%p)\n", sizeP, pool_selected,le);
#endif
      return le;
    }
#ifdef DEBUG_MEM_MNGT_ALLOC
    msg ("[MEM_MNGT][ERROR][MINOR] memory pool %d is empty trying next pool alloc count = %d\n", pool_selected, counters[pool_selected]);
#    ifdef USER_MODE
    //    display_mem_load ();
    //    check_mem_area ((void *)&mem_block_var);
#    endif
#endif
  } while (pool_selected++ < 9);

  //  Mac_rlc_xface->mac_rlc_exit();
  //wcdma_handle_error (WCDMA_ERROR_OUT_OF_MEM_BLOCK);
  return NULL;
};

//-----------------------------------------------------------------------------
mem_block_t      *
get_free_copy_mem_block (void)
{
//-----------------------------------------------------------------------------
  mem_block_t      *le;

  if ((le = list_remove_head (&mem_block_var.mem_lists[MEM_MNGT_POOL_ID_COPY]))) {
#ifdef DEBUG_MEM_MNGT_ALLOC_SIZE
      msg ("[MEM_MNGT][INFO] ALLOC COPY MEM BLOCK (%p)\n",le);
#endif
    return le;
  } else {
    msg ("[MEM_MNGT][ERROR] POOL COPY IS EMPTY\n");
    //#ifdef DEBUG_MEM_MNGT_ALLOC
    check_mem_area ((void *)&mem_block_var);
    //    break_point ();
    //#endif

    mac_xface->macphy_exit("");
    //    Mac_rlc_xface->mac_rlc_exit();
    //wcdma_handle_error (WCDMA_ERROR_OUT_OF_MEM_BLOCK);

    return NULL;
  }
}

//-----------------------------------------------------------------------------
mem_block_t      *
copy_mem_block (mem_block_t * leP, mem_block_t * destP)
{
//-----------------------------------------------------------------------------

  if ((destP != NULL) && (leP != NULL) && (destP->pool_id == MEM_MNGT_POOL_ID_COPY)) {
    destP->data = leP->data;
  } else {
    msg ("[MEM_MNGT][COPY] copy_mem_block() pool dest src or dest is NULL\n");
  }
  return destP;
}

//-----------------------------------------------------------------------------
void
display_mem_load (void)
{
//-----------------------------------------------------------------------------

  mem_pool       *memory = (mem_pool *) &mem_block_var;

  msg ("POOL 0 (%d elements of %d Bytes): ", MEM_MNGT_MB0_NB_BLOCKS, MEM_MNGT_MB0_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID0]);
  msg ("POOL 1 (%d elements of %d Bytes): ", MEM_MNGT_MB1_NB_BLOCKS, MEM_MNGT_MB1_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID1]);
  msg ("POOL 2 (%d elements of %d Bytes): ", MEM_MNGT_MB2_NB_BLOCKS, MEM_MNGT_MB2_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID2]);
  msg ("POOL 3 (%d elements of %d Bytes): ", MEM_MNGT_MB3_NB_BLOCKS, MEM_MNGT_MB3_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID3]);
  msg ("POOL 4 (%d elements of %d Bytes): ", MEM_MNGT_MB4_NB_BLOCKS, MEM_MNGT_MB4_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID4]);
  msg ("POOL 5 (%d elements of %d Bytes): ", MEM_MNGT_MB5_NB_BLOCKS, MEM_MNGT_MB5_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID5]);
  msg ("POOL 6 (%d elements of %d Bytes): ", MEM_MNGT_MB6_NB_BLOCKS, MEM_MNGT_MB6_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID6]);
  msg ("POOL 7 (%d elements of %d Bytes): ", MEM_MNGT_MB7_NB_BLOCKS, MEM_MNGT_MB7_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID7]);
  msg ("POOL 8 (%d elements of %d Bytes): ", MEM_MNGT_MB8_NB_BLOCKS, MEM_MNGT_MB8_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID8]);
  msg ("POOL 9 (%d elements of %d Bytes): ", MEM_MNGT_MB9_NB_BLOCKS, MEM_MNGT_MB9_BLOCK_SIZE);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID9]);
  msg ("POOL C (%d elements): ", MEM_MNGT_MBCOPY_NB_BLOCKS);
  list_display (&memory->mem_lists[MEM_MNGT_POOL_ID_COPY]);
}

//-----------------------------------------------------------------------------
void
check_mem_area (void *arg)
{
//-----------------------------------------------------------------------------
  int             index, mb_index;
  mem_pool       *memory = (mem_pool *) arg;

  for (index = 0; index < MEM_MNGT_MB0_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[index].data != &(memory->mem_pool0[index][0])) && (memory->mem_blocks[index].pool_id != MEM_MNGT_POOL_ID0)) {
      msg ("[MEM] ERROR POOL0 block index %d\n", index);
    }
  }
  mb_index = MEM_MNGT_MB0_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB1_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool1[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID1)) {
      msg ("[MEM] ERROR POOL1 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB1_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB2_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool2[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID2)) {
      msg ("[MEM] ERROR POOL2 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB2_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB3_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool3[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID3)) {
      msg ("[MEM] ERROR POOL3 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB3_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB4_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool4[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID4)) {
      msg ("[MEM] ERROR POOL4 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB4_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB5_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool5[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID5)) {
      msg ("[MEM] ERROR POOL5 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB5_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB6_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool6[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID6)) {
      msg ("[MEM] ERROR POOL6 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB6_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB7_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool7[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID7)) {
      msg ("[MEM] ERROR POOL7 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB7_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB8_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool8[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID8)) {
      msg ("[MEM] ERROR POOL8 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB8_NB_BLOCKS;
  for (index = 0; index < MEM_MNGT_MB9_NB_BLOCKS; index++) {
    if ((memory->mem_blocks[mb_index + index].data != &(memory->mem_pool9[index][0])) && (memory->mem_blocks[mb_index + index].pool_id != MEM_MNGT_POOL_ID9)) {
      msg ("[MEM] ERROR POOL9 block index %d\n", index);
    }
  }
  mb_index += MEM_MNGT_MB9_NB_BLOCKS;
  for (index = mb_index; index < MEM_MNGT_NB_ELEMENTS; index++) {
    if ((memory->mem_blocks[index].data != NULL) && (memory->mem_blocks[index].pool_id != MEM_MNGT_POOL_ID_COPY)) {
      msg ("[MEM] ERROR POOL COPY block index %d\n", index);
    }
  }
}

//-----------------------------------------------------------------------------
void
check_free_mem_block (mem_block_t * leP)
{
//-----------------------------------------------------------------------------
  int             block_index;
  if ((leP >= &mem_block_var.mem_blocks[0]) && (leP <= &mem_block_var.mem_blocks[MEM_MNGT_NB_ELEMENTS])) {
    block_index = ((u32_t) leP - (u32) (&mem_block_var.mem_blocks[0])) / sizeof (mem_block_t);
    if (block_index < MEM_MNGT_MB0_NB_BLOCKS) {
      if (((u32_t) (leP->data) != (u32) (&(mem_block_var.mem_pool0[block_index][0]))) && (leP->pool_id != MEM_MNGT_POOL_ID0)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index < (MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS)) {
      if ((leP->data != &(mem_block_var.mem_pool1[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID1)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index < MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool2[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID2)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index < MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool3[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID3)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index < MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool4[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID4)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index < MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS + MEM_MNGT_MB5_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool5[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID5)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index <
               MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS + MEM_MNGT_MB5_NB_BLOCKS + MEM_MNGT_MB6_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool6[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID6)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index <
               MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS + MEM_MNGT_MB5_NB_BLOCKS + MEM_MNGT_MB6_NB_BLOCKS +
               MEM_MNGT_MB7_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool7[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID7)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index <
               MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS + MEM_MNGT_MB5_NB_BLOCKS + MEM_MNGT_MB6_NB_BLOCKS +
               MEM_MNGT_MB7_NB_BLOCKS + MEM_MNGT_MB8_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool8[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID8)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    } else if (block_index <
               MEM_MNGT_MB0_NB_BLOCKS + MEM_MNGT_MB1_NB_BLOCKS + MEM_MNGT_MB2_NB_BLOCKS + MEM_MNGT_MB3_NB_BLOCKS + MEM_MNGT_MB4_NB_BLOCKS + MEM_MNGT_MB5_NB_BLOCKS + MEM_MNGT_MB6_NB_BLOCKS +
               MEM_MNGT_MB7_NB_BLOCKS + MEM_MNGT_MB8_NB_BLOCKS + MEM_MNGT_MB9_NB_BLOCKS) {
      if ((leP->data != &(mem_block_var.mem_pool9[block_index][0])) && (leP->pool_id != MEM_MNGT_POOL_ID9)) {
        msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
      }
    }

  } else {
    msg ("[MEM][ERROR][FATAL] free mem block is corrupted\n");
  }
}
