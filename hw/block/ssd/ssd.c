// File: ssd.c
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "ssd.h"
#include "common.h"
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include "ftl_inverse_mapping_manager.h"
#include "vssim_config_manager.h"
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef GET_WORKLOAD
#include <time.h>
#include <sys/time.h>
#endif

//extern long mygc_cnt;
//extern long mycopy_page_nb;

int ssd_num = 1;

char *get_ssd_name()
{
    return "nvme ssd";
}

void do_warmup(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int GC_THRESHOLD_BLOCK_NB = sc->GC_THRESHOLD_BLOCK_NB;
	struct request_f2fs *request1;           //add by hao

    ssd->in_warmup_stage = true;
    char *tfname = "./data/warmup.trace";

    FILE *tfp = fopen(tfname, "r");
    if (tfp == NULL) {
        printf("CANNOT open trace file [%s], skipping warmup\n", tfname);
        return;
    }

    int64_t w_sector_num;
    int w_length, w_type;

    int t_nb_sects = 0;
    int t_ios = 0;
    int ntraverses = 0;
    int64_t i = 0;

    int64_t ssd_sz = 24 * 1024 * 1024 * 2 - 50 * 1024 * 2;

    printf("=======[%s] WARMUP from %s=======\n", "nvme ssd", tfname);
    while (1) {
        int sr = fscanf(tfp, "%*f%*d%" PRId64 "%d%d\n", &w_sector_num, 
                &w_length, &w_type);
        if ((sr == EOF) && (ntraverses <= 0)) {
            ntraverses++;
            fseek(tfp, 0, SEEK_SET);
        } else if (sr == EOF) {
            break;
        }
        if (w_type == 0)  {         /* skip writes */
            //continue;

			request1->lpns_info[i].f2fs_ino = 0;
			request1->lpns_info[i].f2fs_off = 0;
			request1->lpns_info[i].f2fs_current_lpn = 0;
			request1->lpns_info[i].f2fs_temp = 0;
			request1->lpns_info[i].f2fs_type = 0;
			request1->lpns_info[i].f2fs_old_lpn = 0;

            request1->length =  w_length;

			request1->sector_nb = w_sector_num % ssd_sz;
            SSD_WRITE(ssd, request1);
            i++;
        }
        t_nb_sects += w_length;
        t_ios++;
        //mylog("write: (%"PRId64", %d)\n", w_sector_num, w_length);
    }
    fclose(tfp);
    printf("========[%s] SSD WARMUP ENDS %"PRId64"/%d blocks, %d I/Os,"
            "%d MB========\n", get_ssd_name(), ssd->total_empty_block_nb, GC_THRESHOLD_BLOCK_NB, t_ios, t_nb_sects*512/1024/1024);

    ssd->in_warmup_stage = false;
}

