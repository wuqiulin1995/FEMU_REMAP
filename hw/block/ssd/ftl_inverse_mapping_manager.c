// File: ftl_inverse_mapping_manager.c
// Date: 2014. 12. 03.
// Author: Jinsoo Yoo (jedisty@hanyang.ac.kr)
// Copyright(c)2014
// Hanyang University, Seoul, Korea
// Embedded Software Systems Laboratory. All right reserved

#include "common.h"
#include <math.h>

//FILE* ftl_block_w;

void INIT_INVERSE_MAPPING_TABLE(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t PAGE_MAPPING_ENTRY_NB = sc->PAGE_MAPPING_ENTRY_NB;
	int i = 0, j = 0;

	/* Allocation Memory for Inverse Page Mapping Table */
	// ssd->inverse_mapping_table = (void*)calloc(PAGE_MAPPING_ENTRY_NB, sizeof(int64_t));
	// if(ssd->inverse_mapping_table == NULL){
	// 	printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
	// 	return;
	// }

	// /* Initialization Inverse Page Mapping Table */
	// FILE* fp = fopen("./data/inverse_mapping.dat","r");
	// if(fp != NULL){
	// 	fread(ssd->inverse_mapping_table, sizeof(int64_t), PAGE_MAPPING_ENTRY_NB, fp);
	// }
	// else{
	// 	int i;
	// 	for(i=0;i<PAGE_MAPPING_ENTRY_NB;i++){
	// 		ssd->inverse_mapping_table[i] = -1;
	// 	}
	// }

	ssd->inverse_mapping_table = (void *)calloc(PAGE_MAPPING_ENTRY_NB, sizeof(inverse_mapping_entry));
	if(ssd->inverse_mapping_table == NULL){
		printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
		return;
	}

	inverse_mapping_entry* inverse = (inverse_mapping_entry* )ssd->inverse_mapping_table;
	for(i = 0; i < PAGE_MAPPING_ENTRY_NB; i++)
	{
		inverse->fingerprint = -1;
		inverse->lpn_cnt = 0;
		// for(j = 0; j < MAX_LPN_CNT; j++)
		// {
		// 	inverse->lpn[j] = -1;
		// }
		inverse->head = NULL;
		inverse->tail = NULL;

		inverse += 1;
	}
}

void INIT_NVRAM_OOB(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t SUPERBLOCK_NB = sc->BLOCK_NB;

	/* Allocation Memory for NVRAM_OOB_TABLE */
	ssd->NVRAM_OOB_TABLE = (void*)calloc(SUPERBLOCK_NB, sizeof(NVRAM_OOB_seg));
	if(ssd->NVRAM_OOB_TABLE == NULL){
		printf("ERROR[%s] Calloc NVRAM_OOB_TABLE fail\n", __FUNCTION__);
		return;
	}

	int i;
	NVRAM_OOB_seg* curr_OOB_seg = (NVRAM_OOB_seg*)ssd->NVRAM_OOB_TABLE;

	for(i=0;i<SUPERBLOCK_NB;i++){
		curr_OOB_seg->alloc_seg = 0;
		curr_OOB_seg->free_entry = 0;
		curr_OOB_seg->invalid_entry = 0;

		curr_OOB_seg += 1;
	}
}

void INIT_zipf_AND_fingerprint(struct ssdstate *ssd)
{
	int i;
	double a = 0.2, sum = 0.0;

	ssd->Pzipf = (double*)calloc(UNIQUE_PAGE_NB+1, sizeof(double));
	ssd->fingerprint = (int64_t*)calloc(UNIQUE_PAGE_NB+1, sizeof(int64_t));

	for(i = 0; i <= UNIQUE_PAGE_NB; i++)
	{
		ssd->Pzipf[i] = 0.0;
		ssd->fingerprint[i] = -1;
	}

	for(i = 1; i <= UNIQUE_PAGE_NB; i++)
	{
		sum += 1 / pow((double)i, a);
	}

	for(i = 1; i <= UNIQUE_PAGE_NB; i++)
	{
		ssd->Pzipf[i] = ssd->Pzipf[i-1] + 1 / pow((double)i, a) / sum;
	}
}

void INIT_BLOCK_STATE_TABLE(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t BLOCK_MAPPING_ENTRY_NB = sc->BLOCK_MAPPING_ENTRY_NB;
    int64_t PAGE_MAPPING_ENTRY_NB = sc->PAGE_MAPPING_ENTRY_NB;

	/* Allocation Memory for Inverse Block Mapping Table */
	ssd->block_state_table = (void*)calloc(BLOCK_MAPPING_ENTRY_NB, sizeof(block_state_entry));
	if(ssd->block_state_table == NULL){
		printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
		return;
	}

	/* Initialization Inverse Block Mapping Table */
	FILE* fp = fopen("./data/block_state_table.dat","r");
	if(fp != NULL){
		fread(ssd->block_state_table, sizeof(block_state_entry), BLOCK_MAPPING_ENTRY_NB, fp);
	}
	else{
		int i;
		block_state_entry* curr_b_s_entry = (block_state_entry*)ssd->block_state_table;

		for(i=0;i<BLOCK_MAPPING_ENTRY_NB;i++){
			curr_b_s_entry->type		= EMPTY_BLOCK;
			curr_b_s_entry->valid_page_nb	= 0;
			curr_b_s_entry->erase_count		= 0;

			curr_b_s_entry += 1;
		}
	}
}

void INIT_VALID_ARRAY(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    void *block_state_table = ssd->block_state_table;
    int BLOCK_MAPPING_ENTRY_NB = sc->BLOCK_MAPPING_ENTRY_NB;
    int PAGE_NB = sc->PAGE_NB;

	int i;
	block_state_entry* curr_b_s_entry = (block_state_entry*)block_state_table;
	int8_t* valid_array;

	FILE* fp = fopen("./data/valid_array.dat","r");
	if(fp != NULL){
		for(i=0;i<BLOCK_MAPPING_ENTRY_NB;i++){
			valid_array = (int8_t*)calloc(PAGE_NB, sizeof(int8_t));
			fread(valid_array, sizeof(int8_t), PAGE_NB, fp);
			curr_b_s_entry->valid_array = valid_array;

			curr_b_s_entry += 1;
		}
	}
	else{
		for(i=0;i<BLOCK_MAPPING_ENTRY_NB;i++){
			valid_array = (int8_t*)calloc(PAGE_NB, sizeof(int8_t));
			memset(valid_array,0,PAGE_NB);
			curr_b_s_entry->valid_array = valid_array;

			curr_b_s_entry += 1;
		}			
	}
}

