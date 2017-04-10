/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
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
    "ateam",
    /* First member's full name */
    "win",
    /* First member's email address */
    "win@gmail.com",
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

//操作空闲列表
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) //4GB

#define MAX(x,y) ((x)>(y)?(x):(y))

//pack将大小和以分配位结合起来，返回一个值，可以放在头部和脚部
#define PACK(size,alloc) ((size)|(alloc))

#define GET(p)      (*(unsigned int *)(p))
#define PUT(p,val)  (*(unsigned int *) (p) = (val))

#define GET_SIZE(p)  (GET(p)& ~0x7) // return size
#define GET_ALLOC(p) (GET(p)& 0x1) // return the if allocated

//下面是对块指针的操作，块指针是第一个有效载荷字节的指针
#define HDRP(bp)    ((char *)(bp)-WSIZE) //返回头部地址
#define FTRP(bp)    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)// -8byte是因为这从块指针开始4（头部）+4（块指针）
//返回下一个或者前一个块指针
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp)-WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

// 隐式空闲链表,这个结构自产生起就有四个字节，一个填充字，2个序言块，一个结尾块。
static char *heap_listp;
static void *extend_heap(size_t words);
static void *coalesce(void* bp);

//需要注意的是这里的mem_sbrk返回的men_brk是堆的最后一个字节，也就是空闲链表的结尾块
/* 
 * mm_init - initialize the malloc package.
 创建一个带初始空闲块的堆
 */
int mm_init(void)
{
    //create the initial empty heap,空的空闲链表
    if((heap_listp = mem_sbrk(4*WSIZE))==(void *)-1)
        return -1;
    PUT(heap_listp,0);//填充块
    PUT(heap_listp+(WSIZE), PACK(DSIZE,1));//两个序言块，大小为8，alloca 1
    PUT(heap_listp+(2*WSIZE),PACK(DSIZE,1));
    PUT(heap_listp+(3*WSIZE),PACK(0,1));//结尾块的头部，大小为0，以分配位为1，序言块和结尾块是用来双字对齐的
    heap_listp += (2*WSIZE);//块指针，此时指向第二个序言块
    //扩展堆时一次性扩展4GB，这也太大了吧
    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}


//bp是块指针,这个块是空闲块
static void *coalesce(void *bp)
{
    //前后块的标记位
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc && next_alloc)
        return bp;
    //next_alloc 未分配
    else if(prev_alloc && !next_alloc)
        {
            size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
            PUT(HDRP(bp),PACK(size,0));
            PUT(FTRP(bp),PACK(size,0));
        }
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

//extend_heap是在已有的heap上调用sbrk来增加堆的大小
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    //这个块的大小是双字的整数倍
    size = (words % 2)? (words+1)*WSIZE : words*WSIZE;
    if((long)(bp = mem_sbrk(size))==-1)
        return NULL;
    //bp即之前结尾块
    //新分配的块接在结尾块头部的后面，结尾块成为该空闲块的头部，这个片的最后一个字成为新的结尾块头部
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    //HDRP(NEXT_BLKP(bp-WSIZE))是所申请片的最后一块，先用来称为结尾块
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    //如果前一个块是空闲块那么新申请的就和前面的合并
    return coalesce(bp);
}

static void *find_fit(size_t asize)
{
    void *bp;
    for(bp = heap_listp;GET_SIZE(HDRP(bp))>0;bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
}

static void place(void *bp, size_t asize)
{
    //这个空闲块的大小
    size_t csize = GET_SIZE(HDRP(bp));
    if((csize-asize)>=(2*DSIZE))
    {
        PUT(HDRP(bp),PACK(asize,1));
        PUT(FTRP(bp),PACK(asize,1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp),PACK(csize-asize,0));
        PUT(FTRP(bp),PACK(csize-asize,0));
    }
    else
    {
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if(size==0)
        return NULL;
    if(size<=DSIZE)
        asize = 2*DSIZE;
    else 
        asize = DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);
    if((bp = find_fit(asize))!=NULL)
    {
        place(bp,asize);
        return bp;
    }

    extendsize = MAX(asize,CHUNKSIZE);
    //extend_heap返回的也是块指针
    if((bp = extend_heap(extendsize/WSIZE))==NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 将ptr所指内存块修改为size并返回新的内存块指针
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    if(ptr == NULL)
    {
        newptr = mm_malloc(size);
        return newptr;
    }
    if(size == 0)
    {
        mm_free(ptr);
    }
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    //若旧的大于size就，只复制size部分，否则全部复制，多余的部分不管
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