void do_rand_warmup(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int GC_THRESHOLD_BLOCK_NB = sc->GC_THRESHOLD_BLOCK_NB;
    double GC_THRESHOLD = sc->GC_THRESHOLD;

	struct request_f2fs *request1;           //add by hao

    int i;
    int nios = 10;
    int *io = (int *)malloc(sizeof(int)*nios);
    for (i = 0; i < nios; i++) {
        io[i] = 1 << i;
    }
    int64_t ssd_sz_in_sects = sc->PAGES_IN_SSD * sc->PAGE_SIZE / 512 - 8192;
    //printf("ssd size: %ld\n", ssd_sz_in_sects);
    srand(time(NULL));
    int io_sz;          // in sectors
    int64_t io_oft;     // in sectors
    int64_t written_sz_in_sects = 0;

    /* Coperd: 0 -> read from warmup file, 1 -> generate warmup file */
    int warmup_mode = 0;


    if (warmup_mode == 0) {
        printf("=======[%s] Random WARMUP Begins=======\n", ssd->ssdname);
        FILE *fp = fopen(ssd->warmupfile, "r");
        if (fp == NULL) {
            fprintf(stderr, "CANNOT open warmup file [%s], skipping it ..\n", ssd->warmupfile);
            return;
        }

        ssd->in_warmup_stage = 1;
        i = 0;
        while(fscanf(fp, "%"PRId64"%d\n", &io_oft, &io_sz) != EOF) {
			request1->lpns_info[i].f2fs_ino = 0;
			request1->lpns_info[i].f2fs_off = 0;
			request1->lpns_info[i].f2fs_current_lpn = 0;
			request1->lpns_info[i].f2fs_temp = 0;
			request1->lpns_info[i].f2fs_type = 0;
			request1->lpns_info[i].f2fs_old_lpn = 0;


			request1->length = io_sz;

			request1->sector_nb = io_oft;
            SSD_WRITE(ssd, request1);
            written_sz_in_sects += io_sz;
            i++;
        }
        ssd->in_warmup_stage = 0;
        printf("========[%s] WARMUP ENDS %"PRId64"/%d blocks,"
                "%" PRId64 " MB========\n", ssd->ssdname, ssd->total_empty_block_nb, 
                GC_THRESHOLD_BLOCK_NB, written_sz_in_sects*512/1024/1024);

    } else {
        printf("=======[%s] Generating WARMUP File Begins=======\n", ssd->ssdname);
        FILE *fp = fopen(ssd->warmupfile, "w");
        if (fp == NULL) {
            fprintf(stderr, "CANNOT open warmup file [%s]\n", ssd->warmupfile);
            exit(EXIT_FAILURE);
        }
        i = 0;
        while (written_sz_in_sects <= ssd_sz_in_sects * (sc->GC_THRESHOLD - 0.02)) {
			io_sz = io[rand() % nios] * 2; 
			io_oft = (rand() % (ssd_sz_in_sects / 4)) * 4;

			request1->lpns_info[i].f2fs_ino = 0;
			request1->lpns_info[i].f2fs_off = 0;
			request1->lpns_info[i].f2fs_current_lpn = 0;
			request1->lpns_info[i].f2fs_temp = 0;
			request1->lpns_info[i].f2fs_type = 0;
			request1->lpns_info[i].f2fs_old_lpn = 0;
			
			
			request1->length = io_sz;
			
			request1->sector_nb = io_oft;
			SSD_WRITE(ssd, request1);
            i++;
            //printf("%"PRId64", %d\n", io_oft, io_sz);
            written_sz_in_sects += io_sz;

            fprintf(fp, "%ld\t%d\n", io_oft, io_sz);
        }
        printf("========[%s] Generating WARMUP File ENDS %"PRId64"/%d blocks,"
                "%" PRId64 " MB========\n", ssd->ssdname, ssd->total_empty_block_nb, 
                GC_THRESHOLD_BLOCK_NB, written_sz_in_sects*512/1024/1024);
    }


    if (warmup_mode == 1) {
        if (ssd_num == 5)
            exit(EXIT_FAILURE);
    }
}

//sector_entry* PARSE_SECTOR_LIST(trim_data, length);
void SSD_INIT(struct ssdstate *ssd)
{
    memset(ssd, 0, sizeof(struct ssdstate));

    /* Coperd: ssdstate structure initialization */
    strcpy(ssd->ssdname, "vssd");
    char ftmp[64] = {'\0'};
    sprintf(ftmp, "%d", ssd_num++);
    strcat(ssd->ssdname, ftmp);
    strcpy(ssd->conffile, ssd->ssdname);
    strcat(ssd->conffile, ".conf");
    strcpy(ssd->statfile, ssd->ssdname);
    strcat(ssd->statfile, ".csv");
    strcpy(ssd->warmupfile, ssd->ssdname);
    strcat(ssd->warmupfile, ".trace");

    printf("[%s] is up\n", ssd->ssdname);
    sleep(1);

    ssd->io_request_start = NULL;
    ssd->io_request_end = NULL;
    ssd->io_request_nb = 0;


	FTL_INIT(ssd);

    ssd->statfp = fopen(ssd->statfile, "w+");
    if (ssd->statfp == NULL) {
        fprintf(stderr, "Error creating stat files!!!\n");
        exit(EXIT_FAILURE);
    }
    setvbuf(ssd->statfp, NULL, _IONBF, 0);
    fprintf(ssd->statfp, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n", 
            "#br","#r", "rd-sz", "#bw", "#w", "wr-sz", "#nvme-rw", 
            "lat-nvme-rw", "lat-ssd-rd", "lat-ssd-wr",
            "#gc", "cb-pg", 
            "lat-each-gc", "lat-svb", "lat-cp", "lat-up");
    fflush(ssd->statfp);

    /* Coperd: do warmup immediately after SSD structures are initialized */
    do_rand_warmup(ssd);
}