void INIT_EMPTY_BLOCK_LIST(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_MAPPING_ENTRY_NB = sc->BLOCK_MAPPING_ENTRY_NB;
    int PLANES_PER_FLASH = sc->PLANES_PER_FLASH;
    int FLASH_NB = sc->FLASH_NB;
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;
	int i, j, k;

	empty_block_entry* curr_entry;
	empty_block_root* curr_root;

	empty_block_entry* empty_curr_entry;
	ssd->empty_block_list = (void*)calloc(PLANES_PER_FLASH * FLASH_NB, sizeof(empty_block_root));
	if(ssd->empty_block_list == NULL){
		printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
		return;
	}

	FILE* fp = fopen("./data/empty_block_list.dat","r");
	if(fp != NULL){
		ssd->total_empty_block_nb = 0;
		fread(ssd->empty_block_list,sizeof(empty_block_root),PLANES_PER_FLASH*FLASH_NB, fp);
		curr_root = (empty_block_root*)ssd->empty_block_list;
		ssd->empty_block_table_index = 0;
#ifdef SB_DEBUG
		memset(&ssd->sb_deb, 0x00, sizeof(sb_debug));
		ssd->sb_deb.plane_nb = sc->PLANES_PER_FLASH * sc->FLASH_NB - 1;
#endif
	}
	else{
		curr_root = (empty_block_root*)ssd->empty_block_list;		
		for(i=0;i<PLANES_PER_FLASH;i++)
		{
			for(j=0;j<FLASH_NB;j++)
			{
				for(k=i;k<BLOCK_NB;k+=PLANES_PER_FLASH)
				{
					curr_entry = (empty_block_entry*)calloc(1, sizeof(empty_block_entry));	
					if(curr_entry == NULL){
						printf("ERROR[%s] Calloc fail\n", __FUNCTION__);
						break;
					}
	
					if(k==i)
					{
						curr_root->empty_head = curr_entry;
						curr_root->head = NULL;
						curr_root->tail = curr_entry;
						curr_root->tail->phy_flash_nb = j;
						curr_root->tail->phy_block_nb = k;
						curr_root->tail->curr_phy_page_nb = 0;
					}
					else
					{
						curr_entry->next = NULL;
						curr_root->tail->next = curr_entry;
						curr_root->tail = curr_entry;
						curr_root->tail->phy_flash_nb = j;
						curr_root->tail->phy_block_nb = k;
						curr_root->tail->curr_phy_page_nb = 0;
					}
					UPDATE_BLOCK_STATE(ssd, j, k, EMPTY_BLOCK);
				}
				curr_root->empty_block_nb = (unsigned int)sc->EACH_EMPTY_TABLE_ENTRY_NB;
				curr_root += 1;
			}
		}
		//shuai: the location here is a little bit different from the codes
		//i downloaded from github
		ssd->total_empty_block_nb = (int64_t)BLOCK_MAPPING_ENTRY_NB;
		ssd->empty_block_table_index = 0;
#ifdef SB_DEBUG
		memset(&ssd->sb_deb, 0x00, 1*sizeof(sb_debug));
		ssd->sb_deb.plane_nb = sc->PLANES_PER_FLASH * sc->FLASH_NB - 1;
#endif
	}
	// printf("*******ssd->total_empty_block_nb = %ld\n******", ssd->total_empty_block_nb);
}

void INIT_VICTIM_BLOCK_LIST(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_MAPPING_ENTRY_NB = sc->BLOCK_MAPPING_ENTRY_NB;
    int PLANES_PER_FLASH = sc->PLANES_PER_FLASH;
    int FLASH_NB = sc->FLASH_NB;
    int PAGE_NB = sc->PAGE_NB;
    int64_t EACH_EMPTY_TABLE_ENTRY_NB = sc->EACH_EMPTY_TABLE_ENTRY_NB;

	int i, j, k;

	victim_block_entry* curr_entry;
	victim_block_root* curr_root;

	ssd->victim_block_list = (void*)calloc(PLANES_PER_FLASH * FLASH_NB, sizeof(victim_block_root));
	if(ssd->victim_block_list == NULL){
		printf("ERROR[%s] Calloc mapping table fail\n", __FUNCTION__);
		return;
	}

	FILE* fp = fopen("./data/victim_block_list.dat","r");
	if(fp != NULL){
		ssd->total_victim_block_nb = 0;
		fread(ssd->victim_block_list, sizeof(victim_block_root), PLANES_PER_FLASH*FLASH_NB, fp);
		curr_root = (victim_block_root*)ssd->victim_block_list;

		for(i=0;i<PLANES_PER_FLASH;i++){

			for(j=0;j<FLASH_NB;j++){

				ssd->total_victim_block_nb += curr_root->victim_block_nb;
				k = curr_root->victim_block_nb;
				while(k > 0){
					curr_entry = (victim_block_entry*)calloc(1, sizeof(victim_block_entry));
					if(curr_entry == NULL){
						printf("ERROR[%s] Calloc fail\n", __FUNCTION__);
						break;
					}

					fread(curr_entry, sizeof(victim_block_entry), 1, fp);
					curr_entry->next = NULL;
					curr_entry->prev = NULL;

					if(k == curr_root->victim_block_nb){
						curr_root->head = curr_entry;
						curr_root->tail = curr_entry;
					}					
					else{
						curr_root->tail->next = curr_entry;
						curr_entry->prev = curr_root->tail;
						curr_root->tail = curr_entry;
					}
					k--;
				}
				curr_root += 1;
			}
		}
	}
	else{
		curr_root = (victim_block_root*)ssd->victim_block_list;		

		for(i=0;i<PLANES_PER_FLASH;i++){

			for(j=0;j<FLASH_NB;j++){

				curr_root->head = NULL;
				curr_root->tail = NULL;
				curr_root->victim_block_nb = 0;

				curr_root += 1;
			}
		}
		ssd->total_victim_block_nb = 0;
	}
}

