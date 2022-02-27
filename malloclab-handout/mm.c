/*
 * mm.c - Malloc package using explicit free list.
 * structure of allocated block:
 *    size 001(the last 3 bits of size; same below)
 *    payload and padding
 *    size 001
 * 
 * structure of free block:
 *    size 000
 *    next free block
 *    prev free block
 *    ...
 *    size 000
 * 
 * minimum block size = 32 bytes (4 words)
 * 
 * address-ordered policy: free list blocks are always in address order
 * segregated fit is applied: 10 free lists maintaining blocks with available words {2}, {3 ~ 4}, {5 ~ 8}, ..., {256 ~ 512}, {513 ~ inf}
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "deadlycat's team",
    /* First member's full name */
    "deadlycat",
    /* First member's email address */
    "deadlycat@is.best",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* useful macros in the textbook */
#pragma region 

/* word and header/footer size (bytes) */
//#define WSIZE 8
#define WSIZE sizeof(long)

/* extend heap by this amount (bytes) */
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* read and write a word at address p */
#define GET(p) (*(unsigned long*)(p))
#define PUT(p, val) (*(unsigned long*)(p) = (val))

/* read the size and allocated fieldsds from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp) - WSIZE)
#define FTRP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)) - (WSIZE << 1))

/* given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char*)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

#pragma endregion

/* Minimum block size (apart from prologue and epilogue) is 4 words. */
#define MIN_BLOCK_SIZE (WSIZE << 2)

/* Number of free lists with different block size; should be even (for alignment) */
#define LNUM 20

/* ptr to element in free list */
typedef long unsigned *freelistp;
#define LNEXT(fp) ((freelistp)*(freelistp)(fp))
#define LPREV(fp) ((freelistp)*((freelistp)(fp) + 1))
#define PUT_LNEXT(fp, val) (*(freelistp)(fp) = (long unsigned)val)
#define PUT_LPREV(fp, val) (*((freelistp)(fp) + 1) = (long unsigned)val)

static freelistp* arr_root;
static void* prologue;
static void* epilogue;

static int get_index(size_t size);
static freelistp extend_heap(size_t size);
static void add_list(void* bp);
static void remove_list(void* bp);
static void mm_check();

/* given size of block (aligned, >= minimum size), return index of free list which the block should be in */
static int get_index(size_t size)
{
    size_t words = (size - 2 * WSIZE) / WSIZE;
    int index = -1;
    --words;
    for (int i = 0; i < LNUM && words; ++i)
    {
        words >>= 1;
        ++index;
    }
    if (index < 0)
    {
        printf("get_index error\n");
        exit(1);
    }
    return index;
}


/* extend heap and create an aligned free block; may coalesce with previous free block */
static freelistp extend_heap(size_t size)
{
    char* bp;
    size_t newsize = size < MIN_BLOCK_SIZE ? MIN_BLOCK_SIZE : ALIGN(size);

    /* allocate */
    if ((bp = mem_sbrk(newsize)) == (void*)-1)
        return NULL;
    
    /* initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(newsize, 0));
    PUT(FTRP(bp), PACK(newsize, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    epilogue = HDRP(NEXT_BLKP(bp));

    /* try to coalesce with previous block */
    freelistp p_prev = (freelistp)PREV_BLKP(bp);
    int alloc_prev = GET_ALLOC(HDRP(p_prev));
    if (alloc_prev == 0)
    {
        remove_list(p_prev);
        newsize += GET_SIZE(HDRP(p_prev));
        PUT(HDRP(p_prev), PACK(newsize, 0));
        PUT(FTRP(p_prev), PACK(newsize, 0));
        add_list(p_prev);
        return p_prev;
    }
    else
    {
        add_list(bp);
        return (freelistp)bp;
    }
}

/* add block into proper free list; need block size to be initialized */
static void add_list(void* bp)
{
    /* find the list which block will be added into */
    size_t size = GET_SIZE(HDRP(bp));
    int index = get_index(size);

    /* find correct position and add */
    freelistp pos = arr_root[index];
    if (pos == NULL)
    {
        PUT_LNEXT(&arr_root[index], bp);
        PUT_LPREV(bp, &arr_root[index]);
        PUT_LNEXT(bp, NULL);
    }
    else
    {
        while (LNEXT(pos) != NULL && LNEXT(pos) < (freelistp)bp)
            pos = LNEXT(pos);
        PUT_LPREV(bp, pos);
        PUT_LNEXT(bp, LNEXT(pos));
        if (LNEXT(pos) != NULL)
            PUT_LPREV(LNEXT(pos), bp);
        PUT_LNEXT(pos, bp);
    }
}