void SSD_TERM(struct ssdstate *ssd)
{	
	FTL_TERM(ssd);
}

//long last_time = 0;

int64_t SSD_WRITE(struct ssdstate *ssd, struct request_f2fs *request1)
{
#if 0
    int64_t curtime = get_usec();
    if (curtime - last_time >= 100000) {
        /* Coperd: print statistics */
        printf("curtime=%ld, GC=%ld, CP_PAGES=%ld\n", curtime, mygc_cnt, mycopy_page_nb);
        last_time = curtime;
        mycopy_page_nb = 0;
    }
#endif
    
	unsigned int length;
	int64_t sector_nb;


	length = request1->length; 
	sector_nb = request1->sector_nb;


#if defined SSD_THREAD	

	pthread_mutex_lock(&cq_lock);
	DEQUEUE_COMPLETED_READ();
	pthread_mutex_unlock(&cq_lock);

	if(EVENT_QUEUE_IS_FULL(WRITE, length)){
		w_queue_full = 1;
		while(w_queue_full == 1){
			pthread_cond_signal(&eq_ready);
		}
	}
	
	pthread_mutex_lock(&eq_lock);
	ENQUEUE_IO(WRITE, sector_nb, length);
	pthread_mutex_unlock(&eq_lock);

    #ifdef SSD_THREAD_MODE_1
	pthread_cond_signal(&eq_ready);
    #endif

#elif defined FIRM_IO_BUFFER
	DEQUEUE_COMPLETED_READ();
	if(EVENT_QUEUE_IS_FULL(WRITE, length)){
		SECURE_WRITE_BUFFER();
	}
	ENQUEUE_IO(WRITE, sector_nb, length);
#else
	return FTL_WRITE(ssd, request1);
#endif
}

int64_t SSD_READ(struct ssdstate *ssd, unsigned int length, int64_t sector_nb)
{
#if 0
    int64_t curtime = get_usec();
    if (curtime - last_time >= 100000) {
        /* Coperd: print statistics */
        printf("curtime=%ld, GC=%ld, CP_PAGES=%ld\n", curtime, mygc_cnt, mycopy_page_nb);
        last_time = curtime;
        mycopy_page_nb = 0;
    }
#endif

#if defined SSD_THREAD

	pthread_mutex_lock(&cq_lock);
	DEQUEUE_COMPLETED_READ();
	pthread_mutex_unlock(&cq_lock);

	if(EVENT_QUEUE_IS_FULL(READ, length)){
		r_queue_full = 1;
		while(r_queue_full == 1){
			pthread_cond_signal(&eq_ready);
		}
	}

	pthread_mutex_lock(&eq_lock);
	ENQUEUE_IO(READ, sector_nb, length);
	pthread_mutex_unlock(&eq_lock);

    #ifdef SSD_THREAD_MODE_1
	pthread_cond_signal(&eq_ready);
    #endif

#elif defined FIRM_IO_BUFFER
	DEQUEUE_COMPLETED_READ();
	if(EVENT_QUEUE_IS_FULL(READ, length)){
		SECURE_READ_BUFFER();
	}
	ENQUEUE_IO(READ, sector_nb, length);
#else
	return FTL_READ(ssd, sector_nb, length);
#endif
}

