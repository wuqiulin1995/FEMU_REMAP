// File: ftl_mapping_manager.h
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _MAPPING_MANAGER_H_
#define _MAPPING_MANAGER_H_

#include "vssim_config_manager.h"

//extern int64_t* mapping_table;
//extern void* block_table_start;

//extern unsigned int flash_index;
//extern unsigned int* plane_index;
//extern unsigned int* block_index;
//

struct ssdstate;

void INIT_MAPPING_TABLE(struct ssdstate *ssd);
void TERM_MAPPING_TABLE(struct ssdstate *ssd);

int64_t GET_MAPPING_INFO(struct ssdstate *ssd, int64_t lpn);
int GET_NEW_PAGE(struct ssdstate *ssd, int mode, int mapping_index, int64_t* ppn);

int UPDATE_OLD_PAGE_MAPPING(struct ssdstate *ssd, int64_t lpn);
int UPDATE_NEW_PAGE_MAPPING(struct ssdstate *ssd, int64_t lpn, int64_t ppn);

unsigned int CALC_FLASH(struct ssdstate *ssd, int64_t ppn);
unsigned int CALC_BLOCK(struct ssdstate *ssd, int64_t ppn);
unsigned int CALC_PAGE(struct ssdstate *ssd, int64_t ppn);


//add by hao

void INIT_METADATA_TABLE(struct ssdstate *ssd);


int ftl_meta_write(struct ssdstate *ssd, void *meta);
int ftl_meta_state_get(struct ssdstate *ssd, uint64_t lpn,
        uint32_t *state);
int ftl_meta_write(struct ssdstate *ssd, void *meta);
void *ftl_meta_index(struct ssdstate *ssd, void *meta, uint32_t index);
int ftl_meta_read(struct ssdstate *ssd, void *meta);

#endif
