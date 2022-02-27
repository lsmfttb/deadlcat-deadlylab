#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

typedef struct 
{
    int valid;
    int timestamp;
    unsigned long tag;

}Simulator;


void work(FILE* pFile, int s, int E, int b, char* t, int tlen, int v, int* hits, int* misses, int* evictions)
{
    char identifier;
    unsigned long address, tag, index;
    int size;
    const int array_size = E * (1 << s);
    Simulator* arr = (Simulator*)malloc(sizeof(Simulator) * array_size);
    for (int i = 0; i < array_size; ++i)
    {
        arr[i].valid = 0;
        arr[i].tag = 0;
        arr[i].timestamp = 0;
    }
    int counter = 0;
    int flag;
    while (fscanf(pFile, " %c %lx,%d", &identifier, &address, &size) > 0)
    {
        if (identifier == 'I') continue;
        tag = address >> (s + b);
        index = (address >> b) & ((1 << s) - 1);
        ++counter;
        flag = 0;
        for (int i = 0; i < E; ++i)
        {
            int m = E * index + i;
            if (arr[m].valid && arr[m].tag == tag)
            {
                flag = 1;
                arr[m].timestamp = counter;
                ++*hits;
                break;
            }
        }
        if (!flag)
        {
            int LRUindex = -1;
            int LRUvalue = 0x7fffffff;
            for (int i = 0; i < E; ++i)
            {
                int m = E * index + i;
                int v = arr[m].valid ? arr[m].timestamp : -1;
                if (LRUindex == -1 || v < LRUvalue)
                {
                    LRUindex = m;
                    LRUvalue = v;
                }
            }
            arr[LRUindex].valid = 1;
            arr[LRUindex].tag = tag;
            arr[LRUindex].timestamp = counter;
            ++*misses;
            if (LRUvalue != -1)
            {
                ++*evictions;
                flag = -1;
            }
        }
        if (identifier == 'M') ++*hits;
        if (v)
        {
            printf("%c %lx,%d ", identifier, address, size);
            if (flag == 1)
            {
                printf("hit ");
            }
            else if (flag == 0)
            {
                printf("miss ");
            }
            else
            {
                printf("miss eviction ");
            }
            if (identifier == 'M')
            {
                printf("hit ");
            }
            printf("\n");
        }
    }
    free(arr);
}

int main(int argc, char* argv[])
{
    int opt;
    int s = 0, E = 0, b = 0, v = 0, h = 0;
    char t[256] = { 0 };
    char* tp;
    int tlen = 0;
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            s = atoi(optarg);
            break;
        case 'E':
            E = atoi(optarg);
            break;
        case 'b':
            b = atoi(optarg);
            break;
        case 't':
            tp = optarg;
            tlen = strlen(optarg);
            strncpy(t, optarg, 255);
            break;
        case 'v':
            v = 1;
            break;
        case 'h':
            h = 1;
            break;
        default:
            break;
        }
    }
    if (h)
    {
        printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n"
        "  -h         Print this help message.\n  -v         Optional verbose flag.\n"
        "  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n"
        "  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\nExamples:\n"
        "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
        "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0], argv[0], argv[0]);
        exit(0);
    }
    if (!(s && E && b && tlen))
    {
        printf("%s: Missing required command line argument\n", argv[0]);
        printf("Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\nOptions:\n"
        "  -h         Print this help message.\n  -v         Optional verbose flag.\n"
        "  -s <num>   Number of set index bits.\n  -E <num>   Number of lines per set.\n"
        "  -b <num>   Number of block offset bits.\n  -t <file>  Trace file.\nExamples:\n"
        "  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n"
        "  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n", argv[0], argv[0], argv[0]);
        exit(0);
    }
    if (tlen >= 256)
    {
        printf("%s: File name too long\n", tp);
        exit(0);
    }
    FILE * pFile = fopen(t, "r");
    if (!pFile)
    {
        printf("%s: No such file or directory\n", t);
        exit(0);
    }
    int hits = 0, misses = 0, evictions = 0;
    work(pFile, s, E, b, t, tlen, v, &hits, &misses, &evictions);
    printSummary(hits, misses, evictions);
    return 0;
}
