// File: vssim_config_manager.c
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"
#include <assert.h>
#include "god.h"

#ifdef WS_COUNT
void INIT_WS_COUNT(struct ssdstate *ssd)
{
	struct ssdconf *sc = &(ssd->ssdparams);
	ssd->is_GC=0;
    ssd->ws_gc_count=0;
    ssd->ws_erase_count=0;
    
    ssd->ws_total_read_count=0;
    ssd->ws_total_write_count=0;
    

    ssd->ws_gc_read_count=0;
    ssd->ws_gc_write_count=0;

	ssd->ws_time=0;
	ssd->ws_temp=0;

	ssd->wql_temp = 0;
	ssd->wql_time = 0;

	ssd->ws_old_new_e=0;   //old lpn == new lpn
    ssd->ws_old_new_ne=0;  //old lpn != new lpn
    ssd->ws_user_page_write_between_trim=0;
    ssd->ws_gc_page_write_between_trim=0;
	ssd->ws_gc_old_lpn_count=0;

	ssd->ws_ppa_valid=0;
	ssd->ws_ppa_invalid=0;
	ssd->ws_ppa_pre_free=0;
	ssd->ws_ppa_free=sc->BLOCK_MAPPING_ENTRY_NB*sc->PAGE_NB;
    ssd->ws_newpage=0;

	FILE *fout = NULL; 
	fout = fopen(OUTPUT_FILENAME, "w");
	if(fout == NULL)
	{
		printf("Error: Output file open error\n");
		getchar();
	}
	fprintf(fout, "is_gc, GC迁移的页总数量, GC擦除的块数, GC迁移的old页, GC写入数据量(页), 用户写入数据量(页),  write_new_page ,old_lpn==new_lpn, old_lpn!=new_lpn, valid_page, \\
invalid_page, prefree_page, freepage, \n");
	fclose(fout);
    return;
}

void wql_print(struct ssdstate *ssd)
{
	struct ssdconf *sc = &(ssd->ssdparams);
    int FLASH_NB = sc->FLASH_NB;
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;
    int VICTIM_TABLE_ENTRY_NB = sc->VICTIM_TABLE_ENTRY_NB;
	int SB_BLK_NB = sc->FLASH_NB * sc->PLANES_PER_FLASH;

    victim_block_root *victim_block_list = (victim_block_root *)ssd->victim_block_list;

	int i, j;
	int entry_nb = 0;
	
	int entry_nb1=0;
	int k, off = 0;
	unsigned int first_block_nb;

	FILE *fout = NULL; 

	victim_block_root* curr_v_b_root;
	victim_block_entry* curr_v_b_entry;
	victim_block_entry* victim_block[SB_BLK_NB]; //  WQL:assume plane_per_flash = 1

	block_state_entry* b_s_entry;
	int curr_valid_page_nb = 0, curr_prefree_page_nb = 0;
	int min_valid_page_nb = PAGE_NB * FLASH_NB * sc->PLANES_PER_FLASH;
	if(ssd->total_victim_block_nb == 0)
	{
		printf("ERROR[%s] There is no victim block--1\n", __FUNCTION__);
		return FAIL;
	}

	curr_v_b_root = victim_block_list;

	if(curr_v_b_root->victim_block_nb != 0)
	{	
		entry_nb = curr_v_b_root->victim_block_nb;
		curr_v_b_entry = curr_v_b_root->head;
	}
	else
	{
		entry_nb = 0;
		printf("ERROR[%s] There is no victim superblock--2\n", __FUNCTION__);
		return FAIL;
	}

	curr_v_b_root = victim_block_list;

	fout = fopen(SB_PRE_FILENAME, "w");
	if(fout == NULL)
	{
		printf("Error: Output file open error\n");
		getchar();
	}
	fprintf(fout, "SB count, prefree page nb in SB, total prefree page nb,\n");

	for(i=0;i<entry_nb;i++)
	{
		curr_valid_page_nb = 0; 
		curr_prefree_page_nb = 0;
		
		for(j=0;j<VICTIM_TABLE_ENTRY_NB;j++)
		{
			curr_v_b_root = victim_block_list + j;
			if(i==0)
			{
				entry_nb1 = curr_v_b_root->victim_block_nb;
				if(entry_nb1!=entry_nb)
				{
					printf("ERROR[%s] Victim block count error\n", __FUNCTION__);
					return FAIL;
				}
			}

			curr_v_b_entry = curr_v_b_root->head;
			for(k=0;k<i;k++)
			{
				curr_v_b_entry = curr_v_b_entry->next;
			}

			if(j==0)
			{
				first_block_nb = curr_v_b_entry->phy_block_nb;
			}

			if(curr_v_b_entry->phy_block_nb != first_block_nb)
			{
				printf("ERROR[%s] Victim block offset error\n", __FUNCTION__);
				return FAIL;
			}
			
			b_s_entry = GET_BLOCK_STATE_ENTRY(ssd, curr_v_b_entry->phy_flash_nb, curr_v_b_entry->phy_block_nb);
			curr_valid_page_nb += b_s_entry->valid_page_nb;
			curr_prefree_page_nb += b_s_entry->prefree_page_nb;
		}
		fprintf(fout, "%d, %d, %d,\n", i, curr_prefree_page_nb, ssd->ws_ppa_pre_free);
	}
	

	fflush(fout);
	fclose(fout);
	return;
}

void ws_print(struct ssdstate *ssd)
{
	//FILE *OUTPOU_FILE 
    /*printf("\n");
    printf("ssd->ws_total_read_count = %d\n", ssd->ws_total_read_count);
    printf("ssd->ws_total_write_count = %d\n", ssd->ws_total_write_count);
    printf("ssd->ws_gc_read_count = %d\n", ssd->ws_gc_read_count);
    printf("ssd->ws_gc_write_count = %d\n", ssd->ws_gc_write_count);
    printf("ssd->ws_erase_count = %d\n", ssd->ws_erase_count);
    printf("\n");*/
	FILE *fout = NULL; 
	fout = fopen(OUTPUT_FILENAME, "a");
	if(fout == NULL)
	{
		printf("Error: Output file open error\n");
		getchar();
	}

	fprintf(fout, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d,\n", ssd->is_GC, ssd->ws_gc_write_count,
		ssd->ws_erase_count, 
		ssd->ws_gc_old_lpn_count, ssd->ws_gc_page_write_between_trim, ssd->ws_user_page_write_between_trim,
		ssd->ws_newpage, ssd->ws_old_new_e, ssd->ws_old_new_ne, 
		ssd->ws_ppa_valid, ssd->ws_ppa_invalid, ssd->ws_ppa_pre_free, ssd->ws_ppa_free);
	
	fflush(fout);
	//fprintf(fout, "GC迁移的页总数量, GC迁移的old页, 用户写入数据量(页), GC写入数据量(页), old lpn==new lpn, old lpn!=new lpn,\n");
	fclose(fout);
	
	ssd->is_GC=0;
	ssd->ws_gc_write_count=0;
	ssd->ws_erase_count=0;
	 
	ssd->ws_gc_old_lpn_count=0;
	ssd->ws_gc_page_write_between_trim=0;
	ssd->ws_user_page_write_between_trim=0;
	
	ssd->ws_newpage=0;
	ssd->ws_old_new_e=0;
	ssd->ws_old_new_ne=0;

	return;
}

// void ws_print_lba(struct ssdstate *ssd)
// {
// 	FILE *fout = NULL; 
// 	fout = fopen(OUTPUT_FILENAME, "a");
// 	if(fout == NULL)
// 	{
// 		printf("Error: Output file open error\n");
// 		getchar();
// 	}
// 	int ws_ppa_valid=0;
//     int ws_ppa_invalid=0;
//     int ws_ppa_pre_free=0;

// 	struct ssdconf *sc = &(ssd->ssdparams);
// 	int FLASH_NB = sc->FLASH_NB;
//     int BLOCK_NB = sc->BLOCK_NB;
//     int PAGE_NB = sc->PAGE_NB;

// 	block_state_entry* b_s_entry;
// 	char* valid_array;

// 	int i, j, k;
// 	for(i=0; i<FLASH_NB; i++)
// 	{
// 		for(j=0; j<BLOCK_NB; j++)
// 		{
// 			b_s_entry = GET_BLOCK_STATE_ENTRY(ssd, i, j);
// 			valid_array = b_s_entry->valid_array;
// 			for(k=0; k<PAGE_NB; k++)
// 			{
// 				if(valid_array[k]=='V')
// 					ws_ppa_valid++;
// 				else if(valid_array[k]=='I')
// 					ws_ppa_invalid++;
// 				else if(valid_array[k]=='P')
// 					ws_ppa_pre_free++;
// 			}
// 		}
// 	}

	
// 	fflush(fout);
// 	fclose(fout);
// 	return;
// }
#endif //WS_COUNT

void INIT_SSD_CONFIG(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
	FILE* pfData;

	unsigned int p1, q;                      //add by hao



	pfData = fopen(ssd->conffile, "r");
	
	char* szCommand = NULL;
	
	szCommand = (char*)malloc(1024);
	memset(szCommand, 0x00, 1024);

	sc->sos = 32;                            //add by hao
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