#if 0
void TERM_INVERSE_MAPPING_TABLE(struct ssdstate *ssd)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    void *inverse_mapping_table = ssd->inverse_mapping_table;
    int64_t PAGE_MAPPING_ENTRY_NB = sc->PAGE_MAPPING_ENTRY_NB;

	FILE* fp = fopen("./data/inverse_mapping.dat", "w");
	if(fp==NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
		return;
	}

	/* Write The inverse page table to file */
	fwrite(inverse_mapping_table, sizeof(int64_t), PAGE_MAPPING_ENTRY_NB, fp);

	/* Free the inverse page table memory */
	free(inverse_mapping_table);
}
#endif

#if 0
void TERM_BLOCK_STATE_TABLE(struct ssdstate *ssd)
{
	FILE* fp = fopen("./data/block_state_table.dat","w");
	if(fp==NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
		return;
	}

	/* Write The inverse block table to file */
	fwrite(block_state_table, sizeof(block_state_entry), BLOCK_MAPPING_ENTRY_NB, fp);

	/* Free The inverse block table memory */
	free(block_state_table);
}
#endif

#if 0
void TERM_VALID_ARRAY(void)
{
	int i;
	block_state_entry* curr_b_s_entry = (block_state_entry*)block_state_table;
	int8_t* valid_array;

	FILE* fp = fopen("./data/valid_array.dat","w");
        if(fp == NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
		return;
	}
 
	for(i=0;i<BLOCK_MAPPING_ENTRY_NB;i++){
		valid_array = curr_b_s_entry->valid_array;
		fwrite(valid_array, sizeof(int8_t), PAGE_NB, fp);
		curr_b_s_entry += 1;
	}
}
#endif

#if 0
void TERM_EMPTY_BLOCK_LIST(void)
{
	int i, j, k;

	empty_block_entry* curr_entry;
	empty_block_root* curr_root;

	FILE* fp = fopen("./data/empty_block_list.dat","w");
	if(fp==NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
	}

	fwrite(empty_block_list,sizeof(empty_block_root),PLANES_PER_FLASH*FLASH_NB, fp);

	curr_root = (empty_block_root*)empty_block_list;
	for(i=0;i<PLANES_PER_FLASH;i++){

		for(j=0;j<FLASH_NB;j++){

			k = curr_root->empty_block_nb;
			if(k != 0){
				curr_entry = (empty_block_entry*)curr_root->head;
			}
			while(k > 0){

				fwrite(curr_entry, sizeof(empty_block_entry), 1, fp);

				if(k != 1){
					curr_entry = curr_entry->next;
				}
				k--;
			}
			curr_root += 1;
		}
	}
}
#endif

#if 0
void TERM_VICTIM_BLOCK_LIST(void)
{
	int i, j, k;

	victim_block_entry* curr_entry;
	victim_block_root* curr_root;

	FILE* fp = fopen("./data/victim_block_list.dat","w");
	if(fp==NULL){
		printf("ERROR[%s] File open fail\n", __FUNCTION__);
	}

	fwrite(victim_block_list, sizeof(victim_block_root), PLANES_PER_FLASH*FLASH_NB, fp);

	curr_root = (victim_block_root*)victim_block_list;
	for(i=0;i<PLANES_PER_FLASH;i++){

		for(j=0;j<FLASH_NB;j++){

			k = curr_root->victim_block_nb;
			if(k != 0){
				curr_entry = (victim_block_entry*)curr_root->head;
			}
			while(k > 0){

				fwrite(curr_entry, sizeof(victim_block_entry), 1, fp);

				if(k != 1){
					curr_entry = curr_entry->next;
				}
				k--;
			}
			curr_root += 1;
		}
	}
}
#endif