static void remove_list(void* bp)
{
    freelistp next = LNEXT(bp);
    freelistp prev = LPREV(bp);
    PUT_LNEXT(prev, next);
    if (next != NULL)
        PUT_LPREV(next, prev);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    char* p;

    /* create the initial empty heap */
    if ((p = mem_sbrk((LNUM + 4) * WSIZE)) == (void*)-1)
        return -1;
    for (int i = 0; i <= LNUM; ++i)
        PUT(p + i * WSIZE, 0);
    PUT(p + (LNUM + 1) * WSIZE, PACK(WSIZE << 1, 1));   /* prologue header */
    PUT(p + (LNUM + 2) * WSIZE, PACK(WSIZE << 1, 1));   /* prologue footer */
    PUT(p + (LNUM + 3) * WSIZE, PACK(0, 1));            /* epilogue header */
    arr_root = (freelistp*)p;
    prologue = p + (LNUM + 1) * WSIZE;
    epilogue = p + (LNUM + 3) * WSIZE;

    // /* extend the empty heap with a free block of CHUNKSIZE bytes */
    // if (extend_heap(CHUNKSIZE) == NULL)
    //     return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by finding suitable free block in free list.
 *     Remove the block from free list; if size of remaining part >= minimum block size, initialize it and add it to free list.
 *     Always allocate a block whose size is a multiple of the alignment.
 *     Minimum block size is 32 bytes.
 */
void *mm_malloc(size_t size)
{
    size_t newsize = MAX(ALIGN(size + 2 * WSIZE), MIN_BLOCK_SIZE);
    size_t blocksize;
    freelistp p = NULL;

    /* try to find suitable block in free list */
    for (int index = get_index(newsize); index < LNUM; ++index)
    {
        if (arr_root[index] != NULL)
        {
            for (p = arr_root[index]; p != NULL; p = LNEXT(p))/* bug: infinity loop */
            {
                blocksize = GET_SIZE(FTRP(p));
                if (blocksize >= newsize)
                {
                    /* block found */
                    index = LNUM;
                    break;
                }
            }
        }
    }

    if (p == NULL)  /* need to extend heap */
        p = extend_heap(CHUNKSIZE * (newsize / CHUNKSIZE + 1));
    blocksize = GET_SIZE(HDRP(p));

    /* newsize = size of allocated block; blocksize = size of remaining part */
    if (blocksize - newsize < MIN_BLOCK_SIZE)
    {
        newsize = blocksize;
        blocksize = 0;
    }
    else
        blocksize -= newsize;

    /* initialize allocated block and remaining part */
    remove_list(p);
    PUT(HDRP(p), PACK(newsize, 1));
    PUT(FTRP(p), PACK(newsize, 1));
    if (blocksize != 0)
    {
        PUT(HDRP(NEXT_BLKP(p)), PACK(blocksize, 0));
        PUT(FTRP(NEXT_BLKP(p)), PACK(blocksize, 0));
        add_list(NEXT_BLKP(p));
    }
    return p;
}

/*
 * mm_free - Free a block by resetting allocated bit and add it to free list. Coalesce if possible.
 */
void mm_free(void *ptr)
{
    int alloc_next = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
    int alloc_prev = GET_ALLOC(HDRP(PREV_BLKP(ptr)));
    freelistp p_next = (freelistp)NEXT_BLKP(ptr);
    freelistp p_prev = (freelistp)PREV_BLKP(ptr);
    freelistp p = (freelistp)ptr;
    size_t newsize;

    switch ((alloc_next << 1) | alloc_prev)
    {
    case 0: /* next block is free; previous block is free; coalesce three blocks */
        remove_list(p_next);
        remove_list(p_prev);
        newsize = GET_SIZE(HDRP(p_next)) + GET_SIZE(HDRP(p_prev)) + GET_SIZE(HDRP(p));
        PUT(HDRP(p_prev), PACK(newsize, 0));
        PUT(FTRP(p_prev), PACK(newsize, 0));
        add_list(p_prev);
        break;
    case 1: /* next block is free; prevoius block is allocated; coalesce with next block */
        remove_list(p_next);
        newsize = GET_SIZE(HDRP(p_next)) + GET_SIZE(HDRP(p));
        PUT(HDRP(p), PACK(newsize, 0));
        PUT(FTRP(p), PACK(newsize, 0));
        add_list(p);
        break;
    case 2: /* next block is allocated; prevoius block is free; coalesce with previous block */
        remove_list(p_prev);
        newsize = GET_SIZE(HDRP(p_prev)) + GET_SIZE(HDRP(p));
        PUT(HDRP(p_prev), PACK(newsize, 0));
        PUT(FTRP(p_prev), PACK(newsize, 0));
        add_list(p_prev);
        break;
    case 3: /* next block is allocated; previous block is allocated; add new block into block list */
        newsize = GET_SIZE(HDRP(p));
        PUT(HDRP(ptr), PACK(newsize, 0));
        PUT(FTRP(ptr), PACK(newsize, 0));
        add_list(p);
        break;
    default:
        printf("mm_free error\n");
        exit(1);
        break;
    }
}

/*
 * mm_realloc - If size is increased, try to get space from next block if it is free, and allocate new block if failed;
 *     else do nothing.
 */
void *mm_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return mm_malloc(size);
    
    if (size == 0)
    {
        mm_free(ptr);
        return NULL;
    }

    size_t oldsize = GET_SIZE(HDRP(ptr));
    size_t newsize = MAX(ALIGN(size + 2 * WSIZE), MIN_BLOCK_SIZE);
    
    if (newsize > oldsize)
    {
        char* next = NEXT_BLKP(ptr);
        size_t nextsize = GET_SIZE(HDRP(next));
        if (GET_ALLOC(HDRP(next)) == 0 && nextsize >= MIN_BLOCK_SIZE + (newsize - oldsize))
        {
            /* get space from next block */
            remove_list(next);
            next += newsize - oldsize;
            PUT(HDRP(next), PACK(nextsize - (newsize - oldsize), 0));
            PUT(FTRP(next), PACK(nextsize - (newsize - oldsize), 0));
            add_list(next);

            PUT(HDRP(ptr), PACK(newsize, 1));
            PUT(FTRP(ptr), PACK(newsize, 1));
            return ptr;
        }
        else
        {
            void* newptr = mm_malloc(size);
            memcpy(newptr, ptr, size);
            mm_free(ptr);
            return newptr;
        }
    }
    else
    {
        /* do nothing */
        return ptr;
    }
}

