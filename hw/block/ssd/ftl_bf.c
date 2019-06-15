#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include "vssim_config_manager.h"

#include "common.h"

#include "ftl_bf.h"

//--------------------------------------------------------------------------
/*
布隆过滤器的冷热效果不仅与布隆过滤器的size,decay,热阈值有关，而且与盘的大小也有关系，具体就是指页的大小，super block的大小。
*/



int64_t  Bloom_filter(struct ssdstate *ssd, int64_t ino, int64_t off)
{

	unsigned int next_filter;
	unsigned int hash[NUM_HASHES];                                //hash[2]
	unsigned char buf1[WORD_BUF_SIZE];                             //buf[4]
	unsigned char buf2[WORD_BUF_SIZE];                             //buf[4]

	//int current_filter, current_decay_filter;
	int i, j;
	float evict_weight;
	int sector_size;
	unsigned int hash_ino, hash_off;
	//unsigned int max_ino, max_off;
	float max_weight;
	float hot_threshold = 3;
	int64_t bloom_temp = 1;


	hash_ino = hash_off = 0;
	//初始化filter二维数组


	//current_filter = 0;
   // current_decay_filter = -1;


	max_weight = 0.00;

	//sector_size = dis_sub_reqs.sub_req_state;

	hash_ino = ino;
	hash_off = off;

	int_to_char(buf1, hash_ino);
	int_to_char(buf2, hash_off);
	get_hashes(hash, buf1, buf2);
	evict_weight = check_hot(ssd, ssd->filter, hash);
	if (evict_weight > max_weight)
	{
		max_weight = evict_weight;
	}

	if (max_weight >= hot_threshold)
	{
		bloom_temp = HOT;
		//hot_count++;
	}
	else
	{
		bloom_temp = COLD;
		//cold_count++;
	}


	//22222222222222222222222222222222222222222222222222222222222222222
	//decay the LRU filter (After data buffer process, reset the weight of filters and the LRU filter)
	if (ssd->data_buffer_counter % DACAY == 1)
	{
		if (ssd->current_decay_filter == -1)
			ssd->current_decay_filter = 0;

		//reset current decay filter
		for (j = 0; j < FILTER_SIZE_BYTES; j++)
			ssd->filter[ssd->current_decay_filter][j] = 0;

		for (j = 0; j < FILTER_NUMBER; j++)
			ssd->filter_weight[(FILTER_NUMBER + ssd->current_filter - j) % FILTER_NUMBER] = 2.0 - j * (2.0 / FILTER_NUMBER);

		ssd->current_decay_filter = (FILTER_NUMBER + ssd->current_decay_filter + 1) % FILTER_NUMBER;
	}

	//3333333333333333333333333333333333333333333333333333333333333333333
	//handle current lpn in BFs

	hash_ino = ino;
	hash_off = off;

	int_to_char(buf1, hash_ino);
	int_to_char(buf2, hash_off);
	get_hashes(hash, buf1, buf2);
	//check if current lpn hit in BFs
	if (in_dict(ssd->filter[ssd->current_filter], hash))   // hit current BF
	{
		i = 0;
		do
		{
			i++;
			next_filter = (ssd->current_filter + i) % FILTER_NUMBER;
			if (i > FILTER_NUMBER - 1)
				break;
		} while (in_dict(ssd->filter[next_filter], hash));

		if (i < FILTER_NUMBER) //insert current lpn to current BF
			insert_word(ssd->filter[next_filter], hash);
	}
	else //insert current lpn to current BF
		insert_word(ssd->filter[ssd->current_filter], hash);

	ssd->current_filter++;
	if (ssd->current_filter > FILTER_NUMBER - 1)
		ssd->current_filter = 0;

	return bloom_temp;
}

//------------------------------------------------------------------------------


//-----------------------------------------------------------------------------

void get_hashes(unsigned int hash[], unsigned char in1[], unsigned char in2[])
{
	unsigned char *str1 = (unsigned char *)in1;
	unsigned char *str2 = (unsigned char *)in2;

	int pos = 4;
	//int i;

/*
	hash[0] = RSHash  (str, pos);
	hash[1] = DJBHash (str, pos);
	hash[2] = FNVHash (str, pos);
	hash[3] = JSHash  (str, pos);
	hash[4] = PJWHash (str, pos);
	hash[5] = SDBMHash(str, pos);
	hash[6] = DEKHash (str, pos);
	hash[7] = MURMURHash (str, pos);
*/
	hash[0] = FNVHash(str1, pos);
	hash[1] = MURMURHash(str2, pos);
	//	hash[2] = RSHash  (str, pos);
	//    hash[3] = DJBHash (str, pos);
	//	hash[4] = JSHash  (str, pos);
	//	hash[5] = PJWHash (str, pos);
	//	hash[6] = SDBMHash(str, pos);
	//	hash[7] = DEKHash (str, pos);


		//add by zy
		//printf("zy: \n----------In function %s:----------------\n", __FUNCTION__);
		//printf("zy: String: %s\n", str);
		//for(i=0; i < NUM_HASHES; i++)
		//     printf("zy: Num %d hash function: %x.\n", i, hash[i]);
}