empty_block_entry* GET_EMPTY_BLOCK(struct ssdstate *ssd, int mode, int mapping_index)
{
	struct ssdconf *sc = &(ssd->ssdparams);
    void *empty_block_list = ssd->empty_block_list;
    int64_t EMPTY_TABLE_ENTRY_NB = sc->EMPTY_TABLE_ENTRY_NB;
    int PAGE_NB = sc->PAGE_NB;
    int PLANES_PER_FLASH = sc->PLANES_PER_FLASH;
	//FILE* fp = fopen("./data/1234.dat","a");
	//printf("*******ssd->total_empty_block_nb = %ld\n******", ssd->total_empty_block_nb);
    if(ssd->total_empty_block_nb == 0){
        printf("ERROR[%s] There is no empty block\n", __FUNCTION__);
        return NULL;
    }

    int input_mapping_index = mapping_index;
	unsigned int phy_flash_nb;
	unsigned int phy_block_nb;
	block_state_entry* b_s_entry;
	int i = 0;

    empty_block_entry* curr_empty_block;
    empty_block_root* curr_root_entry;

	while(ssd->total_empty_block_nb != 0)
	{
		if(mode == VICTIM_OVERALL)
		{
			curr_root_entry = (empty_block_root*)empty_block_list + ssd->empty_block_table_index;
			if(curr_root_entry->head == NULL)
			{
				if(curr_root_entry->empty_head == NULL)
				{
					ssd->empty_block_table_index++;
                    if(ssd->empty_block_table_index == EMPTY_TABLE_ENTRY_NB){
                     	ssd->empty_block_table_index = 0;
                	}
					continue;
				}
				else
				{
					if(ssd->empty_block_table_index==0)
					{
						int i;
						empty_block_entry* tmp_curr_empty_block;
    					empty_block_root* tmp_curr_root_entry;
						for(i=0; i<EMPTY_TABLE_ENTRY_NB; i++)
						{
							tmp_curr_root_entry = (empty_block_root*)empty_block_list + i;
							tmp_curr_empty_block =  tmp_curr_root_entry->empty_head;
							tmp_curr_root_entry->empty_head = tmp_curr_root_entry->empty_head->next;
							tmp_curr_empty_block->next = NULL;
							tmp_curr_root_entry->head = tmp_curr_empty_block;
							tmp_curr_root_entry->empty_block_nb -= 1;
						}
						ssd->empty_block_table_index++;
						return ((empty_block_root*)empty_block_list)->head;
					}
					else
					{
						printf("ERROR[%s]: Plane!=0, but head == NULL", __FUNCTION__);
					}	
				}				
			}
			else if(curr_root_entry->head != NULL)
			{
				curr_empty_block = curr_root_entry->head;
				if(curr_empty_block->curr_phy_page_nb == PAGE_NB)
				{
					if(curr_empty_block->phy_flash_nb == sc->FLASH_NB-1 
						&& curr_empty_block->phy_block_nb % sc->PLANES_PER_FLASH == sc->PLANES_PER_FLASH-1)
					{
						int i;
						empty_block_entry* tmp_curr_empty_block;
    					empty_block_root* tmp_curr_root_entry;
						for(i=0; i<EMPTY_TABLE_ENTRY_NB; i++)
						{
							tmp_curr_root_entry = (empty_block_root*)empty_block_list + i;
							tmp_curr_empty_block = tmp_curr_root_entry->head;
							INSERT_VICTIM_BLOCK(ssd, tmp_curr_empty_block);
							ssd->total_empty_block_nb -= 1;
							tmp_curr_root_entry->head = NULL;
						}
						if(ssd->empty_block_table_index!=EMPTY_TABLE_ENTRY_NB-1)
						{
							printf("ERROR[%s]: ssd->empty_block_table_index!=EMPTY_TABLE_ENTRY_NB-1", __FUNCTION__);
						}
						ssd->empty_block_table_index = 0;
					}
					else
					{
						ssd->empty_block_table_index++;
						if(ssd->empty_block_table_index == EMPTY_TABLE_ENTRY_NB){
							printf("ERROR[%s]: This cannot happen!\n", __FUNCTION__);
							ssd->empty_block_table_index = 0;
						}
					}
					continue;
				}
				else
				{
					ssd->empty_block_table_index++;
                	if(ssd->empty_block_table_index == EMPTY_TABLE_ENTRY_NB){
                    	ssd->empty_block_table_index = 0;
                	}
                	return curr_empty_block;
				}		
			}
		}
		else if(mode == VICTIM_INCHIP){
            curr_root_entry = (empty_block_root*)empty_block_list + mapping_index;
            if(curr_root_entry->empty_block_nb == 0){

                /* If the flash memory has multiple planes, move index */
                if(PLANES_PER_FLASH != 1){
                    mapping_index++;
                    if(mapping_index % PLANES_PER_FLASH == 0){
                        mapping_index = mapping_index - (PLANES_PER_FLASH-1);
                    }
                    if(mapping_index == input_mapping_index){
                        printf("ERROR[%s] There is no empty block\n",__FUNCTION__);
                        return NULL;
                    }
                }
                /* If there is no empty block in the flash memory, return fail */
                else{
#ifdef FTL_DEBUG
                    printf("ERROR[%s]-INCHIP There is no empty block\n",__FUNCTION__);
#endif
                    return NULL;
                }
                continue;
            }
            else{
                return curr_empty_block;
            }	
        }

        else if(mode == VICTIM_NOPARAL){
            curr_root_entry = (empty_block_root*)empty_block_list + mapping_index;
            if(curr_root_entry->empty_block_nb == 0){

                mapping_index++;
                ssd->empty_block_table_index++;
                if(mapping_index == EMPTY_TABLE_ENTRY_NB){
                    mapping_index = 0;
                    ssd->empty_block_table_index = 0;
                }
                continue;
            }
            else{
                 if(curr_empty_block->curr_phy_page_nb == PAGE_NB){

                    /* Eject Empty Block from the list */
                    INSERT_VICTIM_BLOCK(ssd, curr_empty_block);

                    /* Update The total number of empty block */
                    ssd->total_empty_block_nb--;

                    continue;
                }
                return curr_empty_block;
            }	
        }
    }

    printf("ERROR[%s] There is no empty block\n", __FUNCTION__);
    return NULL;
}

int INSERT_EMPTY_BLOCK(struct ssdstate *ssd, unsigned int phy_flash_nb, unsigned int phy_block_nb)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int FLASH_NB = sc->FLASH_NB;
    int PLANES_PER_FLASH = sc->PLANES_PER_FLASH;

    void *empty_block_list = ssd->empty_block_list;

	int mapping_index;
	int plane_nb;

	empty_block_root* curr_root_entry;
	empty_block_entry* new_empty_block;

	new_empty_block = (empty_block_entry*)calloc(1, sizeof(empty_block_entry));
	if(new_empty_block == NULL){
		printf("ERROR[%s] Alloc new empty block fail\n", __FUNCTION__);
		return FAIL;
	}

	/* Init New empty block */
	new_empty_block->phy_flash_nb = phy_flash_nb;
	new_empty_block->phy_block_nb = phy_block_nb;
	new_empty_block->curr_phy_page_nb = 0;
	new_empty_block->next = NULL;

	plane_nb = phy_block_nb % PLANES_PER_FLASH;
	mapping_index = plane_nb * FLASH_NB + phy_flash_nb;

	curr_root_entry = (empty_block_root*)empty_block_list + mapping_index;

	if(curr_root_entry->empty_head == NULL){
		curr_root_entry->empty_head = new_empty_block;
		curr_root_entry->tail = new_empty_block;
		curr_root_entry->empty_block_nb = 1;
	}
	else{
		curr_root_entry->tail->next = new_empty_block;
		curr_root_entry->tail = new_empty_block;
		curr_root_entry->empty_block_nb += 1;
	}
	ssd->total_empty_block_nb++;
	// printf("[%s]: flash_nb =%d, block_nb = %d\n", __FUNCTION__, phy_flash_nb, phy_block_nb);
	return SUCCESS;
}

