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
	int age;	
	int valid; 
	int modified; 
	uint32_t tag;
} cline;

typedef struct {
	cline *lines;
} cset;

typedef struct {
	int s;
	int E;
	int b;
	cset *sets;
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
	cache myCache;
	myCache.E = way;
	myCache.s = index_bit(set); //index bit
	myCache.b = index_bit(blocksize); //offset bit

	myCache.sets = (cset *)malloc(set*sizeof(cset));

	for(int i=0; i<set; i++){
		myCache.sets[i].lines = (cline *)malloc(way*sizeof(cline));
	}

	for(int i=0; i<set; i++){
		for(int j=0; j<way; j++){
			myCache.sets[i].lines[j].valid=0;
			myCache.sets[i].lines[j].age=0;
			myCache.sets[i].lines[j].tag=0;
			myCache.sets[i].lines[j].modified=0;
		}
	}

	return myCache;
}

void free_cache(cache *myCache, int set, int way){

}

void access_cache(cache *myCache, char *op, uint32_t addr,
 int *read, int *write, 
 int* writeback, int*readhit, int*writehit, int*readmiss, int*writemiss) {

	int oldest = 0;
	int victim = 0;
	int s = myCache->s; // index bit
	int b = myCache->b;// offset bit
	int way = myCache->E;
	uint32_t tag_from_data = addr >> (s+b);
	uint32_t index = addr;
	index = index >> b;
	index = index & 0xF;
		
	if(strcmp(op, "R")==0){				// when 'R'
		*read += 1;
		for(int j=0; j<way; j++){ // iterate

			if(myCache->sets[index].lines[j].valid == 1){  // if data is there, compare tag
				if(myCache->sets[index].lines[j].tag == tag_from_data){  // if same, hit. if not, skip
					*readhit += 1;
					myCache->sets[index].lines[j].age = time;
					return;
				}
			}
			if(myCache->sets[index].lines[j].valid == 0){ // seeking for empty cache space
				*readmiss += 1;
				myCache->sets[index].lines[j].valid = 1;
				myCache->sets[index].lines[j].age = time;
				myCache->sets[index].lines[j].tag = tag_from_data;
				return;
			}
			if(j==way-1){ 									// couldn't find any tag matched space nor empty space
				oldest = time - (myCache->sets[index].lines[0].age);
				for(int i=0; i<way; i++){
					if(oldest < time - (myCache->sets[index].lines[i].age)){
						oldest = time - (myCache->sets[index].lines[i].age);
						victim=i;												// victim selection
					}
				}
				*readmiss += 1;
				myCache->sets[index].lines[victim].valid = 1;    //victim eviction
				myCache->sets[index].lines[victim].age = time;
				myCache->sets[index].lines[victim].tag = tag_from_data;
				if(myCache->sets[index].lines[victim].modified==1){	//write back
					*writeback += 1;
					myCache->sets[index].lines[victim].modified = 0;
				}
				return;
			}
		}
	}
	else{			// when 'W'
		*write += 1;
		for(int j=0; j<way; j++){ // iterate

			if(myCache->sets[index].lines[j].valid == 1){ // if data is there, compare tag
				if(myCache->sets[index].lines[j].tag == tag_from_data){ // if same, hit. if not, skip
					*writehit += 1;
					myCache->sets[index].lines[j].modified=1;
					myCache->sets[index].lines[j].age = time;
					return;
				}
			}
			if(myCache->sets[index].lines[j].valid == 0){ // seeking for empty cache space
				*writemiss += 1;
				myCache->sets[index].lines[j].valid = 1;
				myCache->sets[index].lines[j].age = time;
				myCache->sets[index].lines[j].tag = tag_from_data;
				myCache->sets[index].lines[j].modified=1;
				return;
			}
			if(j==way-1){ 								// couldn't find any tag matched space nor empty space
				oldest = time - (myCache->sets[index].lines[0].age);
				for(int i=0; i<way; i++){
					if(oldest < time - (myCache->sets[index].lines[i].age)){
						oldest = time - (myCache->sets[index].lines[i].age);
						victim=i;												// victim selection
					}
				}
				*writemiss += 1;													//victim eviction
				myCache->sets[index].lines[victim].valid = 1;
				myCache->sets[index].lines[victim].age = time;
				myCache->sets[index].lines[victim].tag = tag_from_data;
				if(myCache->sets[index].lines[victim].modified==1){				//write back
					*writeback += 1;
				}
				myCache->sets[index].lines[victim].modified = 1;

				return;
			}
		}
	}
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
			if(L->sets[i].lines[j].valid){
				line = L->sets[i].lines[j].tag << (s+b);
				line = line|(i << b); 
			}
			else{		
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
	set = capacity/way/blocksize;

    /* TODO: Define a cache based on the struct declaration */
    simCache = build_cache(set, capacity, way, blocksize);

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
        time++;
        access_cache(&simCache, op, addr, &read, &write, &writeback, &readhit, &writehit, &readmiss, &writemiss);
    }

    // test example
	sdump(read, write, writeback, readhit, writehit, readmiss, writemiss);
	if (xflag){
	    	xdump(&simCache);
	}

	// free_cache(&simCache, set ,way);

    return 0;
}
