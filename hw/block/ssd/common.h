// File: common.h
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include <sys/time.h>
#include "ftl_type.h"
#include "god.h"

// #define MAX_LPN_CNT 100
#define DATA_BITS_NVME 12
#define PI_BYTES_NVME 24

#define TOTAL_OOB_SEG 81920 // 80MB NVRAM
#define OOB_ENTRY_PER_SEG 64   // 8KB segment
#define OOB_ENTRY_BYTES 16
#define INVALID_ENTRY_THRE 0.05 // 无效条目大于总条目的5%可做NVRAM GC
#define NVRAM_READ_DELAY 50 // 50ns / 64B (PCM)
#define NVRAM_WRITE_DELAY 500 // 500ns /64B (PCM)

// #define X_FTL
#define WAL_WRITE 100
#define CP_WRITE 102

// #define DUP_RATIO 50  // 50% duplicate data
#define UNIQUE_PAGE_NB 4200000 // lpn_valid * (100 - dup_ratio) / 100

#ifdef DUP_RATIO
#define FING_DELAY 32000 // 指纹计算延时 32000ns
#else
#define FING_DELAY 0 // 指纹计算延时 ns
#endif

#define DEDUP_WRITE 103
#define FS_GC_WRITE 104

#define REMAP_CKPT 2
#define REMAP_COPY 3
#define REMAP_MOVE 4

 //Multiple bloom filters
#define FILTER_NUMBER 4
#define FILTER_SIZE 14
#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))

#define STAT_COUNT

#ifdef STAT_COUNT
#define STAT_OUTPUT_FILE ("/home/nvm/f2fsgc_remap_seg1K_80M_fileserver.csv")
#define PRINT_INTERVAL 5	//输出的时间间隔（秒）
#endif

#define SUPERBLOCK
#define SB_DEBUG

#ifdef SB_DEBUG	
typedef struct SUPERBLOCK_DEBUG
{
	int plane_nb;
	int block_nb;
}sb_debug;
#endif

#define WHOLE_BLOCKING      1
#define CHANNEL_BLOCKING    2
#define CHIP_BLOCKING       3

//extern int GC_MODE;
//extern int64_t *gc_slot;

//extern static long mygc_cnt = 0;
//extern static long mycopy_page_nb = 0;
//extern static long last_time = 0;

/* FTL */
/* VSSIM Function */
//#define MONITOR_ON

/* Page Mapping FTL */
#ifdef PAGE_MAP
	#define GC_ON			/* Garbage Collection for PAGE MAP */
	#define GC_TRIGGER_OVERALL
	#define GC_VICTIM_OVERALL
	//#define WRITE_NOPARAL
	//#define FTL_MAP_CACHE		/* FTL MAP Cache for PAGE MAP */
#endif

/* Block Mapping FTL */
#ifdef BLOCK_MAP
	#define GC_ON
	#define GC_TRIGGER_OVERALL
#endif

/* Hybrid Mapping FTL */
#ifdef FAST_FTL
	//#define FTL_FAST1
	//#define LAST_SEQ_COND
	//#define SSD_SEQ_W_VALIDATION
#endif
#ifdef LAST_FTL
	//#define FAST_SEQ_COND
#endif

/* Disaggregated Mapping */
#ifdef DA_MAP
	#define BM_START_SECTOR_NB 8388608 // 8388608: 4GByte for PM, Remains for BM
	#define GC_ON
	//#define GC_TRIGGER_OVERALL
#endif

//#define REMAIN_IO_DELAY	/* Remain delay for validation */
//#define O_DIRECT_VSSIM	/* O_DIRECT IO mode */

/* VSSIM Benchmark*/
#ifndef VSSIM_BENCH
  //#define DEL_QEMU_OVERHEAD
  //#define FIRM_IO_BUFFER	/* SSD Read/Write Buffer ON */
  //#define SSD_THREAD		/* Enable SSD thread & SSD Read/Write Buffer */
  //#define SSD_THREAD_MODE_1
//#define SSD_THREAD_MODE_2	/* Pending WB Flush until WB is full */
#endif

/* HEADER - VSSIM CONFIGURATION */
#include "vssim_config_manager.h"

/* HEADER - FTL MODULE */
#include "ftl.h"
#include "ftl_perf_manager.h"
#include "ftl_inverse_mapping_manager.h"

/* HEADER - VSSIM BENCH */
#ifndef VSSIM_BENCH
	#include "ssd_util.h"
#endif

/* HEADER - SSD MODULE */
#include "ssd_io_manager.h"
#include "ssd_log_manager.h"

/* HEADER - FIRMWARE */
#include "firm_buffer_manager.h"

