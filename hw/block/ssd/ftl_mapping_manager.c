// File: ftl_mapping_manager.c
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"

#include <sys/types.h> 
#include <sys/stat.h>

int64_t* mapping_table;
void* block_table_start;

#ifdef FTL_META_TRANS
	FILE* fp_meta;
#endif

void INIT_MAPPING_TABLE(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);

	/* Allocation Memory for Mapping Table */
	ssd->mapping_table = (int64_t*)calloc(sc->PAGE_MAPPING_ENTRY_NB, sizeof(int64_t));
    ssd->in_seg = (int8_t*)calloc(sc->PAGE_MAPPING_ENTRY_NB, sizeof(int8_t));
	if(ssd->mapping_table == NULL){
		printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
		return;
	}

	/* Initialization Mapping Table */
	
	/* If mapping_table.dat file exists */
	FILE* fp = fopen("./data/mapping_table.dat","r");
	if(fp != NULL){
        printf("init mapping table from file\n");
		fread(ssd->mapping_table, sizeof(int64_t), sc->PAGE_MAPPING_ENTRY_NB, fp);
	}
	else{	
		int i;	
		for(i=0;i<sc->PAGE_MAPPING_ENTRY_NB;i++){
			ssd->mapping_table[i] = -1;
            ssd->in_seg[i] = -1;
		}
	}
}

void TERM_MAPPING_TABLE(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t PAGE_MAPPING_ENTRY_NB = sc->PAGE_MAPPING_ENTRY_NB;
	FILE* fp = fopen("./data/mapping_table.dat","w");
	if(fp==NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
		return;
	}

	/* Write the mapping table to file */
	fwrite(mapping_table, sizeof(int64_t),PAGE_MAPPING_ENTRY_NB,fp);

	/* Free memory for mapping table */
	free(mapping_table);
}

int64_t GET_MAPPING_INFO(struct ssdstate *ssd, int64_t lpn)
{
    int64_t *mapping_table = ssd->mapping_table;
	int64_t ppn = mapping_table[lpn];

	return ppn;
}

int GET_NEW_PAGE(struct ssdstate *ssd, int mode, int mapping_index, int64_t* ppn)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;


	empty_block_entry* curr_empty_block;

	curr_empty_block = GET_EMPTY_BLOCK(ssd, mode, mapping_index);

#ifdef SB_DEBUG
    int off = ssd->empty_block_table_index;
    int plane_nb = curr_empty_block->phy_flash_nb * sc->PLANES_PER_FLASH + curr_empty_block->phy_block_nb % sc->PLANES_PER_FLASH;
    int block_nb = curr_empty_block->phy_block_nb / sc->PLANES_PER_FLASH;
    // printf("index = %d, off = %d, plane_nb = %d, block_nb = %d, page_nb = %d\n", index, off, plane_nb, block_nb, curr_empty_block->curr_phy_page_nb);
    if(plane_nb==0)
    {
        if(ssd->sb_deb.plane_nb != sc->PLANES_PER_FLASH * sc->FLASH_NB - 1)
        {
            printf("1ERROR[%s]: last_plane = %d, new_plane = %d\n", __FUNCTION__, ssd->sb_deb.plane_nb, 0);
            return FAIL;
        }
        ssd->sb_deb.block_nb = block_nb;
        ssd->sb_deb.plane_nb = 0;
    }
    else
    {
        if(ssd->sb_deb.block_nb != block_nb)
        {
            printf("ERROR[%s]: ssd->sb_deb.block_nb = %d, block_nb = %d\n", __FUNCTION__, ssd->sb_deb.block_nb, block_nb);
            return FAIL;
        }
        if(ssd->sb_deb.plane_nb != plane_nb-1)
        {
            printf("2ERROR[%s]: last_plane = %d, new_plane = %d\n", __FUNCTION__, ssd->sb_deb.plane_nb, plane_nb);
            return FAIL;
        }
        ssd->sb_deb.plane_nb = plane_nb;
        // ssd->sb_deb.block_nb = block_nb;
    }
    // fclose(fp);
    // printf("curr_empty_block->phy_block_nb=%d, sc->PLANES_PER_FLASH=%d\n",curr_empty_block->phy_block_nb, sc->PLANES_PER_FLASH);
