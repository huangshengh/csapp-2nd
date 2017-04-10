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


//使用显示空闲链表

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "a",
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


//定义操作显式空闲链表的宏,分配的时候直接在为分配链表中查找而不需要全部查找

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x,y) ((x)>(y)?(x):(y))

#define PACK(size,alloc) ((size)|(alloc))

#define GET(p) (*(unsigned int *)(p))
#define PUT(p,val)  (*(unsigned int *)(p) = (val))


#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

//bp为块指针，是前驱后驱之后的那个位置
#define HDRP(bp)  ((char *)(bp) - 3*WSIZE)
#define FTRP(bp)  ((char *)(bp) + GET_SIZE(HDRP(bp))-4*WSIZE)

//此处使用指针,这两个宏是操作块链表的，这个prev操作对于第一个块不适合，因为第一个块前面是序言块
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(((char *)(bp)-3*WSIZE)))
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp)- 4*WSIZE)))


//对于已分配块来说这个前后指针也是邻近的未分配块
//使用函数来返回前驱和后继空闲块的指针,参数是块指针
void *nextptr(void * bp)
{
    //要考虑bp是第二个序言块的情况，在find——fit中会出现。
    if (GET_SIZE(bp)==8)
        bp = bp + 2*DSIZE;
    //条件是已分配,且bp不是最后一个块,从后面一个bp开始算
    while(GET_ALLOC(HDRP(NEXT_BLKP(bp)))&&NEXT_BLKP(bp)!=NULL)
        bp = NEXT_BLKP(bp);
    //此时说明next_blkp (bp)是有大小，未分配的
    if(GET_SIZE(HDRP(NEXT_BLKP(bp)))&&!GET_ALLOC(HDRP(NEXT_BLKP(bp))))
        return NEXT_BLKP(bp);
    else
        return NULL;
}



void *prevptr(void * bp)
{
    //如果bp是序言块，则修改bp位置
    if (GET_SIZE(bp)==8)
        bp = bp + 2*DSIZE;
    //不要指到序言块
    while(GET_ALLOC(HDRP(bp)-WSIZE) && GET_SIZE(HDRP(bp)-WSIZE)!=8)
        bp = PREV_BLKP(bp);
    if(GET_SIZE(HDRP(bp)-WSIZE)!= 8 && !GET_ALLOC(HDRP(bp)-WSIZE))
        return PREV_BLKP(bp);
    else
        return NULL;
}

static char* heap_listp;

//合并的时候需要注意，因为指针部分可以覆盖了，提高利用率
static void * coalesece(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if(prev_alloc&&next_alloc)
        return bp;
    //后面的未分配
    else if(prev_alloc&&!next_alloc)
    {
        void *temp;
        void * next = bp -WSIZE; 
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        //后面空闲块的后继指针
        temp = nextptr(NEXT_BLKP(bp));
        //更新后继指针
        next = temp;
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }
    else if(!prev_alloc && next_alloc)
    {
        void *temp;
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        temp = nextptr(bp);
        void * next = PREV_BLKP(bp) - WSIZE;
        next = temp;
        PUT(FTRP(bp),PACK(size,0));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))+GET_SIZE(HDRP(NEXT_BLKP(bp)));
        void *temp;
        temp = nextptr(NEXT_BLKP(bp));
        void * next = PREV_BLKP(bp) - WSIZE;
        next = temp;
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

//每次扩展堆时，后继指针都是null，但是前驱的指针还是要计算一下。
//扩展的时候扩展的是一个整块。这个整块后面就是结尾块
static void * extend_heap(size_t words)
{
    char *bp;
    size_t size;
    //8字节对齐
    size = (words%2) ? (words+1)*WSIZE: words*WSIZE;
    //bp是申请的第一个字节，我们需要调整才有前驱后继的指针位置,实际上bp是前驱指针的位置
    //因为bp前面的结尾块会被当成头部使用
    if((long) (bp = mem_sbrk(size))==-1)
        return NULL;
    //后继指针为null
    void * next = bp+WSIZE;
    next = NULL;
    //compute prev，显式空闲链表最少需要16字节,小于16说明他前面是序言块
    if(GET_SIZE(bp-DSIZE)<16)
    {
        bp = NULL;
    }
    else
        bp = prevptr(bp+DSIZE);

    PUT(HDRP(bp+DSIZE),PACK(size,0));
    PUT(FTRP(bp+DSIZE),PACK(size,0));
    //结尾块就不用在bp+disze了，
    PUT(HDRP(NEXT_BLKP(bp)),PACK(0,1));
    return coalesece(bp+DSIZE);
}

