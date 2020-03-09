/* mm-explicit-free-list.c
 * Explicit free list. Each allocated block contains a footer and a header
 * and each free block contains a footer, a header, a previous pointer that points
 * to the previous free block and a next pointer that points to the next free block.
 * Uses LIFO policy. 
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"
#define WSIZE      4 
#define DSIZE      (2 * WSIZE)    
#define CHUNKSIZE  (1 << 12)      

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define PACK(size, alloc)  ((size) | (alloc))

#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (val))

#define GET_SIZE(p)   (GET(p) & ~0x7)
#define GET_ALLOC(p)  (GET(p) & 0x1)

#define HDRP(bp)  ((void *)(bp) - WSIZE)
#define FTRP(bp)  ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp)  ((void *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp)  ((void *)(bp) - GET_SIZE((void *)(bp) - DSIZE))

#define GET_NEXT_PTR(bp)  (*(char **)(bp + WSIZE))
#define GET_PREV_PTR(bp)  (*(char **)(bp))

#define SET_NEXT_PTR(bp, np) (GET_NEXT_PTR(bp) = np)
#define SET_PREV_PTR(bp, pp) (GET_PREV_PTR(bp) = pp)

/* Global declarations */
static char *heap_listp = 0;
static char *explicit_listp = 0;

static void *coalesce(void *ptr);
static void *extend_heap(size_t words);
static void *find_fit(size_t asize);
static void place(void *ptr, size_t asize);
//static void checkheap();

static void insertp(void *bp);
static void removep(void *bp);

static void insertp(void *bp) {
	SET_NEXT_PTR(bp, explicit_listp);
	SET_PREV_PTR(explicit_listp, bp);
	SET_PREV_PTR(bp, NULL);
	explicit_listp = bp;
}
static void removep(void *bp) {
	if (GET_PREV_PTR(bp))
		SET_NEXT_PTR(GET_PREV_PTR(bp), GET_NEXT_PTR(bp));
	else
		explicit_listp = GET_NEXT_PTR(bp);
	SET_PREV_PTR(GET_NEXT_PTR(bp), GET_PREV_PTR(bp));
}

static void *coalesce(void *bp) {

	size_t NEXT_ALLOC = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
	size_t PREV_ALLOC = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
	size_t size = GET_SIZE(HDRP(bp));

	if (PREV_ALLOC && !NEXT_ALLOC) {
		size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
		removep(NEXT_BLKP(bp));
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if (!PREV_ALLOC && NEXT_ALLOC) {
		size += GET_SIZE(HDRP(PREV_BLKP(bp)));
		bp = PREV_BLKP(bp);
		removep(bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	else if (!PREV_ALLOC && !NEXT_ALLOC) {
		size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp)));
		removep(PREV_BLKP(bp));
		removep(NEXT_BLKP(bp));
		bp = PREV_BLKP(bp);
		PUT(HDRP(bp), PACK(size, 0));
		PUT(FTRP(bp), PACK(size, 0));
	}
	insertp(bp);
	return bp;
}

static void place(void *bp, size_t asize) {
	size_t csize = GET_SIZE(HDRP(bp));

	if ((csize - asize) >= 4 * WSIZE) {
		PUT(HDRP(bp), PACK(asize, 1));
		PUT(FTRP(bp), PACK(asize, 1));
		removep(bp);
		bp = NEXT_BLKP(bp);
		PUT(HDRP(bp), PACK(csize - asize, 0));
		PUT(FTRP(bp), PACK(csize - asize, 0));
		coalesce(bp);
	}
	else {
		PUT(HDRP(bp), PACK(csize, 1));
		PUT(FTRP(bp), PACK(csize, 1));
		removep(bp);
	}
}

static void *extend_heap(size_t words) {
	char *bp;
	size_t size;

	size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
	if (size < 16) {
		size = 16;
	}
	if ((int)(bp = mem_sbrk(size)) == -1) {
		return NULL;
	}
	PUT(HDRP(bp), PACK(size, 0));
	PUT(FTRP(bp), PACK(size, 0));
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
	return coalesce(bp);
}

static void *find_fit(size_t asize) {
	void *bp;
	for (bp = explicit_listp; GET_ALLOC(HDRP(bp)) == 0; bp = GET_NEXT_PTR(bp)) {
		if (asize <= (size_t)GET_SIZE(HDRP(bp))) {
			return bp;
		}
	}
	return NULL;
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {

	if ((heap_listp = mem_sbrk(8 * WSIZE)) == NULL)
		return -1;

	PUT(heap_listp, 0);                           
	PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); 
	PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); 
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     
	explicit_listp = heap_listp + 2 * WSIZE;

	if (extend_heap(4) == NULL) {
		return -1;
	}
	return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
	size_t asize; 
	size_t extendsize; 
	void *bp;
	if (size == 0)
		return (NULL);
	if (size <= DSIZE)
		asize = 2 * DSIZE;
	else
		asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
	if ((bp = find_fit(asize)) != NULL) {
		place(bp, asize);
		return (bp);
	}
	extendsize = MAX(asize, CHUNKSIZE);
	if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
		return (NULL);
	place(bp, asize);
	return (bp);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
	size_t size;
	if (ptr == NULL)
		return;
	size = GET_SIZE(HDRP(ptr));
	PUT(HDRP(ptr), PACK(size, 0));
	PUT(FTRP(ptr), PACK(size, 0));
	//checkheap();
	coalesce(ptr);
}

/*
* mm_realloc - Implemented simply in terms of mm_malloc and mm_free
*/
void *mm_realloc(void *ptr, size_t size) {
	if ((int)size < 0)
		return NULL;
	else if ((int)size == 0) {
		mm_free(ptr);
		return NULL;
	}
	else if (size > 0) {
		size_t current_size = GET_SIZE(HDRP(ptr));
		size_t new_size = size + 2 * WSIZE; 
										   
		if (new_size <= current_size) {
			return ptr;
		}
		else {
			size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
			size_t csize;
			if (!next_alloc && ((csize = current_size + GET_SIZE(HDRP(NEXT_BLKP(ptr))))) >= new_size) {
				removep(NEXT_BLKP(ptr));
				PUT(HDRP(ptr), PACK(csize, 1));
				PUT(FTRP(ptr), PACK(csize, 1));
				//checkheap();
				return ptr;
			}
			else {
				void *new_ptr = mm_malloc(new_size);
				place(new_ptr, new_size);
				memcpy(new_ptr, ptr, new_size);
				mm_free(ptr);
				//checkheap();
				return new_ptr;
			}
		}
	}
	else {
		return NULL;
	}
}

//static void checkheap() {
//	void *bp;
//	for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = (void *)NEXT_BLKP(bp)) {
//		if ((uintptr_t)bp % DSIZE) 
//		{
//			printf("Pointer %p is not aligned\n", bp);
//		}
//		if (GET_SIZE(HDRP(bp)) != GET_SIZE(FTRP(bp)))
//		{
//			printf("header does not match with footer\n");
//		}
//	}
//	for (bp = explicit_listp; GET_ALLOC(HDRP(bp) == 0; bp = GET_NEXT_PTR){
//		if (GET_SIZE(HDRP(bp) != GET_SIZE(FTRP(bp)){
//			printf("header does not match with footer\n");
//		}
//	}
//}