#endif

	/* If the flash memory has no empty block,
                Get empty block from the other flash memories */
    if(mode == VICTIM_INCHIP && curr_empty_block == NULL){
        /* Try again */
        curr_empty_block = GET_EMPTY_BLOCK(ssd, VICTIM_OVERALL, mapping_index);
    }

	if(curr_empty_block == NULL){
		printf("ERROR[%s] fail\n", __FUNCTION__);
		return FAIL;
	}

	*ppn = curr_empty_block->phy_flash_nb*BLOCK_NB*PAGE_NB \
	       + curr_empty_block->phy_block_nb*PAGE_NB \
	       + curr_empty_block->curr_phy_page_nb;

	curr_empty_block->curr_phy_page_nb += 1;

	return SUCCESS;
}

int UPDATE_OLD_PAGE_MAPPING(struct ssdstate *ssd, int64_t lpn)
{
	int64_t old_ppn;
#ifdef FTL_MAP_CACHE
	old_ppn = CACHE_GET_PPN(lpn);
#else
	old_ppn = GET_MAPPING_INFO(ssd, lpn);
    //printf("shuai_debug: old_ppn = %d\n", old_ppn);
#endif

	if(old_ppn == (int64_t)(-1)){
#ifdef FTL_DEBUG
		printf("[%s] New page \n", __FUNCTION__);
#endif
		return SUCCESS;
	}
	else{
        if(DECREASE_INVERSE_MAPPING(ssd, old_ppn, lpn) == SUCCESS)
        {
            UPDATE_BLOCK_STATE_ENTRY(ssd, CALC_FLASH(ssd, old_ppn), CALC_BLOCK(ssd, old_ppn), CALC_PAGE(ssd, old_ppn), INVALID);
            if(ssd->in_seg[lpn] == 1)
            {
                INCREASE_INVALID_OOB_ENTRY_COUNT(ssd, CALC_BLOCK(ssd, old_ppn));
            }
        }
	}

	return SUCCESS;
}

int UPDATE_NEW_PAGE_MAPPING(struct ssdstate *ssd, int64_t lpn, int64_t ppn,	int block_type)
{
    int64_t *mapping_table = ssd->mapping_table;

	/* Update Inverse Page Mapping Table */
    if(INCREASE_INVERSE_MAPPING(ssd, ppn, lpn) == SUCCESS)
    {
    /* Update Page Mapping Table */
#ifdef FTL_MAP_CACHE
	    CACHE_UPDATE_PPN(lpn, ppn);
#else
	    mapping_table[lpn] = ppn;
#endif
        UPDATE_BLOCK_STATE(ssd, CALC_FLASH(ssd, ppn), CALC_BLOCK(ssd, ppn), block_type);
	    UPDATE_BLOCK_STATE_ENTRY(ssd, CALC_FLASH(ssd, ppn), CALC_BLOCK(ssd, ppn), CALC_PAGE(ssd, ppn), VALID);
    }

	return SUCCESS;
}

unsigned int CALC_FLASH(struct ssdstate *ssd, int64_t ppn)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;
    int FLASH_NB = sc->FLASH_NB;

	unsigned int flash_nb = (ppn/PAGE_NB)/BLOCK_NB;

	if(flash_nb >= FLASH_NB){
        printf("ppn = %ld\n", ppn);
		printf("ERROR[%s] flash_nb %u\n", __FUNCTION__,flash_nb);
	}
	return flash_nb;
}

unsigned int CALC_BLOCK(struct ssdstate *ssd, int64_t ppn)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;

	unsigned int block_nb = (ppn/PAGE_NB)%BLOCK_NB;

	if(block_nb >= BLOCK_NB){
        printf("ppn = %ld\n", ppn);
		printf("ERROR[%s] block_nb %u\n",__FUNCTION__, block_nb);
	}
	return block_nb;
}

