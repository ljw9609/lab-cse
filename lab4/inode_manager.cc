#include "inode_manager.h"
// ID:515030910036
// Name:吕嘉伟
// disk layer -----------------------------------------

disk::disk()
{
  bzero(blocks, sizeof(blocks));
}

void
disk::read_block(blockid_t id, char *buf)
{
  /* done!
   *your lab1 code goes here.
   *if id is smaller than 0 or larger than BLOCK_NUM 
   *or buf is null, just return.
   *put the content of target block into buf.
   *hint: use memcpy
   *void *memcpy(void *dest, void *src, unsigned int count)
  */
  if(id < 0 || id > BLOCK_NUM || buf == NULL)
    return;
  memcpy(buf,blocks[id],BLOCK_SIZE);
}

void
disk::write_block(blockid_t id, const char *buf)
{
  /* done!
   *your lab1 code goes here.
   *hint: just like read_block
  */
  if(id < 0 || id > BLOCK_NUM || buf == NULL)
    return;
  memcpy(blocks[id],buf,BLOCK_SIZE);
}

// block layer -----------------------------------------

// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /* done!
   * your lab1 code goes here.
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.

   *hint: use macro IBLOCK and BBLOCK.
          use bit operation.
          remind yourself of the layout of disk.
   */
  blockid_t id;
  blockid_t start_block_id = (BLOCK_NUM / BPB + INODE_NUM + 4); //last inode table + 1
  uint32_t offset;
  char buf[BLOCK_SIZE];

  for(id = start_block_id;id < BLOCK_NUM;id++){
    offset = id % BPB;  //block[id]在bitmap对应block的偏移量,一个block存BLOCK_SIZE*8个block
    read_block(BBLOCK(id),buf); //bitmap block containing block[id] info
    char mask = 1 << (offset % 8); //use for 0/1 judge
    bool judge = ~(buf[offset/8] | (~(0x1 << (offset % 8)))); //0 = used,!0 = not used
    
    if(judge != 0 ){
      buf[offset/8] |= 1 << (offset % 8); //set bitmap to 1
      write_block(BBLOCK(id),buf); //write to bitmap block
      printf("test alloc block,id=%d\n",id);
      return id;
    }
  }
  return 0;
}

void
block_manager::free_block(uint32_t id)
{
  /* done!
   * your lab1 code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
  blockid_t start_block_id = (BLOCK_NUM / BPB + INODE_NUM + 4);
  uint32_t offset;
  char buf[BLOCK_SIZE];

  if(id < start_block_id || id >= BLOCK_NUM){
    printf("illegal block id:%d\n",id);
    return;
  }
  else{
    offset = id % BPB;
    read_block(BBLOCK(id),buf); //bitmap block containing block[id] info
    char mask = ~(1 << (offset % 8));
    buf[offset/8] &= mask;  //set bitmap to 0
    write_block(BBLOCK(id),buf);
    printf("test free block,id=%d\n",id);
  }
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager()
{
  d = new disk();

  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;

  char buf[BLOCK_SIZE];
  bzero(buf, sizeof(buf));
  memcpy(buf, &sb, sizeof(sb));
  write_block(1, buf);
}

void
block_manager::read_block(uint32_t id, char *buf)
{
  d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf)
{
  d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* done!
   * your lab1 code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().
    
   * if you get some heap memory, do not forget to free it.
   */
  
  struct inode *ino;
  char buf[BLOCK_SIZE];
  
  for(int inum = 1;inum <= INODE_NUM;inum++){
    bm->read_block(IBLOCK(inum,bm->sb.nblocks),buf);
    ino = (struct inode*)buf + inum%IPB;
    if(ino->type == 0){ //if not exit
      ino->type = type;
      ino->size = 0;
      ino->atime = ino->mtime = ino->ctime = time(NULL);
      //put_inode(inum,ino);
      bm->write_block(IBLOCK(inum,bm->sb.nblocks),buf);
      printf("test alloc inode,inum=%d,type=%d\n",inum,type);
      /*
      char tmp_buf[BLOCK_SIZE];
      struct inode *tmp_ino;
      bm->read_block(IBLOCK(inum,bm->sb.nblocks),tmp_buf);
      tmp_ino = (struct inode*)tmp_buf;
      printf("inum=%d,type=%d\n",inum,tmp_ino->type);
      */
      return inum;  //return inum
    }
  }

  return 0;
}

