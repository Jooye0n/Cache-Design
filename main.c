#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for getopt()

#define BYTES_PER_WORD 4
// #define DEBUG

/*
 * Cache structures
 */
int time = 0;

typedef struct {
	int age;	//cache->sets[i].lines[j].age   --> for LRU
	int valid;  //cache->sets[i].lines[j].valid
	int modified; // cache->sets[i].lines[j].modified
	uint32_t tag; // cache->sets[i].lines[j].tag
} cline;

typedef struct {
	cline *lines; // cache->sets[i].lines[j]
} cset;

typedef struct {
	int s; // set 비트 수?
	int E; // way 비트 수
	int b; // block size?
	cset *sets; // cache->sets[i]
} cache;

static int index_bit(int n) {
	int cnt = 0;
	while(n){
		cnt++;
		n = n >> 1;
	}
	return cnt-1;
}

cache build_cache(int set, int capacity, int way, int blocksize) {
	// malloc for cache
	// cache[set][assoc][word per block]
	//32비트가 워드 하나로 고정임 항상
	cache myCache;
	myCache.E = index_bit(way); //
	myCache.s = index_bit(set); //index bit
	myCache.b = index_bit(blocksize); //offset bit

	myCache.sets = (cset *)malloc(set*sizeof(cset));

	for(int i=0; i<set; i++){
		myCache.sets[i].lines = (cline *)malloc(way*sizeof(cline));
	}

	// Cache.sets[0].lines[0].tag = 0x00000002; 이런식의 assign 가능
	
	return myCache;
}

void access_cache() {

}

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    	printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                           */
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    	printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */
/* 					                            		       */
/* Cache Design						                           */
/*  							                               */
/* 	    cache[set][assoc][word per block]		               */
/*                                						       */
/*      				                        		       */
/*       ----------------------------------------	           */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*                              						       */
/*                                                             */
/***************************************************************/
void xdump(cache* L)
{
	int i,j,k = 0;
	int b = L->b, s = L->s;
	int way = L->E, set = 1 << s;
	int E = index_bit(way);

	uint32_t line;

	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			if(L->sets[i].lines[j].valid){	// 캐시에 있으면 (valid == 1) 이면, 블락의 베이스 주소로 변환
				line = L->sets[i].lines[j].tag << (s+b);     // 
				line = line|(i << b); // b-> log2(block size)   --> 뒷 자리 바꿔줌
			}
			else{					// 캐시 비어있으면 0x0000 0000 출력
				line = 0;
			}
			printf("0x%08x  ", line);
		}
		printf("\n");
	}
	printf("\n");
}




int main(int argc, char *argv[]) {
	int i, j, k;
	int capacity=1024;
	int way=8;
	int blocksize=8;
	int set;

	//cache
	cache simCache;

	// counts
	int read=0, write=0, writeback=0;
	int readhit=0, writehit=0;
	int readmiss=0, writemiss = 0;

	// Input option
	int opt = 0;
	char* token;
	int xflag = 0;

	// parse file
	char *trace_name = (char*)malloc(32);
	FILE *fp;
    char line[16];
    char *op;
    uint32_t addr;

    /* You can define any variables that you want */

	trace_name = argv[argc-1];
	if (argc < 3) {
		printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
		exit(1);
	}
	while((opt = getopt(argc, argv, "c:x")) != -1){
		switch(opt){
			case 'c':
                // extern char *optarg;
				token = strtok(optarg, ":");
				capacity = atoi(token);
				token = strtok(NULL, ":");
				way = atoi(token);
				token = strtok(NULL, ":");
				blocksize  = atoi(token);
				break;
			case 'x':
				xflag = 1;
				break;
			default:
			printf("Usage: %s -c cap:assoc:block_size [-x] input_trace \n",argv[0]);
			exit(1);

		}
	}

	// allocate
	set = capacity/way/blocksize;   // index

    /* TODO: Define a cache based on the struct declaration */
    simCache = build_cache(set, capacity, way, blocksize);

    //printf("tag: 0x%x\n", simCache.sets[0].lines[0].tag);  이런식 출력 가능

	// simulate
	fp = fopen(trace_name, "r"); // read trace file
	if(fp == NULL){
		printf("\nInvalid trace file: %s\n", trace_name);
		return 1;
	}
	cdump(capacity, way, blocksize);

    /* TODO: Build an access function to load and store data from the file */
    while (fgets(line, sizeof(line), fp) != NULL) {
        op = strtok(line, " ");
        addr = strtoull(strtok(NULL, ","), NULL, 16);

#ifdef DEBUG
        // You can use #define DEBUG above for seeing traces of the file.
        fprintf(stderr, "op: %s\n", op);
        fprintf(stderr, "addr: %x\n", addr);
#endif
        // ...
        // access_cache()
        // ...
    }

    // test example
	sdump(read, write, writeback, readhit, writehit, readmiss, writemiss);
	if (xflag){
	    	xdump(&simCache);
	}

    	return 0;
}
