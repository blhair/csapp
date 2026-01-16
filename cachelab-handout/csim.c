#include "cachelab.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>  // getopt 在这里
struct cache_line {
    bool valid;
    unsigned long tag;
    int lru_counter;
};
struct cache_set {
    struct cache_line* lines;
};
struct cache {
    struct cache_set* sets;
    int s; // number of set index bits
    int E; // number of lines per set
    int b; // number of block offset bits
} Cache = {NULL, 0, 0, 0};

int global_counter = 0;

void cache_init(int s, int E, int b);
void cache_free();
int cache_access(unsigned long address);

/* 
 * main - The main function for cache simulator
 */

int main(int argc, char *argv[]) {
    char *trace_file = NULL;
    int verbose = 0;         // 是否开启 verbose 模式
    int size, E, bsize;
    
    char ch; // 这里的 ch 类型也可以是 int

    // 循环调用 getopt，直到返回 -1 表示解析完毕
    // "s:E:b:t:hv" 告诉它我们期待哪些参数
    while ((ch = getopt(argc, argv, "s:E:b:t:hv")) != -1) {
        switch (ch) {
            case 's':
                // optarg 是一个全局指针，指向当前选项后的参数值（比如 "4"）
                // atoi 把字符串转成整数
                size = atoi(optarg); 
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                bsize = atoi(optarg);
                break;
            case 't':
                // 文件名是字符串，直接保存指针
                trace_file = optarg;
                break;
            case 'v':
                verbose = 1; // 这是一个开关
                break;
            case 'h':
                // 这里调用你的 printUsage() 函数，打印帮助信息
                // printUsage();
                exit(0);
            default:
                // 如果用户输入了奇怪的参数（比如 -z），getopt 返回 '?'
                // printUsage();
                exit(1);
        }
    }
    cache_init(size, E, bsize);
    if (trace_file == NULL) {
        fprintf(stderr, "Trace file is required\n");
        exit(1);
    }
    FILE *fp = fopen(trace_file, "r");
    if (fp == NULL) {
        fprintf(stderr, "Could not open trace file %s\n", trace_file);
        exit(1);
    }
    char operation;
    unsigned long address;
    int size_access;
    int hits = 0, misses = 0, evictions = 0;
    while (fscanf(fp, " %c %lx,%d", &operation, &address, &size_access) == 3) {
        if (operation == 'I') {
            continue; // 忽略指令访问
        }
        
        // Call cache_access just once for L, S, or first part of M
        int result = cache_access(address);
        
        if (verbose) {
            printf("%c %lx,%d ", operation, address, size_access);
        }

        if (result == 1) {
            hits++;
            if (verbose) printf("hit ");
        } else if (result == 0) {
            misses++;
            if (verbose) printf("miss ");
        } else if (result == -1) {
            misses++;
            evictions++;
            if (verbose) printf("miss eviction ");
        }

        if (operation == 'M') {
            hits++; // Second access is always partial hit
            if (verbose) printf("hit ");
            // Update LRU again for the second access
            cache_access(address); 
        }
        
        if (verbose) printf("\n");
    }
    fclose(fp);
    cache_free();


    printSummary(hits, misses, evictions);

    return 0;
}

/* 
 * cache_init - Initialize the cache with given s, E, b values
 */
void cache_init(int s, int E, int b) {
    int S = 1 << s; // 计算总的集合数 S = 2^s
    Cache.sets = (struct cache_set*)malloc(S * sizeof(struct cache_set));
    for (int i = 0; i < S; i++) {
        Cache.sets[i].lines = (struct cache_line*)malloc(E * sizeof(struct cache_line));
        for (int j = 0; j < E; j++) {
            Cache.sets[i].lines[j].valid = false;
            Cache.sets[i].lines[j].tag = 0;
            Cache.sets[i].lines[j].lru_counter = 0;
        }
    }
    Cache.s = s;
    Cache.E = E;
    Cache.b = b;
}

/*
 * cache_free - Free the allocated memory for the cache
 */
void cache_free() {
    int S = 1 << Cache.s; // 计算总的集合数 S = 2^s
    for (int i = 0; i < S; i++) {
        free(Cache.sets[i].lines);
    }
    free(Cache.sets);
    Cache.sets = NULL;
}
/*
 * cache_access - Simulate accessing the cache with a given address
 * Returns 1 for hit, 0 for miss, and -1 for eviction
 */
int cache_access(unsigned long address) {
    unsigned long set_index = (address >> Cache.b) & ((1 << Cache.s) - 1);
    unsigned long tag = address >> (Cache.s + Cache.b);
    struct cache_set* set = &Cache.sets[set_index];

    // Check for hit
    for (int i = 0; i < Cache.E; i++) {
        if (set->lines[i].valid && set->lines[i].tag == tag) {
            // Hit: Update LRU counters
            global_counter++;
            set->lines[i].lru_counter = global_counter;
            return 1; // Hit
        }
    }

    // Miss: Find an empty line or evict the LRU line
    int lru_index = 0;
    int min_lru = 2147483647;
    for (int i = 0; i < Cache.E; i++) {
        if (!set->lines[i].valid) {
            // Empty line found
            set->lines[i].valid = true;
            set->lines[i].tag = tag;
            global_counter++;
            set->lines[i].lru_counter = global_counter;
            return 0; // Miss
        }
        if (set->lines[i].lru_counter < min_lru) {
            min_lru = set->lines[i].lru_counter;
            lru_index = i;
        }
    }

    // Evict the LRU line
    set->lines[lru_index].tag = tag;
    global_counter++;
    set->lines[lru_index].lru_counter = global_counter;
    return -1; // Eviction
}