/*
 * mm_check - Check heap and free list; print the result
 */
void mm_check(char* s)
{
    printf("\nMM_CHECK at %s:\n", s);

    /* heap */
    printf(" Heap: size = %d\n", (char*)epilogue - (char*)prologue + WSIZE);
    char* p;
    size_t size, alloc;
    int allocated_cnt = 0, free_cnt = 0;
    for (p = (char*)prologue; p < (char*)epilogue; p += GET_SIZE(p))
    {
        size = GET_SIZE(p);
        alloc = GET_ALLOC(p);
        if (GET(p) != GET(p + size - WSIZE))
        {
            printf("  error: block size or allocated state is inconsistent\n");
            exit(1);
        }
        printf("  block: address=%lx, size = %u, allocated = %u\n", (long unsigned)p, size, alloc);
        if (alloc) ++allocated_cnt; else ++free_cnt;
    }
    size = GET_SIZE(p);
    alloc = GET_ALLOC(p);
    if (size == 0 && alloc == 1)
        printf("  epilogue is fine\n  stats: %d allocated, %d free\n", allocated_cnt, free_cnt);
    else
    {
        printf("  error: epilogue data corrupt\n");
        exit(1);
    }

    /* free list */
    freelistp node, prev;
    printf(" Free list: num = %d\n", LNUM);
    for (int i = 0; i < LNUM; ++i)
    {
        printf("  List %d: ", i);
        if (arr_root[i] == NULL)
            printf("empty\n");
        else
        {
            printf("root");
            for (node = arr_root[i], prev = (freelistp)&arr_root[i]; node; prev = node, node = LNEXT(node))
            {
                printf(" -> %lx", (long unsigned)node);
                if (LPREV(node) != prev)
                {
                    printf("\n  error: wrong LPREV of last node shown\n");
                    exit(1);
                }
            }
            printf("\n");
        }
    }
    return;
}