unsigned int CALC_PAGE(struct ssdstate *ssd, int64_t ppn)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int PAGE_NB = sc->PAGE_NB;

	unsigned int page_nb = ppn%PAGE_NB;

	return page_nb;
}


//add by hao
void INIT_METADATA_TABLE(struct ssdstate *ssd) 
{
    char *state = NULL;
    struct stat buf;
    size_t res;

	struct ssdconf *sc = &(ssd->ssdparams);
    ssd->int_meta_size = 0;      // Internal meta (state: ERASED / WRITTEN)

    //
    // Internal meta are the first "ln->int_meta_size" bytes
    // Then comes the tgt_oob_len with is the following ln->param.sos bytes
    //

    ssd->meta_len = ssd->int_meta_size + sc->sos;
    ssd->meta_tbytes = ssd->meta_len;
    // ssd->meta_tbytes = ssd->meta_len * sc->PAGE_MAPPING_ENTRY_NB; //hao:整个元数据所需要的最大空间
    // printf("Coperd,allocating meta_buf: %d MB\n", ssd->meta_tbytes/1024/1024);
    /* Coperd: we put all the meta data into this buffer */
    ssd->meta_buf = malloc(ssd->meta_tbytes);
    if (!ssd->meta_buf) {
        printf("Coperd, meta buffer allocation failed!\n");
        exit(1);
    }
    memset(ssd->meta_buf, 0, ssd->meta_tbytes);

#ifdef FTL_META_TRANS
	fp_meta = fopen("./data/meta.txt","a");
#endif  

#if 0                                 //changed by hao: 0--->1
    if (!ssd->meta_fname) {      // Default meta file
        ssd->meta_auto_gen = 1;
        ssd->meta_fname = malloc(10);
        if (!ssd->meta_fname) {
			printf("ERROR[%s] fail\n", __FUNCTION__);
			return;
	    }

        strncpy(ssd->meta_fname, "meta.txt\0", 10);
        printf("Coperd,femu_oc_init_meta, setting meta_fname=%s\n", ssd->meta_fname);
    } else {
        ssd->meta_auto_gen = 0;
    }

    ssd->metadata = fopen(ssd->meta_fname, "w+"); // Open the metadata file

    if (!ssd->metadata) {
        error_report("nvme: femu_oc_init_meta: fopen(%s)\n", ssd->meta_fname);
        return;
    }

    if (fstat(fileno(ssd->metadata), &buf)) {                //hao:fileno返回metadata对应的文件描述符，并copy到buf
        error_report("nvme: femu_oc_init_meta: fstat(%s)\n", ssd->meta_fname);
        return;
    }

    if (buf.st_size == ssd->meta_tbytes)             // All good
        return;

    printf("Coperd,meta file size != meta_tbytes[%ld]\n", ssd->meta_tbytes);
    //
    // Create meta-data file when it is empty or invalid
    //
    if (ftruncate(fileno(ssd->metadata), 0)) {
        error_report("nvme: femu_oc_init_meta: ftrunca(%s)\n", ssd->meta_fname);
        return;
    }

    state = malloc(ssd->meta_tbytes);
    if (!state) {
        error_report("nvme: femu_oc_init_meta: malloc f(%s)\n", ssd->meta_fname);
        return;
    }

    memset(state, 0, ssd->meta_tbytes);

    printf("Coperd, init metadata file with all FEMU_OC_SEC_UNKNOWN\n");
    res = fwrite(state, 1, ssd->meta_tbytes, ssd->metadata);

    free(state);

    if (res != ssd->meta_tbytes) {
        error_report("nvme: femu_oc_init_meta: fwrite(%s), res(%lu)\n",
                     ssd->meta_fname, res);
        return;
     }

    rewind(ssd->metadata);  //hao:文件内部的位置指针重新指向一个流（数据流/文件）的开头
#endif

    return;
 }