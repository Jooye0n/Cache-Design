# Project 3. MIPS Data Cache Simulator
Skeleton developed by CMU, modified for AJOU SCE212

## Instructions
All student is required to implement functions below. 
1. __build_cache()__  
    A role of this function is to allocate memory for cache structure. 
    Student must use the declared cache structure and arguments from the command line. 
    All variables in the cache structure should be initialized and used in a proper way.

2. __access_cache()__  
    If a student lets a system allocate the memory of cache structures, 
    the next step would be reading a file and applying address lines in the file to 
    the cache structures. Given those addresses, the cache which was implemented should be 
    used and contains proper addresses in the cache line. 

    
    >Note that in this project cache does not have any data so the only thing that student concerns is the 
    proper address value.

    For a write policy, there are two write policies in the cache: 
    write-through and write-back. In this project, for simplicity, 
    write-back policy is used and only counted by a variable. Hence, students 
    should define a proper count variable to get the same result with the files in  `sample_output` directory.

    During cache miss, whenever existing cache entry is evicted due to the lack of room,
    there should be an appropriate replacement policy. 
    In the project, we use "Least Replacement Unit" (LRU). Thus, student should be 
    aware of it to make the required output result.
    
* __Sample files__  
Each sample file in `sample_input` directory has two columns. The first column stands for 
a type of operations such as "R" for Read and "W" for Write. The second column is an address value.
So, each student should interpret properly and use them for their own function.

## ETC
The functions above can be modified if the application produces same output in the answer.
So, feel free to change and add any functions if student desires. If you have any question, 
please post your question in the Ajou BB.