int INSERT_VICTIM_BLOCK(struct ssdstate *ssd, empty_block_entry* full_block)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int FLASH_NB = sc->FLASH_NB;
    int PLANES_PER_FLASH = sc->PLANES_PER_FLASH;

    void *victim_block_list = ssd->victim_block_list;

	int mapping_index;
	int plane_nb;

	victim_block_root* curr_v_b_root;
	victim_block_entry* new_v_b_entry;

	/* Alloc New victim block entry */
	new_v_b_entry = (victim_block_entry*)calloc(1, sizeof(victim_block_entry));
	if(new_v_b_entry == NULL){
		printf("ERROR[%s] Calloc fail\n", __FUNCTION__);
		return FAIL;
	}

	/* Copy the full block address */
	new_v_b_entry->phy_flash_nb = full_block->phy_flash_nb;
	new_v_b_entry->phy_block_nb = full_block->phy_block_nb;
	new_v_b_entry->prev = NULL;
	new_v_b_entry->next = NULL;

	plane_nb = full_block->phy_block_nb % PLANES_PER_FLASH;
	mapping_index = plane_nb * FLASH_NB + full_block->phy_flash_nb;
	// printf("mapping_index = %d\n", mapping_index);
	curr_v_b_root = (victim_block_root*)victim_block_list + mapping_index;

	/* Update victim block list */
	if(curr_v_b_root->victim_block_nb == 0){
		curr_v_b_root->head = new_v_b_entry;
		curr_v_b_root->tail = new_v_b_entry;
		curr_v_b_root->victim_block_nb = 1;
	}
	else{
		curr_v_b_root->tail->next = new_v_b_entry;
		new_v_b_entry->prev = curr_v_b_root->tail;
		curr_v_b_root->tail = new_v_b_entry;
		curr_v_b_root->victim_block_nb++;
	}

	// printf("mapping_index = %d, curr_v_b_root->victim_block_nb = %d\n", mapping_index, curr_v_b_root->victim_block_nb);

	/* Free the full empty block entry */
	free(full_block);

	/* Update the total number of victim block */
	ssd->total_victim_block_nb++;

	return SUCCESS;
}

int EJECT_VICTIM_BLOCK(struct ssdstate *ssd, victim_block_entry* victim_block)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t PLANES_PER_FLASH = sc->PLANES_PER_FLASH;
    int FLASH_NB = sc->FLASH_NB;
    void *victim_block_list = ssd->victim_block_list;

	int mapping_index;
	int plane_nb;

	victim_block_root* curr_v_b_root;

	plane_nb = victim_block->phy_block_nb % PLANES_PER_FLASH;
	mapping_index = plane_nb * FLASH_NB + victim_block->phy_flash_nb;

	curr_v_b_root = (victim_block_root*)victim_block_list + mapping_index;

	/* Update victim block list */
	if(victim_block == curr_v_b_root->head){
		if(curr_v_b_root->victim_block_nb == 1){
			curr_v_b_root->head = NULL;
			curr_v_b_root->tail = NULL;
		}
		else{
			curr_v_b_root->head = victim_block->next;
			curr_v_b_root->head->prev = NULL;
		}
	}
	else if(victim_block == curr_v_b_root->tail){
		curr_v_b_root->tail = victim_block->prev;
		curr_v_b_root->tail->next = NULL;
	}
	else{
		victim_block->prev->next = victim_block->next;
		victim_block->next->prev = victim_block->prev;
	}

	curr_v_b_root->victim_block_nb--;
	ssd->total_victim_block_nb--;

	/* Free the victim block */
	free(victim_block);

	return SUCCESS;
}

block_state_entry* GET_BLOCK_STATE_ENTRY(struct ssdstate *ssd, unsigned int phy_flash_nb, unsigned int phy_block_nb)
{
    void *block_state_table = ssd->block_state_table;
    // void *victim_block_list = ssd->victim_block_list;
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_NB = sc->BLOCK_NB;

	int64_t mapping_index = phy_flash_nb * BLOCK_NB + phy_block_nb;

	block_state_entry* mapping_entry = (block_state_entry*)block_state_table + mapping_index;

	return mapping_entry;
}

inverse_mapping_entry* GET_INVERSE_MAPPING_INFO(struct ssdstate *ssd, int64_t ppn)
{
    void *inverse_mapping_table = ssd->inverse_mapping_table;
	
	inverse_mapping_entry* inverse_entry = (inverse_mapping_entry*)inverse_mapping_table + ppn;

	return inverse_entry;
}

// NEED MODIFY
int UPDATE_INVERSE_MAPPING(struct ssdstate *ssd, int64_t ppn, int64_t lpn)
{
    int64_t *inverse_mapping_table = ssd->inverse_mapping_table;
#ifdef FTL_MAP_CACHE
	CACHE_UPDATE_LPN(lpn, ppn);
#else
	inverse_mapping_table[ppn] = lpn;
#endif

	return SUCCESS;
}

