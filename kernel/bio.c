// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"
#define NBUC         13
struct {
  struct spinlock lock;
  struct buf buf[NBUF];
} bcache;

struct bMem{
	struct spinlock lock;
	struct buf head;
};

static struct bMem hashTable[NBUC];

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  for(int i = 0; i < NBUC; i++){
  	initlock(&(hashTable[i].lock), "bcache.bucket");
  }
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
	struct buf *lastBuf;

	// Is the block already cached?
	uint64 num = blockno%NBUC;
	acquire(&(hashTable[num].lock));
	if (bcache.lock.locked){
		sleep(&bcache.lock, &(hashTable[num].lock));
	}
	for(b = hashTable[num].head.next, lastBuf = &(hashTable[num].head); b; b = b->next){
		if (!(b->next)){
			lastBuf = b;
		}
		if(b->dev == dev && b->blockno == blockno){
			b->refcnt++;
			release(&(hashTable[num].lock));
			acquiresleep(&b->lock);
			return b;
		}
	}

	struct buf *lruBuf = 0;
	acquire(&bcache.lock);
	for(b = bcache.buf; b < bcache.buf + NBUF; b++){
    if(b->refcnt == 0) {
    	if (lruBuf == 0){
    		lruBuf = b;
    		continue;
    	}
      if (b->tick < lruBuf->tick){
		    lruBuf = b;
      }
    }
  }

  if (lruBuf){
	  uint64 oldTick = lruBuf->tick;
		uint64 oldNum = lruBuf->blockno%NBUC;
	  lruBuf->dev = dev;
	  lruBuf->blockno = blockno;
	  lruBuf->valid = 0;
	  lruBuf->refcnt = 1;
	  lruBuf->tick = ticks;
		if(oldTick == 0){
			lastBuf->next = lruBuf;
			lruBuf->prev = lastBuf;
		}else {
			if (oldNum != num){
				//acquire(&(hashTable[oldNum].lock));
				lruBuf->prev->next = lruBuf->next;
				if (lruBuf->next){
					lruBuf->next->prev = lruBuf->prev;
				}
				//release(&(hashTable[oldNum].lock));
				lastBuf->next = lruBuf;
				lruBuf->prev = lastBuf;
			}
		}
		wakeup(&bcache.lock);
	  release(&bcache.lock);
	  release(&(hashTable[num].lock));
		release(&bcache.lock);
		acquiresleep(&lruBuf->lock);
	  return lruBuf;
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

	uint64 num = b->blockno%NBUC;
	acquire(&(hashTable[num].lock));
  b->refcnt--;
	release(&(hashTable[num].lock));
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt++;
  release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock);
  b->refcnt--;
  release(&bcache.lock);
}


