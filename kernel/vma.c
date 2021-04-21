#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "vma.h"

struct {
  struct spinlock lock;
  struct vm_area_struct areas[NOFILE];
} vma_table;

void
vma_init(void)
{
  initlock(&vma_table.lock, "vma_table");
}

struct vm_area_struct*
vma_alloc(void)
{
  struct vm_area_struct *vmap;

  acquire(&vma_table.lock);
  for(vmap = vma_table.areas; vmap < vma_table.areas + NOFILE; vmap++){
    if(vmap->file == 0){
      release(&vma_table.lock);
      return vmap;
    }
  }
  release(&vma_table.lock);
  return 0;
}

void
vma_free(struct vm_area_struct *vmap)
{
  vmap->file = 0;
}