//并用分离式配来组织空闲链表，我们只需要把空闲链表按大小分类就可以了，在malloc进行这个工作

/* 
 * mm_init - initialize the malloc package.
 * 这个init只会调用一次，对于init的结构而言并没有指针的位置。
 */
int mm_init(void)
{
    if((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp,0);
    PUT(heap_listp+WSIZE ,PACK(DSIZE,1));
    PUT(heap_listp+DSIZE,PACK(DSIZE,1));
    PUT(heap_listp+3*WSIZE,PACK(0,1));
    heap_listp += DSIZE;

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

//其实主要的区别就在于find的时间，在这里
static void * find_fit(size_t asize)
{
    void *bp;
    for(bp = heap_listp;nextptr(bp)!=NULL;bp = nextptr(bp))
    {
        if(asize<=GET_SIZE(HDRP(bp)))
        {
            return bp;
        }
    }
    return NULL;
}

static void place(void *bp,size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if((csize - asize)>=(3*DSIZE))
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
    //一个块最少也得24字节，加上数据字节且需要8字节对齐
    if(size<DSIZE)
        asize = 3*DSIZE;
    else
        asize = DSIZE *((size+(2*DSIZE)+(DSIZE-1))/DSIZE);

    if ((bp = find_fit(asize))!=NULL)
    {
        place(bp,asize);
        return bp;
    }

    extendsize = MAX(asize,CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE))==NULL)
        return NULL;
    place(bp,asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 * free的时候前后指针开始寻找前后空闲块，更新指针。
 * free采用按地址来维护链表具有更高的存储器利用率
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    void *prev = ptr - 8;
    void *next = ptr - 4;
    void * bp = ptr; 
    void * p = ptr;

    PUT(HDRP(ptr),PACK(size,0));
    PUT(FTRP(ptr),PACK(size,0));
    //好了，此时这个块已经是未分配的了，来进行指针的操作
    //把从这个bp一直到前一个空闲块中间的所有块的nextptr全部改成这个新的空闲块
    //prev_blkp是不安全的，因为前面可能是序言块。
    //所以要修改，考虑序言块的情况
    while(GET_ALLOC(HDRP(bp)-WSIZE)==1&&GET_SIZE(HDRP(bp)-WSIZE)!=8)
    {
        void * temp1 = PREV_BLKP(bp) - 4;
        temp1 = nextptr(PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
    }
    //此时prev_blkp(bp)是前一个未分配的块指针
    if(GET_SIZE(HDRP(bp)-WSIZE)!=8)
    {
        void *temp2 = PREV_BLKP(bp)- 4;
        temp2 = nextptr(PREV_BLKP(bp));
        //ptr的prev也更新一下
        prev = PREV_BLKP(bp);
    }
    while(GET_ALLOC(HDRP(NEXT_BLKP(ptr)))==1&&NEXT_BLKP(ptr)!=NULL)
    {
        void * temp3 = NEXT_BLKP(ptr) - 8;
        temp3 = prevptr(NEXT_BLKP(bp));
        ptr = NEXT_BLKP(ptr);
    }
    //此时next_blkp(ptr)是后一个为分配的块
    if(NEXT_BLKP(ptr)!=NULL)
    {
        void * temp4 = NEXT_BLKP(ptr) - 8;
        temp4 = prevptr(NEXT_BLKP(ptr));
        next = NEXT_BLKP(ptr);
    }
    //至此所有与该free块相关联的指针都更新了，然后在合并
    //突然意识到合并的时候，有关的指针不需要更新
    coalesece(p);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
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