int INCREASE_INVERSE_MAPPING(struct ssdstate *ssd, int64_t ppn, int64_t lpn)
{
	void *inverse_mapping_table = ssd->inverse_mapping_table;
	int i = 0;

	if(ppn == -1)
	{
		printf("ERROR[%s]: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
		return FAIL;
	}
	
	inverse_mapping_entry* inverse_entry = (inverse_mapping_entry*)inverse_mapping_table + ppn;
	inverse_node* new_inverse_node;

	// if(inverse_entry->lpn_cnt == MAX_LPN_CNT)
	// {
	// 	printf("ERROR[%s]: Increase inverse mapping at MAX_LPN_CNT: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
	// 	return FAIL;
	// }

	// for(i = 0; i < MAX_LPN_CNT; i++)
	// {
	// 	if(inverse_entry->lpn[i] == -1)
	// 	{
	// 		inverse_entry->lpn[i] = lpn;
	// 		inverse_entry->lpn_cnt++;
	// 		// increase_debug_print(ssd, ppn, lpn, inverse_entry->lpn_cnt);
	// 		return SUCCESS;
	// 	}
	// }

	// printf("ERROR[%s]: Increase inverse mapping ---FAIL---: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
	// return FAIL;

	new_inverse_node = (inverse_node*)calloc(1, sizeof(inverse_node));
	if(new_inverse_node == NULL)
	{
		printf("ERROR[%s] Alloc new inverse node fail\n", __FUNCTION__);
		return FAIL;
	}

	new_inverse_node->lpn = lpn;
	new_inverse_node->pre = NULL;
	new_inverse_node->next = NULL;

	if(inverse_entry->lpn_cnt == 0)
	{
		inverse_entry->head = new_inverse_node;
		inverse_entry->tail = new_inverse_node;
		inverse_entry->lpn_cnt = 1;
	}
	else
	{
		inverse_entry->tail->next = new_inverse_node;
		new_inverse_node->pre = inverse_entry->tail;	
		inverse_entry->tail = new_inverse_node;
		inverse_entry->lpn_cnt++;
	}

	return SUCCESS;
	
}

int DECREASE_INVERSE_MAPPING(struct ssdstate *ssd, int64_t ppn, int64_t lpn)
{
	void *inverse_mapping_table = ssd->inverse_mapping_table;
	int i = 0;

	if(ppn == -1)
	{
		printf("ERROR[%s]: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
		return FAIL;
	}
	
	inverse_mapping_entry *inverse_entry = (inverse_mapping_entry*)inverse_mapping_table + ppn;
	inverse_node *victim_inverse_node;

	if(inverse_entry->lpn_cnt == 0)
	{
		printf("ERROR[%s]: Decrease inverse mapping at 0 entry: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
		return FAIL;
	}

	// for(i = 0; i < MAX_LPN_CNT; i++)
	// {
	// 	if(inverse_entry->lpn[i] == lpn)
	// 	{
	// 		inverse_entry->lpn[i] = -1;
	// 		inverse_entry->lpn_cnt--;
	// 		// decrease_debug_print(ssd, ppn, lpn, inverse_entry->lpn_cnt);
	// 		return SUCCESS;
	// 	}
	// }

	// printf("ERROR[%s]: Decrease inverse mapping ---FAIL---: ppn = %ld, lpn = %ld\n", __FUNCTION__, ppn, lpn);
	// return FAIL;

	victim_inverse_node = inverse_entry->head;
	while(victim_inverse_node != NULL)
	{
		if(lpn == victim_inverse_node->lpn)
			break;
		
		victim_inverse_node = victim_inverse_node->next;
	}

	if(victim_inverse_node == inverse_entry->head)
	{
		if(inverse_entry->lpn_cnt == 1)
		{
			inverse_entry->head = NULL;
			inverse_entry->tail = NULL;
		}
		else
		{
			inverse_entry->head = victim_inverse_node->next;
			inverse_entry->head->pre = NULL;
		}
	}
	else if(victim_inverse_node == inverse_entry->tail)
	{
		inverse_entry->tail = victim_inverse_node->pre;
		inverse_entry->tail->next = NULL;
	}
	else
	{
		victim_inverse_node->pre->next = victim_inverse_node->next;
		victim_inverse_node->next->pre = victim_inverse_node->pre;
	}
	
	inverse_entry->lpn_cnt--;

	free(victim_inverse_node);
	victim_inverse_node = NULL;

	return SUCCESS;
}


int UPDATE_BLOCK_STATE(struct ssdstate *ssd, unsigned int phy_flash_nb, unsigned int phy_block_nb, int type)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int64_t PLANES_PER_FLASH = sc->PLANES_PER_FLASH;
    int FLASH_NB = sc->FLASH_NB;
    int PAGE_NB = sc->PAGE_NB;

	int i;
	block_state_entry* b_s_entry = GET_BLOCK_STATE_ENTRY(ssd, phy_flash_nb, phy_block_nb);

	b_s_entry->type = type;

	if(type == EMPTY_BLOCK){
		for(i=0;i<PAGE_NB;i++){
			UPDATE_BLOCK_STATE_ENTRY(ssd, phy_flash_nb, phy_block_nb, i, 0); 
		}
	}

    return SUCCESS;
}

int UPDATE_BLOCK_STATE_ENTRY(struct ssdstate *ssd, unsigned int phy_flash_nb, unsigned int phy_block_nb, unsigned int phy_page_nb, int valid)
{
    struct ssdconf *sc = &(ssd->ssdparams);
    int BLOCK_NB = sc->BLOCK_NB;
    int PAGE_NB = sc->PAGE_NB;
    int FLASH_NB = sc->FLASH_NB;
	inverse_mapping_entry* inverse_entry;
	int64_t ppn = phy_flash_nb*BLOCK_NB*PAGE_NB + phy_block_nb*PAGE_NB + phy_page_nb;
	int64_t fing;

	if(phy_flash_nb >= FLASH_NB || phy_block_nb >= BLOCK_NB || phy_page_nb >= PAGE_NB){
		printf("ERROR[%s] Wrong physical address\n", __FUNCTION__);
		return FAIL;
	}

	int i = 0;
	int valid_count = 0;
	block_state_entry* b_s_entry = GET_BLOCK_STATE_ENTRY(ssd, phy_flash_nb, phy_block_nb);

	int8_t* valid_array = b_s_entry->valid_array;

	if(valid == VALID)
	{
		// if(valid_array[phy_page_nb] >= 0 && valid_array[phy_page_nb] < MAX_LPN_CNT)
		if(valid_array[phy_page_nb] >= 0)
		{
			if(valid_array[phy_page_nb] == 0)
			{
				ssd->stat_ppn_free--;
				ssd->stat_ppn_valid++;
			}
			else if(valid_array[phy_page_nb] == 1)
			{
				ssd->stat_ppn_n21++;
			}
			valid_array[phy_page_nb]++;
			if(valid_array[phy_page_nb] > ssd->max_valid_array)
				ssd->max_valid_array = valid_array[phy_page_nb];
			ssd->stat_lpn_valid++;
		}
		else
		{
			printf("ERROR[%s] VALID Wrong valid array: %d\n", __FUNCTION__, valid_array[phy_page_nb]);
			return FAIL;
		}
	}
	else if(valid == INVALID)
	{
		// if(valid_array[phy_page_nb] > 0 && valid_array[phy_page_nb] <= MAX_LPN_CNT)
		if(valid_array[phy_page_nb] > 0)
		{
			if(valid_array[phy_page_nb] == 1)
			{
				ssd->stat_ppn_invalid++;
				ssd->stat_ppn_valid--;
				valid_array[phy_page_nb] = -1;
#ifdef DUP_RATIO
				// inverse_entry = GET_INVERSE_MAPPING_INFO(ssd, ppn);
				// fing = inverse_entry->fingerprint;
                // if(inverse_entry->lpn_cnt != 0)
                // {
                //     printf("ERROR[%s]: inverse_entry->lpn_cnt !=0 (%d)\n", __FUNCTION__, inverse_entry->lpn_cnt);
                // }
				// if(fing < 1 || fing > UNIQUE_PAGE_NB)
				// {
				// 	printf("ERROR[%s]: fing < 1 || fing > UNIQUE_PAGE_NB, %d\n", __FUNCTION__, fing);
				// }
                // ssd->fingerprint[fing] = -1;
#endif
			}
			else
			{
				if(valid_array[phy_page_nb] == 2)
				{
					ssd->stat_ppn_n21--;
				}
				valid_array[phy_page_nb]--;
			}	
			ssd->stat_lpn_valid--;
		}
		else
		{
			printf("ERROR[%s] INVALID Wrong valid array: %d\n", __FUNCTION__, valid_array[phy_page_nb]);
			return FAIL;
		}
	}
	else if(valid == 0)
	{
		// if(valid_array[phy_page_nb] > 0 && valid_array[phy_page_nb] <= MAX_LPN_CNT)
		if(valid_array[phy_page_nb] > 0)
		{
			ssd->stat_ppn_valid--;
			if(valid_array[phy_page_nb] > 1)
			{
				ssd->stat_ppn_n21--;
			}
			ssd->stat_lpn_valid -= valid_array[phy_page_nb];
			valid_array[phy_page_nb] = 0;
			ssd->stat_ppn_free++;
		}
		else if(valid_array[phy_page_nb] == -1)
		{
			ssd->stat_ppn_invalid--;
			valid_array[phy_page_nb] = 0;
			ssd->stat_ppn_free++;
		}
		else if(valid_array[phy_page_nb] == 0)
		{
			
		}
		else
		{
			printf("ERROR[%s] FREE Wrong valid array: %d\n", __FUNCTION__, valid_array[phy_page_nb]);
			return FAIL;
		}
	}
	else
	{
		printf("ERROR[%s] Wrong valid value: valid = %d\n", __FUNCTION__, valid);
	}

	/* Update valid_page_nb */
	for(i=0;i<PAGE_NB;i++)
	{
		// if(valid_array[i] > 0 && valid_array[i] <= MAX_LPN_CNT)
		if(valid_array[i] > 0)
		{
			valid_count++;
		}
	}
	b_s_entry->valid_page_nb = valid_count;
	
	return SUCCESS;
}

int USE_REMAP(struct ssdstate *ssd, unsigned int phy_block_nb)
{
	struct ssdconf *sc = &(ssd->ssdparams);
	void* NVRAM_OOB_TABLE = ssd->NVRAM_OOB_TABLE;
    int BLOCK_NB = sc->BLOCK_NB;
	NVRAM_OOB_seg* OOB_seg;
	double invalid_ratio = 0.0;
	int count = 0;

	if(phy_block_nb >= BLOCK_NB){
		printf("ERROR[%s] Wrong physical address\n", __FUNCTION__);
		return FAIL;
	}

	OOB_seg = (NVRAM_OOB_seg*)NVRAM_OOB_TABLE + phy_block_nb;

	if(OOB_seg->free_entry > 0)
		return SUCCESS;

	if(ssd->stat_total_alloc_seg < TOTAL_OOB_SEG)
		return SUCCESS;

	if(ssd->stat_total_OOB_entry > 0)
		invalid_ratio = (double)(ssd->stat_total_invalid_entry) / (ssd->stat_total_OOB_entry);

	if(ssd->stat_total_alloc_seg == TOTAL_OOB_SEG && invalid_ratio <= INVALID_ENTRY_THRE)
		return FAIL;

	while(ssd->stat_total_alloc_seg == TOTAL_OOB_SEG && invalid_ratio > INVALID_ENTRY_THRE && count <= BLOCK_NB)
	{		
		if(NVRAM_OOB_GC(ssd) == FAIL)
			return FAIL;

		count++;

		if(ssd->stat_total_OOB_entry > 0)
			invalid_ratio = (double)(ssd->stat_total_invalid_entry) / (ssd->stat_total_OOB_entry);

		if(count == BLOCK_NB)
			printf("[%s] NVRAM GC count = %d\n", __FUNCTION__, count);
		
		if(OOB_seg->free_entry > 0)
			return SUCCESS;

		if(ssd->stat_total_alloc_seg < TOTAL_OOB_SEG)
			return SUCCESS;
	}

	return FAIL;
}

int NVRAM_OOB_GC(struct ssdstate *ssd)
{
	struct ssdconf *sc = &(ssd->ssdparams);
	void* NVRAM_OOB_TABLE = ssd->NVRAM_OOB_TABLE;
    int BLOCK_NB = sc->BLOCK_NB;
	int max_invalid_entry_cnt = 0, victim_OOB_nb = -1, entry_nb = 0, valid_entry_nb = 0, i = 0;
	NVRAM_OOB_seg* curr_OOB_seg = (NVRAM_OOB_seg*)NVRAM_OOB_TABLE;
	int64_t NVRAM_OOB_rw_time;

	// 选择无效条目最多的segments
	for(i = 0; i < BLOCK_NB; i++)
	{
		if(curr_OOB_seg->invalid_entry > max_invalid_entry_cnt)
		{
			max_invalid_entry_cnt = curr_OOB_seg->invalid_entry;
			victim_OOB_nb = i;
		}	

		curr_OOB_seg += 1;
	}

	if(victim_OOB_nb == -1)
	{
		printf("ERROR[%s] No victim NVRAM segment\n", __FUNCTION__);
		return FAIL;
	}

	curr_OOB_seg = (NVRAM_OOB_seg*)NVRAM_OOB_TABLE + victim_OOB_nb;
	entry_nb = curr_OOB_seg->alloc_seg * OOB_ENTRY_PER_SEG - curr_OOB_seg->free_entry;
	valid_entry_nb = entry_nb - curr_OOB_seg->invalid_entry;

	UPDATE_NVRAM_OOB(ssd, victim_OOB_nb, 0);

	// 写有效条目
	for(i = 0; i < valid_entry_nb; i++)
	{
		if(curr_OOB_seg->free_entry == 0)
		{
			curr_OOB_seg->alloc_seg++;
			curr_OOB_seg->free_entry = OOB_ENTRY_PER_SEG;

			if(curr_OOB_seg->alloc_seg > ssd->stat_max_alloc_seg)
				ssd->stat_max_alloc_seg = curr_OOB_seg->alloc_seg;

			ssd->stat_total_alloc_seg++;
			ssd->stat_total_seg_bytes += OOB_ENTRY_PER_SEG * OOB_ENTRY_BYTES;
		}

		curr_OOB_seg->free_entry--;
		ssd->stat_total_OOB_entry++;
	}

// #ifdef STAT_COUNT
//     ssd->stat_type = 5;
//     stat_print(ssd);
// #endif
	
	NVRAM_OOB_rw_time = (int64_t)entry_nb * OOB_ENTRY_BYTES * NVRAM_READ_DELAY / 64 + (int64_t)valid_entry_nb * OOB_ENTRY_BYTES * NVRAM_WRITE_DELAY / 64;

	// 推迟NVRAM可获取时间点
    UPDATE_NVRAM_TS(ssd, victim_OOB_nb, NVRAM_OOB_rw_time);

	ssd->stat_NVRAMGC_print++;
	ssd->stat_NVRAMGC_delay_print += NVRAM_OOB_rw_time;
	ssd->stat_avg_NVRAMGC_delay = ssd->stat_NVRAMGC_delay_print / ssd->stat_NVRAMGC_print;

	return SUCCESS;
}

int UPDATE_NVRAM_OOB(struct ssdstate *ssd, unsigned int phy_block_nb, int valid)
{
	struct ssdconf *sc = &(ssd->ssdparams);
	void* NVRAM_OOB_TABLE = ssd->NVRAM_OOB_TABLE;
    int BLOCK_NB = sc->BLOCK_NB;
	int entry_cnt = 0, seg_bytes = 0;
	int count = 0;

	if(phy_block_nb >= BLOCK_NB){
		printf("ERROR[%s] Wrong physical address\n", __FUNCTION__);
		return FAIL;
	}

	NVRAM_OOB_seg* OOB_seg = (NVRAM_OOB_seg*)NVRAM_OOB_TABLE + phy_block_nb;

	if(valid == VALID)
	{
		if(OOB_seg->free_entry == 0)
		{
			OOB_seg->alloc_seg++;
			OOB_seg->free_entry = OOB_ENTRY_PER_SEG;

			if(OOB_seg->alloc_seg > ssd->stat_max_alloc_seg)
				ssd->stat_max_alloc_seg = OOB_seg->alloc_seg;

			ssd->stat_total_alloc_seg++;
			ssd->stat_total_seg_bytes += OOB_ENTRY_PER_SEG * OOB_ENTRY_BYTES;
		}

		OOB_seg->free_entry--;
		ssd->stat_total_OOB_entry++;

		if(ssd->stat_total_alloc_seg > TOTAL_OOB_SEG)
		{
			printf("[%s] ssd->stat_total_alloc_seg > TOTAL_OOB_SEG\n", __FUNCTION__);
		}
	}
	else if(valid == 0)
	{
		entry_cnt = OOB_seg->alloc_seg * OOB_ENTRY_PER_SEG - OOB_seg->free_entry;
		seg_bytes = OOB_seg->alloc_seg * OOB_ENTRY_PER_SEG * OOB_ENTRY_BYTES;
		
		if(OOB_seg->alloc_seg < ssd->stat_min_alloc_seg)
			ssd->stat_min_alloc_seg = OOB_seg->alloc_seg;

		ssd->stat_total_alloc_seg -= OOB_seg->alloc_seg;
		ssd->stat_total_OOB_entry -= entry_cnt;
		ssd->stat_total_invalid_entry -= OOB_seg->invalid_entry;
		ssd->stat_total_seg_bytes -= seg_bytes;

		OOB_seg->alloc_seg = 0;
		OOB_seg->free_entry = 0;
		OOB_seg->invalid_entry = 0;
	}
}

int INCREASE_INVALID_OOB_ENTRY_COUNT(struct ssdstate *ssd, unsigned int phy_block_nb)
{
	struct ssdconf *sc = &(ssd->ssdparams);
	void* NVRAM_OOB_TABLE = ssd->NVRAM_OOB_TABLE;
    int BLOCK_NB = sc->BLOCK_NB;

	if(phy_block_nb >= BLOCK_NB){
		printf("ERROR[%s] Wrong physical address\n", __FUNCTION__);
		return FAIL;
	}

	NVRAM_OOB_seg* OOB_seg = (NVRAM_OOB_seg*)NVRAM_OOB_TABLE + phy_block_nb;

	OOB_seg->invalid_entry++;
	ssd->stat_total_invalid_entry++;

	return SUCCESS;
} 

#if 0
void PRINT_VALID_ARRAY(unsigned int phy_flash_nb, unsigned int phy_block_nb)
{
	int i;
	int cnt = 0;
	block_state_entry* b_s_entry = GET_BLOCK_STATE_ENTRY(phy_flash_nb, phy_block_nb);

	printf("Type %d [%d][%d]valid array:\n", b_s_entry->type,  phy_flash_nb, phy_block_nb);
	for(i=0;i<PAGE_NB;i++){
		printf("%c ",b_s_entry->valid_array[i]);
		cnt++;
		if(cnt == 10){
			printf("\n");
		}
	}
	printf("\n");
}
#endif