//add by zy
//transform lpn (int) into chars, which are necessary for hash functions
void int_to_char(unsigned char buf[], unsigned int lpn)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		buf[i] = (unsigned char)lpn;
		lpn = lpn >> 8;
	}
	return;
}

//add by zy
//check in BFs
float check_hot(struct ssdstate *ssd, unsigned char filter[][FILTER_SIZE_BYTES], unsigned int hash[])
{
	float sum_weight = 0;
	int i;

	for (i = 0; i < FILTER_NUMBER; i++) {
		if (in_dict(filter[i], hash))   // hit current BF
			sum_weight += ssd->filter_weight[i];
	}

	return sum_weight;
}

void insert_word(unsigned char filter[], unsigned int hash[])
{
	int i;

	for (i = 0; i < NUM_HASHES; i++)
	{
		/* xor-fold the hash into FILTER_SIZE bits */
		//hash[i] = (hash[i] >> 21) ^ (hash[i] & FILTER_BITMASK);
		//zy: fold the hash into FILTER_SIZE bits
		hash[i] %= (1 << FILTER_SIZE);
		/* set the bit in the filter */
		filter[hash[i] >> 3] |= 1 << (hash[i] & 7);
	}

	return;
}

int in_dict(unsigned char filter[], unsigned int hash[])
{
	int i;

	for (i = 0; i < NUM_HASHES; i++)
	{
		//hash[i] = (hash[i] >> 21) ^ (hash[i] & FILTER_BITMASK);
		//zy: fold the hash into FILTER_SIZE bits
		hash[i] %= (1 << FILTER_SIZE);
		if (!(filter[hash[i] >> 3] & (1 << (hash[i] & 7))))
		{
			return MISS_BF;
		}
	}

	return HIT_BF;
}

//=============================================================================
/****************\
| Hash Functions |
\****************/

unsigned int RSHash(unsigned char *str, unsigned int len)
{
	unsigned int b = 378551;
	unsigned int a = 63689;
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = hash * a + (*str);
		a = a * b;
	}

	return hash;
}

unsigned int JSHash(unsigned char *str, unsigned int len)
{
	unsigned int hash = 1315423911;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash ^= ((hash << 5) + (*str) + (hash >> 2));
	}

	return hash;
}

unsigned int PJWHash(unsigned char *str, unsigned int len)
{
	const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8);
	const unsigned int ThreeQuarters = (unsigned int)((BitsInUnsignedInt * 3) / 4);
	const unsigned int OneEighth = (unsigned int)(BitsInUnsignedInt / 8);
	const unsigned int HighBits = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
	unsigned int hash = 0;
	unsigned int test = 0;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = (hash << OneEighth) + (*str);

		if ((test = hash & HighBits) != 0)
		{
			hash = ((hash ^ (test >> ThreeQuarters)) & (~HighBits));
		}
	}

	return hash;
}

unsigned int SDBMHash(unsigned char *str, unsigned int len)
{
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = (*str) + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

unsigned int DJBHash(unsigned char *str, unsigned int len)
{
	unsigned int hash = 5381;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = ((hash << 5) + hash) + (*str);
	}

	return hash;
}

unsigned int DEKHash(unsigned char *str, unsigned int len)
{
	unsigned int hash = len;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash = ((hash << 5) ^ (hash >> 27)) ^ (*str);
	}
	return hash;
}

unsigned int FNVHash(unsigned char *str, unsigned int len)
{
	const unsigned int fnv_prime = 0x811C9DC5;
	unsigned int hash = 0;
	unsigned int i = 0;

	for (i = 0; i < len; str++, i++)
	{
		hash *= fnv_prime;
		hash ^= (*str);
	}

	return hash;
}

//add by zy
unsigned int MURMURHash(unsigned char *key, unsigned int len)
{
	static const unsigned int c1 = 0xcc9e2d51;
	static const unsigned int c2 = 0x1b873593;
	static const unsigned int r1 = 15;
	static const unsigned int r2 = 13;
	static const unsigned int m = 5;
	static const unsigned int n = 0xe6546b64;

	unsigned int hash = 0;

	const int nblocks = len / 4;
	const unsigned int *blocks = (const unsigned int *)key;
	const unsigned char *tail = (const unsigned char *)(key + nblocks * 4);
	unsigned int k1 = 0;
	int i;
	unsigned int k;
	for (i = 0; i < nblocks; i++)
	{
		k = blocks[i];
		k *= c1;
		k = ROT32(k, r1);
		k *= c2;

		hash ^= k;
		hash = ROT32(hash, r2) * m + n;
	}

	switch (len & 3)
	{
	case 3:
		k1 ^= tail[2] << 16;
	case 2:
		k1 ^= tail[1] << 8;
	case 1:
		k1 ^= tail[0];

		k1 *= c1;
		k1 = ROT32(k1, r1);
		k1 *= c2;
		hash ^= k1;
	}

	hash ^= len;
	hash ^= (hash >> 16);
	hash *= 0x85ebca6b;
	hash ^= (hash >> 13);
	hash *= 0xc2b2ae35;
	hash ^= (hash >> 16);

	return hash;
}