void SSD_DSM_TRIM(struct ssdstate *ssd, unsigned int length, void* trim_data)
{
/*
	if(DSM_TRIM_ENABLE == 0)
		return;
		
	
	sector_entry* pSE_T = NULL;
	sector_entry* pSE = NULL;
	
	int i;
	int64_t sector_nb;
	unsigned int sector_length;
	int* pBuff;
	
	pBuff = (int*)trim_data;
	int k = 0;
	for(i=0;i<length;i++)
	{
		sector_nb = (int64_t)*pBuff;
		pBuff++;
		sector_length = (int)*pBuff;	
		sector_length = sector_length >> 16;
		if(sector_nb == 0 && sector_length == 0)
			break;
		
		pSE_T = new_sector_entry();
		pSE_T->sector_nb = sector_nb;
		pSE_T->length = sector_length;
		
		if(pSE == NULL)
			pSE = pSE_T;
		else
		{
			k++;
			add_sector_list(pSE, pSE_T);
		}
		
		pBuff++;
	}
	
	INSERT_TRIM_SECTORS(pSE);

	release_sector_list(pSE);
*/
}

int SSD_IS_SUPPORT_TRIM(struct ssdstate *ssd)
{
//	return DSM_TRIM_ENABLE;
	return 0;
}

int femu_discard_process(struct ssdstate *ssd, uint32_t length, int64_t sector_nb) {
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t SECTOR_NB = sc->SECTOR_NB;
    int64_t SECTORS_PER_PAGE = sc->SECTORS_PER_PAGE;
    int EMPTY_TABLE_ENTRY_NB = sc->EMPTY_TABLE_ENTRY_NB;

	if(sector_nb + length > SECTOR_NB){
		printf("ERROR[%s] Exceed Sector number\n", __FUNCTION__);
        return FAIL;
    }

	int64_t lba = sector_nb;
	int64_t lpn;
	int64_t old_ppn;

	unsigned int remain = length;
	unsigned int left_skip = sector_nb % SECTORS_PER_PAGE;
	unsigned int right_skip;
	unsigned int write_sects;

	unsigned int ret = FAIL;
    //printf("hao_dubug4444444444444: discard start %d len %d\n",sector_nb, length);

    /* 
     * Coperd: since the whole I/O submission path is single threaded, it's
     * safe to do this. "blocking_to" means the time we will block the
     * current I/O to. It will be finally decided by gc timestamps according 
     * to the GC mode you are using.
     */

	while(remain > 0){

		if(remain > SECTORS_PER_PAGE - left_skip){
			right_skip = 0;
		}
		else{
			right_skip = SECTORS_PER_PAGE - left_skip - remain;
		}

		write_sects = SECTORS_PER_PAGE - left_skip - right_skip;

		//add by hao

		//printf("hao_debug:_FTL_WRITEbbbbbbbbbbbbbbbbbbbbbb %d\n", bloom_temp);
		lpn = lba / (int64_t)SECTORS_PER_PAGE;
		old_ppn = GET_MAPPING_INFO(ssd, lpn);
		//printf("hao_debug:_FTL_WRITE lpn old_ppn %d %d\n",lpn, old_ppn);

		if((left_skip || right_skip) && (old_ppn != -1)){
            //printf("hao_dubug4444444444444: ssd page partial write\n");
            /*cur_need_to_emulate_tt = SSD_PAGE_PARTIAL_WRITE(ssd,
				CALC_FLASH(ssd, old_ppn), CALC_BLOCK(ssd, old_ppn), CALC_PAGE(ssd, old_ppn),
				CALC_FLASH(ssd, new_ppn), CALC_BLOCK(ssd, new_ppn), CALC_PAGE(ssd, new_ppn),
				n_io_info);*/
		}

		//printf("hao_debug:_FTL_WRITE new_ppn %d\n", new_ppn);
        //printf("FTL-WRITE: lpn -> ppn: %"PRId64" -> %"PRId64"\n", lpn, new_ppn);

		UPDATE_OLD_PAGE_MAPPING(ssd, lpn);
        ssd->mapping_table[lpn] = -1;   //hao new add
        
		lba += write_sects;
		remain -= write_sects;
		left_skip = 0;
	}
    
	return true;     
}