void
inode_manager::free_inode(uint32_t inum)
{
  /* done!
   * your lab1 code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   * do not forget to free memory if necessary.
   */
  char buf[BLOCK_SIZE];
  struct inode *ino;

  if (inum <= 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum,bm->sb.nblocks),buf);
  ino = (struct inode*)buf + inum%IPB;

  ino->type = 0;
  bm->write_block(IBLOCK(inum,bm->sb.nblocks),buf);

  printf("test free inode,inum=%d\n",inum);
}


/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_manager::get_inode(uint32_t inum)
{
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];

  printf("\tim: get_inode %d\n", inum);

  if (inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return NULL;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf); //block in inode table containing inode inum
  // printf("%s:%d\n", __FILE__, __LINE__);

  ino_disk = (struct inode*)buf + inum%IPB; 
  if (ino_disk->type == 0) {
    printf("\tim: inode not exist\n");
    return NULL;
  }

  ino = (struct inode*)malloc(sizeof(struct inode));
  *ino = *ino_disk;

  return ino;
}

void
inode_manager::put_inode(uint32_t inum, struct inode *ino)
{
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;

  printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode*)buf + inum%IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf_out, int *size)
{
  /* done!
   * your lab1 code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_out
   */
  printf("test read file,inum=%d\n",inum);
  char buf[BLOCK_SIZE];
  char file_buf[BLOCK_SIZE];
  struct inode *ino;
  int file_size;

  if (inum <= 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum,bm->sb.nblocks),buf);
  ino = (struct inode*)buf + inum%IPB;

  /* update last access time */
  ino->atime = time(NULL); 
  bm->write_block(IBLOCK(inum,bm->sb.nblocks),buf);

  /* read out file size */
  if(ino->size == 0)
    return;
  file_size = ino->size;
  *size = ino->size;

  char *file;
  file = (char *)malloc(*size);
  *buf_out = file;
  
  /* read direct block */
  for(int i = 0;i < NDIRECT;i++){
    if(file_size > 0){
      blockid_t block_id = ino->blocks[i];
      bm->read_block(block_id,file_buf);
      memcpy(file + i * BLOCK_SIZE,file_buf,MIN(file_size,BLOCK_SIZE)); //MIN处理结尾问题
      file_size -= BLOCK_SIZE;
    }
    else
      return;
  }

  /* read indirect block */
  bm->read_block(ino->blocks[NDIRECT],buf); //get indirect blocks
  blockid_t *blocks = (blockid_t *)buf; 
  for(uint i = 0;i < NINDIRECT;i++){
    if(file_size > 0){
      blockid_t block_id = blocks[i];
      bm->read_block(block_id,file_buf);
      memcpy(file + (NDIRECT + i) * BLOCK_SIZE,file_buf,MIN(file_size,BLOCK_SIZE));
      file_size -=BLOCK_SIZE;
    }
    else 
      return;
  }


}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size)
{
  /*
   * your lab1 code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode
   */
  //printf("test write file,inum=%d,write buf=%s,size=%d\n",inum,buf,size);
  
  struct inode *ino;
  char buf_ino[BLOCK_SIZE];
  char buf_file[BLOCK_SIZE];
  char buf_blk[BLOCK_SIZE];
  int original_block_cnt, need_block_cnt, blk;
  int rest_size;
  blockid_t *blocks;

  if (inum <= 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return;
  }

  if (size < 0 || size > MAXFILE * BLOCK_SIZE) {
    printf("\tim: size out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf_ino);
  ino = (struct inode*) buf_ino + inum%IPB;

  ino->mtime = time(NULL);
  if (size != ino->size)
      ino->ctime = ino->mtime;

  
  if (ino->size == 0)
    original_block_cnt = -1;
  else
    original_block_cnt = ((int)(ino->size) - 1) / BLOCK_SIZE;
  need_block_cnt = (size - 1) / BLOCK_SIZE;
  
  printf("\torgsize: %u, newsize: %d, orgblk: %d, newblk: %d\n", ino->size, size, original_block_cnt, need_block_cnt);

  // direct
  rest_size = size;
  for (blk = 0; blk < NDIRECT; blk++) {
    if (rest_size > 0) {
      memcpy(buf_file, buf + blk * BLOCK_SIZE, MIN(BLOCK_SIZE, rest_size));
      rest_size -= BLOCK_SIZE;

      // need blk larger than original blk
      if (blk > original_block_cnt){
        ino->blocks[blk] = bm->alloc_block();
      }
      bm->write_block(ino->blocks[blk], buf_file);
    }
    // need blk smaller than original blk
    else if (blk <= original_block_cnt)
      bm->free_block(ino->blocks[blk]);
  }

  // indirect 
  if (original_block_cnt < NDIRECT && need_block_cnt >= NDIRECT)
    ino->blocks[NDIRECT] = bm->alloc_block();

  bm->read_block(ino->blocks[NDIRECT], buf_blk);
  blocks = (blockid_t *) buf_blk;
  for (; blk < MAXFILE; blk++) {
    if (rest_size > 0) {
      memcpy(buf_file, buf + blk * BLOCK_SIZE, MIN(BLOCK_SIZE, rest_size));
      rest_size -= BLOCK_SIZE;
      // need blk larget than original blk
      if (blk > original_block_cnt){
        blocks[blk - NDIRECT] = bm->alloc_block();
      }

      bm->write_block(blocks[blk - NDIRECT], buf_file);
    }
    // need blk smaller than original blk
    else if (blk <= original_block_cnt)
      bm->free_block(blocks[blk - NDIRECT]);
  }

  // free indirect block
  if (need_block_cnt < NDIRECT && original_block_cnt >= NDIRECT)
    bm->free_block(ino->blocks[NDIRECT]);
  else
    bm->write_block(ino->blocks[NDIRECT], buf_blk);

  ino->size = size;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf_ino);
}



  
void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /* done!
   * your lab1 code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
  

  char buf[BLOCK_SIZE];
  struct inode *ino;

  if(inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum,bm->sb.nblocks),buf);
  ino = (struct inode*)buf + inum%IPB;
  a.type = ino->type;
  a.atime = ino->atime;
  a.mtime = ino->mtime;
  a.ctime = ino->ctime;
  a.size = ino->size;

  printf("test getattr,inum=%d,type=%d\n",inum,a.type);
}

void
inode_manager::remove_file(uint32_t inum)
{
  /* done!
   * your lab1 code goes here
   * note: you need to consider about both the data block and inode of the file
   * do not forget to free memory if necessary.
   */

  printf("test remove file,inum=%d\n",inum);

  char buf_ino[BLOCK_SIZE];
  struct inode *ino;

  if (inum <= 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return;
  }

  bm->read_block(IBLOCK(inum,bm->sb.nblocks),buf_ino);
  ino = (struct inode*)buf_ino + inum%IPB;

  int file_size;
  file_size = ino->size;
  printf("rest_size=%d\n",file_size);
  int i;
  /* direct block */
  for(i = 0;i < NDIRECT;i++){
    if(file_size > 0){
      blockid_t block_id = ino->blocks[i];
      bm->free_block(block_id);
      file_size -= BLOCK_SIZE;
      printf("rest_size=%d\n",file_size);
    }
    else{
      printf("finish remove file\n");
      break;
    }
  }
  char buf[BLOCK_SIZE];
  /* indirect block */
  if(file_size > 0){
  blockid_t *indirect_blocks;
  bm->read_block(ino->blocks[NDIRECT],buf);
  indirect_blocks = (blockid_t *)buf;
  for(;i < MAXFILE;i++){
    if(file_size > 0){
      blockid_t block_id = indirect_blocks[i - NDIRECT];
      bm->free_block(block_id);
      file_size -= BLOCK_SIZE;
    }else{
      printf("finish remove file\n");
      break;
    }
  }
  bm->free_block(ino->blocks[NDIRECT]);
}
  free_inode(inum);
}
