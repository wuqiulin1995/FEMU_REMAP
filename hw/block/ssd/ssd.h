// File: ssd.h
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _SSD_H_
#define _SSD_H_

//#include "hw.h"
#include <stdint.h>
#include "vssim_config_manager.h"


//FILE *fp;
void SSD_INIT(struct ssdstate *ssd);
void SSD_TERM(struct ssdstate *ssd);

int64_t SSD_WRITE(struct ssdstate *ssd, struct request_meta *request1);
int64_t SSD_READ(struct ssdstate *ssd, unsigned int length, int64_t sector_nb);
void SSD_DSM_TRIM(struct ssdstate *ssd, unsigned int length, void* trim_data);
int SSD_IS_SUPPORT_TRIM(struct ssdstate *ssd);

int femu_discard_process(struct ssdstate *ssd, uint32_t length, int64_t sector_nb);

int SSD_REMAP(struct ssdstate *ssd, uint64_t src_lpn, uint64_t dst_lpn, uint32_t len, uint32_t ope);

#endif