/* HEADER - FTL Dependency */
#if defined PAGE_MAP || defined BLOCK_MAP
	#include "ftl_cache.h"
	#include "ftl_gc_manager.h"
	#include "ftl_mapping_manager.h"
#endif
#if defined FAST_FTL || defined LAST_FTL
	#include "ftl_log_mapping_manager.h"
	#include "ftl_data_mapping_manager.h"
#endif
#ifdef DA_MAP
	#include "ftl_gc_manager.h"
	#include "ftl_pm_mapping_manager.h"
	#include "ftl_bm_mapping_manager.h"
	#include "ftl_block_manager.h"
#endif

/*************************
  Define Global Variables 
 *************************/

/* Function Return */
#define SUCCESS		1
#define FAIL		0

/* Write Type */
#if defined FAST_FTL || defined LAST_FTL
#define NEW_SEQ_WRITE   20
#define CONT_SEQ_WRITE  21
#define NEW_RAN_WRITE   22
#define HOT_RAN_WRITE	23
#define COLD_RAN_WRITE	24
#endif

// /* Block Type */
// #define EMPTY_BLOCK             30
// #define SEQ_BLOCK               31
// #define EMPTY_SEQ_BLOCK         32
// #define RAN_BLOCK               33
// #define EMPTY_RAN_BLOCK         34
// #define RAN_COLD_BLOCK		35
// #define EMPTY_RAN_COLD_BLOCK	36
// #define	RAN_HOT_BLOCK		37
// #define	EMPTY_RAN_HOT_BLOCK	38
// #define DATA_BLOCK              39
// #define EMPTY_DATA_BLOCK        40

/*New Block Type */

#define DATA_BLOCK                39  //ready to change
#define EMPTY_BLOCK               100

/* GC Copy Valid Page Type */
#define VICTIM_OVERALL	41
#define VICTIM_INCHIP	42
#define VICTIM_NOPARAL	43

/* Page Type */
#define VALID		50
#define INVALID		51

/* Caller Type for Hybrid FTL */
#if defined FAST_FTL || defined LAST_FTL
#define INIT            60
#define SEQ_MERGE       61
#define RAN_MERGE       62
#endif

/* Random Log Block Type */
#ifdef LAST_FTL
#define COLD_RAN        70
#define HOT_RAN         71
#endif

/* Perf Checker Calloc Type */
#define CH_OP		80
#define REG_OP		81
#define LATENCY_OP	82

#define CHANNEL_IS_EMPTY 700
#define CHANNEL_IS_WRITE 701
#define CHANNEL_IS_READ  702
#define CHANNEL_IS_ERASE 703

#define REG_IS_EMPTY 		705
#define REG_IS_WRITE 		706
#define REG_IS_READ  		707
#define REG_IS_ERASE 		708

#define NOOP			800
#define READ  			801
#define WRITE			802
#define ERASE 			803
#define GC_READ			804
#define GC_WRITE		805
#define SEQ_WRITE		806
#define RAN_WRITE		807
#define RAN_COLD_WRITE		808
#define RAN_HOT_WRITE		809
#define SEQ_MERGE_READ		810
#define RAN_MERGE_READ		811
#define SEQ_MERGE_WRITE		812
#define RAN_MERGE_WRITE		813
#define RAN_COLD_MERGE_READ	814
#define RAN_HOT_MERGE_READ	815
#define RAN_COLD_MERGE_WRITE	816
#define RAN_HOT_MERGE_WRITE	817
#define MAP_READ		818
#define MAP_WRITE		819

#define UPDATE_START_TIME	900
#define UPDATE_END_TIME		901
#define UPDATE_GC_START_TIME	902
#define UPDATE_GC_END_TIME	903

/* VSSIM Function Debug */
//#define MNT_DEBUG			// MONITOR Debugging
//#define WRITE_BUFFER_DEBUG		// WRITE BUFFER Debugging 
//#define FTL_CACHE_DEBUG		// CACHE Debugging
//#define SSD_THREAD_DEBUG
//#define IO_LATENCY_DEBUG		// ??

/* Workload */
//#define GET_FTL_WORKLOAD
//#define GET_FIRM_WORKLOAD
//#define FTL_GET_WRITE_WORKLOAD

/* FTL Debugging */
//#define FTL_DEBUG
//#define FTL_PERF_DEBUG
//#define FTL_IO_LATENCY                  //opened by hao

#define FTL_META_TRANS

/* FTL Perf Debug */
//#define PERF_DEBUG1
//#define PERF_DEBUG2
//#define PERF_DEBUG4

/* SSD Debugging */
//#define SSD_DEBUG
//#define SSD_SYNC

/* FIRMWARE Debugging */
//#define FIRM_IO_BUF_DEBUG
//#define GET_QUEUE_DEPTH

#endif // end of 'ifndef _COMMON_H_'
