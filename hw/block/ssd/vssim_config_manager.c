// File: vssim_config_manager.c
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"
#include <assert.h>
#include "god.h"

void INIT_STAT_COUNT(struct ssdstate *ssd)
{
	struct ssdconf *sc = &(ssd->ssdparams);

	ssd->stat_type=0;

	ssd->stat_time=0;
	ssd->stat_temp=0;

    ssd->stat_total_read_count=0;
    ssd->stat_total_write_count=0;
	ssd->stat_host_write_count=0;

	ssd->gc_count = 0;
    ssd->stat_gc_count=0;
    ssd->stat_erase_count=0;

    ssd->stat_gc_write_count=0;
	ssd->stat_remap_cnt=0;
	ssd->stat_commit_cnt=0;
	ssd->stat_reduced_write=0;

	ssd->stat_ppn_valid=0;
	ssd->stat_ppn_n21=0;
	ssd->stat_ppn_invalid=0;
	ssd->stat_ppn_free=sc->PAGE_MAPPING_ENTRY_NB;

	ssd->stat_lpn_valid=0;
	ssd->max_valid_array=0;

	ssd->stat_total_alloc_seg=0;
	ssd->stat_total_OOB_entry=0;
	ssd->stat_total_invalid_entry=0;
	ssd->stat_total_seg_bytes=0;
	ssd->stat_max_alloc_seg=0;
	ssd->stat_min_alloc_seg=0;

	ssd->stat_avg_write_delay=0;
	ssd->stat_write_req_print=0;
    ssd->stat_write_delay_print=0;
	ssd->stat_min_write_delay = 10000000000;
	ssd->stat_max_write_delay = 0;
	ssd->stat_write_size_print = 0;
	ssd->stat_avg_req_size = 0;
	ssd->stat_read_size_print = 0;

	ssd->stat_avg_GCRNVRAM_delay=0;
    ssd->stat_GCRNVRAM_print=0;
    ssd->stat_GCRNVRAM_delay_print=0;

    ssd->stat_avg_NVRAMGC_delay=0;
    ssd->stat_NVRAMGC_print=0;
    ssd->stat_NVRAMGC_delay_print=0;

	ssd->stat_trim_cmd=0;
    ssd->stat_trim_lpn=0;
    ssd->stat_trim_write_flash=0;

#ifdef STAT_COUNT
	FILE *fout = NULL;
	fout = fopen(STAT_OUTPUT_FILE, "w");
	if(fout == NULL)
	{
		printf("Error: Output file open error\n");
		getchar();
	}
	// fprintf(fout, "stat_type, total page read, total page write, host page write, gc count, \\
	// erase block, gc write, remap cnt, commit cnt, reduced (cp||dedup||gc), ppn valid, ppn n21, \\
	// ppn invalid, ppn free, lpn vallid, total alloc seg, total OOB entry, total invalid entry, total seg bytes, min alloc segs, max alloc segs, \\
	// write req between print, avg write delay, min write delay, max write delay, GCRNVRAM between print, avg GCRNVRAM delay, NVRAMGC between print, avg NVRAMGC delay, max ref cnt\n");
	
	fprintf(fout, "stat_type, total page write, host page write, gc count, reduced write, ppn valid, ppn invalid, lpn vallid, \\
	write req between print, avg write delay, write size (KB), read size (KB), max write delay, trim cmd, trim lpn, trim write flash\n");

	fclose(fout);
#endif //STAT_COUNT

    return;
}

void metadata_print(struct ssdstate *ssd, int32_t i, uint32_t tx_id, uint32_t flag, int64_t h_lpn)
{
	FILE *fout = NULL; 
	fout = fopen("/home/nvm/metadata.csv", "a");
	if(fout == NULL)
	{
		printf("Error: metadata file open error\n");
		getchar();
	}

	fprintf(fout, "%d, %u, %u, %ld\n", i, tx_id, flag, h_lpn);
	
	fflush(fout);
	fclose(fout);

	return;
}

void write_debug_print(struct ssdstate *ssd, int64_t lpn, int64_t new_ppn, int64_t old_ppn)
{
	FILE *fout = NULL; 
	fout = fopen("/home/nvm/write_debug.csv", "a");
	if(fout == NULL)
	{
		printf("Error: write debug file open error\n");
		getchar();
	}

	fprintf(fout, "%ld, %ld, %ld, \n", lpn, new_ppn, old_ppn);
	
	fflush(fout);
	fclose(fout);

	return;
}

void write_remap_print(struct ssdstate *ssd, int32_t i, int64_t dst_lpn, int64_t h_lpn)
{
	FILE *fout = NULL; 
	fout = fopen("/home/nvm/write_remap.csv", "a");
	if(fout == NULL)
	{
		printf("Error: write remap file open error\n");
		getchar();
	}

	fprintf(fout, "%d, %ld, %ld, \n", i, dst_lpn, h_lpn);
	
	fflush(fout);
	fclose(fout);

	return;
}

void decrease_debug_print(struct ssdstate *ssd, int64_t ppn, int64_t lpn, int32_t lpn_cnt)
{
	FILE *fout = NULL; 
	fout = fopen("/home/nvm/decrease_debug.csv", "a");
	if(fout == NULL)
	{
		printf("Error: decrease debug file open error\n");
		getchar();
	}

	fprintf(fout, "%ld, %ld, \n", ppn, lpn, lpn_cnt);
	
	fflush(fout);
	fclose(fout);

	return;
}

void increase_debug_print(struct ssdstate *ssd, int64_t ppn, int64_t lpn, int32_t lpn_cnt)
{
	FILE *fout = NULL; 
	fout = fopen("/home/nvm/increase_debug.csv", "a");
	if(fout == NULL)
	{
		printf("Error: increase debug file open error\n");
		getchar();
	}

	fprintf(fout, "%ld, %ld, %d\n", ppn, lpn, lpn_cnt);
	
	fflush(fout);
	fclose(fout);

	return;
}

void stat_print(struct ssdstate *ssd)
{
	FILE *fout = NULL; 
	fout = fopen(STAT_OUTPUT_FILE, "a");
	if(fout == NULL)
	{
		printf("Error: Output file open error\n");
		getchar();
	}

	// fprintf(fout, "%d, %u, %u, %u, %u, %u, %u, %u, %u, %u, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %lu, %u, %u, %u, %lu, %lu, %lu, %u, %lu, %u, %lu, %d\n", 
	// 	ssd->stat_type, 
	// 	ssd->stat_total_read_count, ssd->stat_total_write_count, ssd->stat_host_write_count,
	// 	ssd->stat_gc_count, ssd->stat_erase_count, ssd->stat_gc_write_count,
	// 	ssd->stat_remap_cnt, ssd->stat_commit_cnt, ssd->stat_reduced_write, 
	// 	ssd->stat_ppn_valid, ssd->stat_ppn_n21, ssd->stat_ppn_invalid, ssd->stat_ppn_free, 
	// 	ssd->stat_lpn_valid, ssd->stat_total_alloc_seg,	ssd->stat_total_OOB_entry, 
	// 	ssd->stat_total_invalid_entry, ssd->stat_total_seg_bytes, ssd->stat_min_alloc_seg, ssd->stat_max_alloc_seg, ssd->stat_write_req_print, ssd->stat_avg_write_delay, ssd->stat_min_write_delay, ssd->stat_max_write_delay,
	// 	ssd->stat_GCRNVRAM_print, ssd->stat_avg_GCRNVRAM_delay, ssd->stat_NVRAMGC_print, ssd->stat_avg_NVRAMGC_delay, ssd->max_valid_array);

	fprintf(fout, "%d, %u, %u, %u, %u, %lu, %lu, %lu, %u, %lu, %lu, %lu, %lu, %u, %u, %u\n",
	ssd->stat_type, 
	ssd->stat_total_write_count, ssd->stat_host_write_count, ssd->stat_gc_count, ssd->stat_reduced_write,
	ssd->stat_ppn_valid, ssd->stat_ppn_invalid, ssd->stat_lpn_valid,
	ssd->stat_write_req_print, ssd->stat_avg_write_delay, ssd->stat_write_size_print/2, ssd->stat_read_size_print/2, ssd->stat_max_write_delay, 
	ssd->stat_trim_cmd, ssd->stat_trim_lpn, ssd->stat_trim_write_flash);

	fflush(fout);
	fclose(fout);
	
	if(ssd->stat_type == 3)
	{
		ssd->stat_avg_write_delay=0;
		ssd->stat_write_req_print=0;
		ssd->stat_write_delay_print=0;
		ssd->stat_min_write_delay = 10000000000;
		ssd->stat_max_write_delay = 0;
		ssd->stat_write_size_print=0;
		ssd->stat_avg_req_size=0;
		ssd->stat_read_size_print=0;

		ssd->stat_avg_GCRNVRAM_delay=0;
		ssd->stat_GCRNVRAM_print=0;
		ssd->stat_GCRNVRAM_delay_print=0;

		ssd->stat_avg_NVRAMGC_delay=0;
		ssd->stat_NVRAMGC_print=0;
		ssd->stat_NVRAMGC_delay_print=0;

		ssd->stat_trim_cmd=0;
    	ssd->stat_trim_lpn=0;
    	ssd->stat_trim_write_flash=0;
	}

	ssd->stat_type=0;

	return;
}

void INIT_SSD_CONFIG(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
	FILE* pfData;

	unsigned int p1, q;                      //add by hao

	pfData = fopen(ssd->conffile, "r");
	
	char* szCommand = NULL;
	
	szCommand = (char*)malloc(1024);
	memset(szCommand, 0x00, 1024);

	sc->sos = PI_BYTES_NVME;                            //add by hao
	sc->max_page = 0;
	ssd->block_cnt = 0;
	ssd->data_buffer_counter = 0;
	ssd->empty_head_block_index = 0;
	ssd->current_filter = 0;
	ssd->current_decay_filter = -1;

	for (q = 0; q < FILTER_NUMBER; q++)
		for (p1 = 0; p1 < FILTER_SIZE_BYTES; p1++)
		ssd->filter[q][p1] = 0;

//初始化filter_weight数组
	for (q = 0; q < FILTER_NUMBER; q++)
		ssd->filter_weight[q] = 0;

	if(pfData!=NULL)
	{
		while(fscanf(pfData, "%s", szCommand)!=EOF)
		{
			if(strcmp(szCommand, "PAGE_SIZE") == 0)
			{
				fscanf(pfData, "%d", &sc->PAGE_SIZE);
			}
			else if(strcmp(szCommand, "PAGE_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->PAGE_NB);
			}
			else if(strcmp(szCommand, "SECTOR_SIZE") == 0)
			{
				fscanf(pfData, "%d", &sc->SECTOR_SIZE);
			}	
			else if(strcmp(szCommand, "FLASH_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->FLASH_NB);
			}	
			else if(strcmp(szCommand, "BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->BLOCK_NB);
			}					
			else if(strcmp(szCommand, "PLANES_PER_FLASH") == 0)
			{
				fscanf(pfData, "%d", &sc->PLANES_PER_FLASH);
			}
			else if(strcmp(szCommand, "REG_WRITE_DELAY") == 0)
			{
				fscanf(pfData, "%d", &sc->REG_WRITE_DELAY);
			}	
			else if(strcmp(szCommand, "CELL_PROGRAM_DELAY") == 0)
			{
				fscanf(pfData, "%d", &sc->CELL_PROGRAM_DELAY);
			}
			else if(strcmp(szCommand, "REG_READ_DELAY") == 0)
			{
				fscanf(pfData, "%d", &sc->REG_READ_DELAY);
			}
			else if(strcmp(szCommand, "CELL_READ_DELAY") == 0)
			{
				fscanf(pfData, "%d", &sc->CELL_READ_DELAY);
			}
			else if(strcmp(szCommand, "BLOCK_ERASE_DELAY") == 0)
			{
				fscanf(pfData, "%d", &sc->BLOCK_ERASE_DELAY);
			}
			else if(strcmp(szCommand, "CHANNEL_SWITCH_DELAY_R") == 0)
			{
				fscanf(pfData, "%d", &sc->CHANNEL_SWITCH_DELAY_R);
			}
			else if(strcmp(szCommand, "CHANNEL_SWITCH_DELAY_W") == 0)
			{
				fscanf(pfData, "%d", &sc->CHANNEL_SWITCH_DELAY_W);
			}
			else if(strcmp(szCommand, "DSM_TRIM_ENABLE") == 0)
			{
				fscanf(pfData, "%d", &sc->DSM_TRIM_ENABLE);
			}
			else if(strcmp(szCommand, "IO_PARALLELISM") == 0)
			{
				fscanf(pfData, "%d", &sc->IO_PARALLELISM);
			}
			else if(strcmp(szCommand, "CHANNEL_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->CHANNEL_NB);
			}
			else if(strcmp(szCommand, "OVP") == 0)
			{
				fscanf(pfData, "%d", &sc->OVP);
			}
            else if (strcmp(szCommand, "GC_MODE") == 0)
            {
                fscanf(pfData, "%d", &sc->GC_MODE);
            }
#if defined FTL_MAP_CACHE || defined Polymorphic_FTL
			else if(strcmp(szCommand, "CACHE_IDX_SIZE") == 0)
			{
				fscanf(pfData, "%d", &sc->CACHE_IDX_SIZE);
			}
#endif
#ifdef FIRM_IO_BUFFER
			else if(strcmp(szCommand, "WRITE_BUFFER_FRAME_NB") == 0)
			{
				fscanf(pfData, "%u", &sc->WRITE_BUFFER_FRAME_NB);
			}
			else if(strcmp(szCommand, "READ_BUFFER_FRAME_NB") == 0)
			{
				fscanf(pfData, "%u", &sc->READ_BUFFER_FRAME_NB);
			}
#endif
#ifdef HOST_QUEUE
			else if(strcmp(szCommand, "HOST_QUEUE_ENTRY_NB") == 0)
			{
				fscanf(pfData, "%u", &sc->HOST_QUEUE_ENTRY_NB);
			}
#endif
#if defined FAST_FTL || defined LAST_FTL
			else if(strcmp(szCommand, "LOG_RAND_BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->LOG_RAND_BLOCK_NB);
			}	
			else if(strcmp(szCommand, "LOG_SEQ_BLOCK_NB") == 0)
			{
				fscanf(pfData, "%d", &sc->LOG_SEQ_BLOCK_NB);
			}	
#endif
			memset(szCommand, 0x00, 1024);
		}	
		fclose(pfData);

	} else {
        printf("Error: Cannot find configuration file for [%s]\n", ssd->ssdname);
        exit(EXIT_FAILURE);
    }

	/* Exception Handler */
	if(sc->FLASH_NB < sc->CHANNEL_NB){
		printf("ERROR[%s] Wrong CHANNEL_NB %d\n",__FUNCTION__, sc->CHANNEL_NB);
		return;
	}
	if(sc->PLANES_PER_FLASH != 1 && sc->PLANES_PER_FLASH % 2 != 0){
		printf("ERROR[%s] Wrong PLANAES_PER_FLASH %d\n", __FUNCTION__, sc->PLANES_PER_FLASH);
		return;
	}
#ifdef FIRM_IO_BUFFER
	if(sc->WRITE_BUFFER_FRAME_NB == 0 || sc->READ_BUFFER_FRAME_NB == 0){
		printf("ERROR[%s] Wrong parameter for SSD_IO_BUFFER",__FUNCTION__);
		return;
	}
#endif

	/* SSD Configuration */
	sc->SECTORS_PER_PAGE = sc->PAGE_SIZE / sc->SECTOR_SIZE;
	sc->PAGES_PER_FLASH = sc->PAGE_NB * sc->BLOCK_NB;
	sc->SECTOR_NB = (int64_t)sc->SECTORS_PER_PAGE * (int64_t)sc->PAGE_NB * (int64_t)sc->BLOCK_NB * (int64_t)sc->FLASH_NB;
    printf("VSSIM: sector_nb = %" PRId64 "\n", sc->SECTOR_NB);
#ifndef Polymorphic_FTL
	sc->WAY_NB = sc->FLASH_NB / sc->CHANNEL_NB;
#endif

	/* Mapping Table */
	sc->BLOCK_MAPPING_ENTRY_NB = (int64_t)sc->BLOCK_NB * (int64_t)sc->FLASH_NB;
	sc->PAGES_IN_SSD = (int64_t)sc->PAGE_NB * (int64_t)sc->BLOCK_NB * (int64_t)sc->FLASH_NB;

#ifdef PAGE_MAP
	sc->PAGE_MAPPING_ENTRY_NB = (int64_t)sc->PAGE_NB * (int64_t)sc->BLOCK_NB * (int64_t)sc->FLASH_NB;
#endif

#if defined PAGE_MAP || defined BLOCK_MAP
	sc->EACH_EMPTY_TABLE_ENTRY_NB = (int64_t)sc->BLOCK_NB / (int64_t)sc->PLANES_PER_FLASH;
	sc->EMPTY_TABLE_ENTRY_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;
	sc->VICTIM_TABLE_ENTRY_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;

	sc->DATA_BLOCK_NB = sc->BLOCK_NB;
#endif

#if defined FAST_FTL || defined LAST_FTL
	sc->DATA_BLOCK_NB = sc->BLOCK_NB - sc->LOG_RAND_BLOCK_NB - sc->LOG_SEQ_BLOCK_NB;
	sc->DATA_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * (int64_t)sc->DATA_BLOCK_NB;

	sc->EACH_EMPTY_BLOCK_ENTRY_NB = (int64_t)sc->BLOCK_NB / (int64_t)sc->PLANES_PER_FLASH;
	sc->EMPTY_BLOCK_TABLE_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;
#endif

#ifdef DA_MAP
	if(sc->BM_START_SECTOR_NB < 0 || sc->BM_START_SECTOR_NB >= sc->SECTOR_NB){
		printf("ERROR[%s] BM_START_SECTOR_NB %d \n", __FUNCTION__, sc->BM_START_SECTOR_NB);
	}

	sc->DA_PAGE_MAPPING_ENTRY_NB = CALC_DA_PM_ENTRY_NB(); 
	sc->DA_BLOCK_MAPPING_ENTRY_NB = CALC_DA_BM_ENTRY_NB();
	sc->EACH_EMPTY_TABLE_ENTRY_NB = (int64_t)sc->BLOCK_NB / (int64_t)sc->PLANES_PER_FLASH;
	sc->EMPTY_TABLE_ENTRY_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;
	sc->VICTIM_TABLE_ENTRY_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;
#endif

	/* FAST Performance Test */
#ifdef FAST_FTL
	sc->SEQ_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * (int64_t)sc->LOG_SEQ_BLOCK_NB;
	sc->RAN_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * (int64_t)sc->LOG_RAND_BLOCK_NB;
	sc->RAN_LOG_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * (int64_t)sc->LOG_RAND_BLOCK_NB * (int64_t)sc->PAGE_NB;

	// PARAL_DEGREE = 4;
	sc->PARAL_DEGREE = sc->RAN_MAPPING_ENTRY_NB;
	if(sc->PARAL_DEGREE > sc->RAN_MAPPING_ENTRY_NB){
		printf("[INIT_SSD_CONFIG] ERROR PARAL_DEGREE \n");
		return;
	}
	sc->PARAL_COUNT = sc->PARAL_DEGREE * sc->PAGE_NB;
#endif

#ifdef LAST_FTL
	sc->SEQ_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * (int64_t)sc->LOG_SEQ_BLOCK_NB;
	sc->RAN_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * ((int64_t)sc->LOG_RAND_BLOCK_NB/2);
	sc->RAN_LOG_MAPPING_ENTRY_NB = (int64_t)sc->FLASH_NB * ((int64_t)sc->LOG_RAND_BLOCK_NB/2) * (int64_t)sc->PAGE_NB;

	sc->SEQ_THRESHOLD = sc->SECTORS_PER_PAGE * 4;
	sc->HOT_PAGE_NB_THRESHOLD = sc->PAGE_NB;

	sc->PARAL_DEGREE = sc->RAN_MAPPING_ENTRY_NB;
	if(sc->PARAL_DEGREE > sc->RAN_MAPPING_ENTRY_NB){
		printf("[INIT_SSD_CONFIG] ERROR PARAL_DEGREE \n");
		return;
	}
	sc->PARAL_COUNT = sc->PARAL_DEGREE * sc->PAGE_NB;
#endif

	/* Garbage Collection */
#if defined PAGE_MAP || defined BLOCK_MAP || defined DA_MAP
	sc->GC_THRESHOLD = 0.9; // 0.7 for 70%, 0.9 for 90%
	sc->GC_THRESHOLD_HARD = 0.95;
	sc->GC_THRESHOLD_BLOCK_NB = (int)((1-sc->GC_THRESHOLD) * (double)sc->BLOCK_MAPPING_ENTRY_NB);
	sc->GC_THRESHOLD_BLOCK_NB_HARD = (int)((1-sc->GC_THRESHOLD_HARD) * (double)sc->BLOCK_MAPPING_ENTRY_NB);
	sc->GC_THRESHOLD_BLOCK_NB_EACH = (int)((1-sc->GC_THRESHOLD) * (double)sc->EACH_EMPTY_TABLE_ENTRY_NB);
	if(sc->OVP != 0){
		sc->GC_VICTIM_NB = sc->FLASH_NB * sc->BLOCK_NB * sc->OVP / 100 / 2;
	}
	else{
		sc->GC_VICTIM_NB = 1;
	}
#endif

    /* Coperd: allocate GC_SLOT structure here */
    int num_gc_slots = 0;
    if (sc->GC_MODE == WHOLE_BLOCKING) {
        num_gc_slots = 1;
    } else if (sc->GC_MODE == CHANNEL_BLOCKING) {
        num_gc_slots = sc->CHANNEL_NB;
    } else if (sc->GC_MODE == CHIP_BLOCKING) {
        num_gc_slots = sc->FLASH_NB * sc->PLANES_PER_FLASH;
    } else {
        printf("Unsupported GC MODE: %d!\n", sc->GC_MODE);
        exit(EXIT_FAILURE);
    }

    assert(num_gc_slots != 0);
    ssd->gc_slot = calloc(num_gc_slots, sizeof(int64_t));

	/* Map Cache */
#ifdef FTL_MAP_CACHE
	sc->MAP_ENTRY_SIZE = sizeof(int64_t);
	sc->MAP_ENTRIES_PER_PAGE = sc->PAGE_SIZE / sc->MAP_ENTRY_SIZE;
	sc->MAP_ENTRY_NB = sc->PAGE_MAPPING_ENTRY_NB / sc->MAP_ENTRIES_PER_PAGE;	
#endif

	/* Polymorphic FTL */
#ifdef Polymorphic_FTL
	sc->WAY_NB = 1;

	sc->PHY_SPARE_SIZE = 436;
	sc->NR_PHY_BLOCKS = (int64_t)sc->FLASH_NB * (int64_t)sc->WAY_NB * (int64_t)sc->BLOCK_NB;
	sc->NR_PHY_PAGES = sc->NR_PHY_BLOCKS * (int64_t)sc->PAGE_NB;
	sc->NR_PHY_SECTORS = sc->NR_PHY_PAGES * (int64_t)sc->SECTORS_PER_PAGE;

	sc->NR_RESERVED_PHY_SUPER_BLOCKS = 4;
	sc->NR_RESERVED_PHY_BLOCKS = sc->FLASH_NB * sc->WAY_NB * sc->NR_RESERVED_PHY_SUPER_BLOCKS;
	sc->NR_RESERVED_PHY_PAGES = sc->NR_RESERVED_PHY_BLOCKS * sc->PAGE_NB;
#endif

    ssd->chnl_next_avail_time = (int64_t *)malloc(sizeof(int64_t) * sc->CHANNEL_NB);
    memset(ssd->chnl_next_avail_time, 0, sizeof(int64_t) * sc->CHANNEL_NB);
    ssd->chip_next_avail_time = (int64_t *)malloc(sizeof(int64_t) * sc->FLASH_NB);
    memset(ssd->chip_next_avail_time, 0, sizeof(int64_t) * sc->FLASH_NB);

	free(szCommand);
}

#ifdef DA_MAP
int64_t CALC_DA_PM_ENTRY_NB(void)
{
	int64_t ret_page_nb = (int64_t)sc->BM_START_SECTOR_NB / sc->SECTORS_PER_PAGE;
	if((sc->BM_START_SECTOR_NB % sc->SECTORS_PER_PAGE) != 0)
		ret_page_nb += 1;

	int64_t block_nb = ret_page_nb / sc->PAGE_NB;
	if((ret_page_nb % sc->PAGE_NB) != 0)
		block_nb += 1;

	ret_page_nb = block_nb * sc->PAGE_NB;
	bm_start_sect_nb = ret_page_nb * sc->SECTORS_PER_PAGE;

	return ret_page_nb;
}

int64_t CALC_DA_BM_ENTRY_NB(void)
{
	int64_t total_page_nb = (int64_t)sc->PAGE_NB * sc->BLOCK_NB * sc->FLASH_NB;
	int64_t ret_block_nb = ((int64_t)total_page_nb - sc->DA_PAGE_MAPPING_ENTRY_NB)/(int64_t)sc->PAGE_NB;

	int64_t temp_total_page_nb = ret_block_nb * sc->PAGE_NB + sc->DA_PAGE_MAPPING_ENTRY_NB;
	if(temp_total_page_nb != total_page_nb){
		printf("ERROR[%s] %ld != %ld\n", __FUNCTION__, total_page_nb, temp_total_page_nb);
	}

	return ret_block_nb;
}
#endif
