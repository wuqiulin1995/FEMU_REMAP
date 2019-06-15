/* config options */
/* 2^FILTER_SIZE is the size of the filter in bits, i.e.,
 * size 11 = 2^11 bits = 2048 bits */
//#define FILTER_SIZE 14
#define NUM_HASHES 2
#define WORD_BUF_SIZE 4  // int -> 4 chars

//#define FILTER_SIZE_BYTES (1 << (FILTER_SIZE - 3))
#define FILTER_BITMASK ((1 << FILTER_SIZE) - 1)

 //Multiple bloom filters
//#define FILTER_NUMBER 4
//number of requests as decay interval
#define DACAY 4096

// FILTER_NUMBER == 8
//#define FILTER_NUMBER 8
//#define DACAY 256

// hot LPNs, data buffer
//512MB
#define PAGE_NUMBER 131072
#define BUFFER_LIMIT 32        //128KB (For a 1TB SSD, buffer would be 256MB)
//16GB
//#define PAGE_NUMBER 4194304
//#define BUFFER_LIMIT 1024    //4MB (For a 1TB SSD, buffer would be 256MB)
#define NOT_CACHE 0
#define IS_CACHE 1
#define HOT 0
#define COLD 1
#define HIT_BF 1
#define MISS_BF 0

//add by zy
#define ROT32(x, y) ((x << y) | (x >> (32 - y))) // avoid effor

//1: record the number of hot LPNs in real time by writing back to output file.
#define RECORD 0

//float hot_threshold = 4;     // FILTER_NUMBER == 4
//const float hot_threshold = 5;   // FILTER_NUMBER == 8
//-----------------------------------------------------------------------------


//data buffer
// struct lpn_node {
// 	unsigned int lpn;
// 	unsigned int ino;
// 	unsigned int offset;
// 	unsigned int type;
// 	unsigned int temp;
// 	struct lpn_node * pre;
// 	struct lpn_node * next;
// } *head, *tail;
// unsigned int node_number;


// hot LPNs
// struct lpn_status {
// 	//int cache_status;   //0: not in the cache, 1: in the cache
// 	int hot_status;     //0: cold, 1: hot
// 	float weight;
// } status[PAGE_NUMBER];



//char filename[] = {"D:\\zy\\Flashsim\\MSR\\write_lpn\\web0_16GB_lpn.flashsim"};
//char filename[] = {"words.txt"};
//char filename[] = {"C:\\Users\\zy_sp3\\Desktop\\f1_lpn.flashsim"};

//record the hot LPNs in the end
//char outputname[] = {"hot1.output"};
//record the evict LPNs
//char output_evict_hot[] = {"evict_hot.output"};



int64_t  Bloom_filter(struct ssdstate *ssd, int64_t ino, int64_t off);





//-----------------------------------------------------------------------------
/* hash functions */
unsigned int RSHash(unsigned char *, unsigned int);
unsigned int DJBHash(unsigned char *, unsigned int);
unsigned int FNVHash(unsigned char *, unsigned int);     //currently used
unsigned int JSHash(unsigned char *, unsigned int);
unsigned int PJWHash(unsigned char *, unsigned int);
unsigned int SDBMHash(unsigned char *, unsigned int);
unsigned int DEKHash(unsigned char *, unsigned int);
unsigned int MURMURHash(unsigned char *, unsigned int);  //currently used

/* helper functions */
void get_hashes(unsigned int hash[], unsigned char in1[], unsigned char in2[]);
void int_to_char(unsigned char[], unsigned int);
float check_hot(struct ssdstate *ssd, unsigned char[][FILTER_SIZE_BYTES], unsigned int[]);
void insert_word(unsigned char[], unsigned int[]);
int in_dict(unsigned char[], unsigned int[]);



