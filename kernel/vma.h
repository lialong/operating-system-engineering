struct vm_area_struct {
  char* addr;
  uint64 length;
  char prot;
  char flags;
  struct File *file;
};