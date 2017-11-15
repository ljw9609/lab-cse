// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <list>

using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::stringstream;
//#define DEBUG = 0;

/*
yfs_client::yfs_client()
{
    ec = new extent_client();

}*/

yfs_client::yfs_client()
{
  ec = NULL;
  lc = NULL;
}

/*
yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client(extent_dst);
    lc = new lock_client(lock_dst);
    log.open("temp.log");
    version = 1;

    lc->acquire(1);
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
    lc->release(1);
}
*/

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst, const char* cert_file)
{
  ec = new extent_client(extent_dst);
  lc = new lock_client(lock_dst);
  log.open("temp.log");
  version = 1;
  
  lc->acquire(1);
  if (ec->put(1, "") != extent_protocol::OK)
      printf("error init root dir\n"); // XYB: init root dir
  lc->release(1);
}

int
yfs_client::verify(const char* name, unsigned short *uid)
{
    int ret = OK;
    return ret;
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    lc->acquire(inum);

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        lc->release(inum);
        return true;
    } 
    printf("isfile: %lld is a dir or a symlnk\n", inum);

    lc->release(inum);

    return false;
}

/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

bool
yfs_client::isdir(inum inum)
{
    extent_protocol::attr a;

    lc->acquire(inum);

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_DIR) {
        printf("isfile: %lld is a dir\n", inum);
        lc->release(inum);
        return true;
    } 
    printf("isfile: %lld is a file or a symlnk\n", inum);

    lc->release(inum);

    return false;
}

bool
yfs_client::issymlink(inum inum)
{
    extent_protocol::attr a;

    lc->acquire(inum);

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        lc->release(inum);
        return false;
    }

    if (a.type == extent_protocol::T_SYMLNK) {
        printf("issymlink: %lld is a symlink\n", inum);
        lc->release(inum);
        return true;
    } 
    printf("issymlink: %lld is a file or a dir\n", inum);

    lc->release(inum);

    return false;
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

    lc->acquire(inum);

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    lc->release(inum);
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    lc->acquire(inum);

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    lc->release(inum);
    return r;
}

int
yfs_client::getsymlink(inum inum, symlinkinfo &sin)
{
    int r = OK;

    lc->acquire(inum);

    printf("getsymlink %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    sin.atime = a.atime;
    sin.mtime = a.mtime;
    sin.ctime = a.ctime;
    sin.size = a.size;
    printf("getsymlink %016llx -> sz %llu\n", inum, sin.size);

release:
    lc->release(inum);
    return r;
}

#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
yfs_client::setattr(inum ino, size_t size)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */

    //format: setattr ino:80,size:128
    log << "setattr " << "ino:" << ino << ",size:" << size << endl;   

    printf("test setattr %016llx,new size=%d",ino,size);

    lc->release(ino);

    string buf;

    extent_protocol::attr attr;

    if(ec->getattr(ino, attr) != extent_protocol::OK){
        printf("get attr error\n");
        r = IOERR;
        return r;
    }

    if(ec->get(ino,buf) != extent_protocol::OK){
        printf("read file error\n");
        r = IOERR;
        lc->release(ino);
        return r;
    }
    // 扩大
    if(attr.size <= size){
        buf.resize(size,'\0');
    }
    else if(attr.size > size){ //缩小，直接截断
        buf.resize(size);
    }

    ec->put(ino,buf);
    #ifdef DEBUG
    printf("finish setattr %016llx,original size=%d",ino,attr.size);
    #endif
    lc->release(ino);
    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    printf("test create\n");
    lc->acquire(parent);

    bool found = false;

    r = lookup_helper(parent,name,found,ino_out);
    // already exist
    if(found){
        printf("already exit\n");
        r = EXIST;
        lc->release(parent);
        return r;
    }

    // not exist,then create
    if(ec->create(extent_protocol::T_FILE,ino_out) != OK){
        printf("create file error\n");
        r = IOERR;
        lc->release(parent);
        return r;
    }

    string buf;
    // modify parent dir
    string name_str(name);
    ec->get(parent,buf);
    buf += name_str;
    buf += '\0';
    buf += filename(ino_out);
    buf += '\0';
    ec->put(parent,buf);

    #ifdef DEBUG
    printf("finish create,content=%s\n",buf.c_str());
    #endif

    lc->release(parent);

    //format: create parent:5,namelength:5,mode:3,ino:6
    //        abc
    log << "create " << "parent:" << parent << ",namelength:" << strlen(name) << ",mode:" << mode << ",ino:" << ino_out << endl << name << endl;
    
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
   printf("test mkdir,parent dir=%016llx,dir name=%s\n",parent,name);
    lc->acquire(parent);

    bool found = false;
    r = lookup_helper(parent,name,found,ino_out);
    // exist
    if(found){
        printf("dir already exist\n");
        r = EXIST;
        lc->release(parent);
        return r;
    }

    // if not exist
    if(ec->create(extent_protocol::T_DIR,ino_out) != OK){
        printf("create dir error\n");
        r = IOERR;
        lc->release(parent);
        return r;
    }

    // modify parent dir
    string buf;
    string name_str(name);
    ec->get(parent,buf);
    buf += name_str;
    buf += '\0';
    buf += filename(ino_out);
    buf += '\0';
    ec->put(parent,buf);

    #ifdef DEBUG
    printf("finish mkdir,parent dir content:%s\n",buf.c_str());
    #endif

    lc->release(parent);

    //format: mkdir parent:5,namelength:5,mode:3,ino:6
    //        abc
    log << "mkdir " << "parent:" << parent << ",namelength:" << strlen(name) << ",mode:" << mode << ",ino:" << ino_out << endl << name << endl;
    
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;
    lc->acquire(parent);
    
    r = lookup_helper(parent,name,found,ino_out);
    
    lc->release(parent);
    return r;
}

int
yfs_client::lookup_helper(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    printf("test lookup name=%s\n",name);
    list<dirent> direnties;
    dirent entry;
    found = false;

    r = readdir_helper(parent,direnties);
    if(r != OK){
        r = IOERR;
        return r;
    }

    string name_str(name);

    for(list<dirent>::iterator itr = direnties.begin();itr != direnties.end();++itr){
        if(itr->name == name_str){
            found = true;
            ino_out = itr->inum;
            return r;
        }
    }
    #ifdef DEBUG
    printf("finish lookup name=%s,found=%d\n",name,found);
    #endif
    return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list)
{
    int r = OK;
    lc->acquire(dir);

    r = readdir_helper(dir,list);

    lc->release(dir);
    return r;
}

int
yfs_client::readdir_helper(inum dir, std::list<dirent> &list)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    printf("test readdir %016llx\n",dir);

    dirent entry;
    string buf;
    //read dir
    if(ec->get(dir,buf) != extent_protocol::OK){
        printf("readdir error\n");
        r = IOERR;
        return r;
    }

    string filename;
    string inum;
    //read by byte
    for(int i = 0;i < buf.size();){
        //read filename
        for(;i < buf.size();i++){
            if(buf[i] != '\0'){
                filename.push_back(buf[i]);
            }
            else{
                i++;
                break;
            }
        }
        //read inum
        for(;i < buf.size();i++){
            if(buf[i] != '\0'){
                inum.push_back(buf[i]);
            }
            else{
                i++;
                break;
            }
        }
        entry.name = filename;
        entry.inum = n2i(inum);
        #ifdef DEBUG
        printf("filename=%s,inum=%016llx\n",entry.name.c_str(),entry,inum);
        #endif
        list.push_back(entry);
        filename.clear();
        inum.clear();
    }
    printf("finish readdir\n");
    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: read using ec->get().
     */
    printf("test read,ino=%016llx,read size=%d,offset=%d\n",ino,size,off);

    // read whole file to buffer
    lc->acquire(ino);

    string buf;
    if(ec->get(ino,buf) != extent_protocol::OK){
        printf("read file error\n");
        r = IOERR;
        lc->release(ino);
        return r;
    }
    // read file start from offset
    if(off + size > buf.size()){
        data = buf.substr(off);
    }
    else{
        data = buf.substr(off,size);
    }
    
    #ifdef DEBUG
    printf("finish read,ino=%016llx,data size=%lu,data=",ino,data.size());
    for(int i = 0;i < data.size();i++){
        printf("%c",data[i]);
    }
    #endif

    lc->release(ino);
    return r;
}


int 
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{   
    //format: write ino:2,size:2,off:2,datalength:2
    //        a a a a a 
    log << "write " << "ino:" << ino << ",size:" << size << ",off:" << off << ",datalength:" << strlen(data) << endl;
    for (int i = 0; i < strlen(data); i++) 
        log << (int)data[i] << " ";
    log << endl;


    int r = OK;
    lc->acquire(ino);

    r = write_helper(ino,size,off,data,bytes_written);

    lc->release(ino);
    return r;
}

int
yfs_client::write_helper(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    printf("test write,ino=%016llx,write siez=%d,offset=%d,data=%s\n",ino,size,off,data);

    string buf, dataBuf(data);
    dataBuf.resize(size, 0);
    if(ec->get(ino, buf) != extent_protocol::OK) {
        r = IOERR;
        return r;
    }

    if (off + size > buf.size())
        buf += string(off + size - buf.size(), 0);
    buf.replace(off, size, dataBuf);
    bytes_written = size;
    ec->put(ino, buf);

    return r;    
}

int yfs_client::unlink(inum parent,const char *name)
{   
    //format: unlink parent:2,namelength:3
    //        abc
    log << "unlink " << "parent:" << parent << ",namelength:" << strlen(name) << endl << name << endl;
    
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
    printf("test unlink,parent ino = %016llx,name=%s\n",parent,name);
    lc->acquire(parent);

    bool found = false;
    inum ino_out = 0;
    r = lookup_helper(parent,name,found,ino_out);
    if (r == IOERR) {
        lc->release(parent);
        return r;
    }
    // file not exist
    if(!found){
        r = NOENT;
        lc->release(parent);
        return r;
    }
    // file exist
    lc->acquire(ino_out);
    if(ec->remove(ino_out) != extent_protocol::OK){
        r = IOERR;
        lc->release(ino_out);
        lc->release(parent);
        return r;
    }
    lc->release(ino_out);
    // modify parent dir
    string buf;
    ec->get(parent,buf);
    string name_str(name);
    int pos = 0;
    

    //printf("orignal dir content:%s\n",buf.c_str());
    pos = buf.find(name_str);
    buf.replace(pos,strlen(name) + filename(ino_out).size() + 2,"");
	ec->put(parent,buf);

    //printf("finish unlink,new dir content:%s\n",buf.c_str());

    lc->release(parent);
    return r;
}

int yfs_client::symlink(inum parent, const char *name, const char *link, inum &ino_out)
{   
    //format: symlink parent:2,namelength:3,linklength:4,ino:5
    //        name
    //        link
    log << "symlink " << "parent:" << parent << ",namelength:" << strlen(name) << ",linklength:" << strlen(link) << ",ino:" << ino_out << endl << name << endl << link << endl;
    
    int r = OK;
    printf("test symlink,parent ino=%016llx,%s--->%s\n",parent,name,link);
    lc->acquire(parent);

    bool found = false;
    lookup_helper(parent,name,found,ino_out);
    if(found){
        //printf("already exist\n");
        r = EXIST;
        lc->release(parent);
        return r;
    }
    // create file type symlnk
    if(ec->create(extent_protocol::T_SYMLNK,ino_out) != extent_protocol::OK){
        //printf("symlnk error\n");
        r = IOERR;
        lc->release(parent);
        return r;
    }

    // modify parent dir
    lc->acquire(ino_out);
    string buf;
    string name_str(name);
    ec->get(parent,buf);
    buf += name_str;
    buf += '\0';
    buf += filename(ino_out);
    buf += '\0';
    ec->put(parent,buf);

    size_t bytes_written;
    write_helper(ino_out, strlen(link), 0, link, bytes_written);
    lc->release(ino_out);
    #ifdef DEBUG
    printf("finish symlnk,bytes_written=%d\n",bytes_written);
    #endif

    lc->release(parent);
    return r;
}


int yfs_client::readlink(inum ino,string &link)
{
    int r = OK;

    lc->acquire(ino);
    if(ec->get(ino,link) != extent_protocol::OK){
        r = IOERR;
    }
    lc->release(ino);

    return r;
}


void 
yfs_client::to_version(int v){
    lc->acquire(1);
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); 
    lc->release(1);
    for (int i = 2; i <= 1024; i++) {
        ec->remove(i);
    }
    log.close();
    log.open("monitor.log", std::fstream::app);
    log << "to_version-" << v << endl;

    for(int i = 1;i <= v;i++){
        stringstream ss;
        string tmp;
        ss << "version" << i << ".log";
        ss >> tmp;
        log << tmp << endl;

        FILE* file = fopen(tmp.c_str(), "r");
        char tmp_action[10];
        while(fscanf(file,"%s",tmp_action) != EOF){
            string action = string(tmp_action);

                if(action == "setattr"){
                    fscanf(file," ");
                    inum ino;
                    size_t size = 0;
                    fscanf(file,"ino:%lld,size:%ld\n",&ino,&size);
                    //log << "log: " << ino << "," << size;
                    setattr(ino,size);
                }

                if(action == "create"){
                    fscanf(file, " ");
                    inum ino;
                    int size = 0;
                    mode_t mode = 0;
                    inum ino_out;
                    fscanf(file,"parent:%lld,namelength:%d,mode:%d,ino:%lld\n",&ino,&size,&mode,&ino_out);
                    string str;
                    for (int i = 0;i < size;i++) {
                        char c = getc(file);
                        str += c;
                    }
                    create(ino,str.c_str(),mode,ino_out);
                }

                if(action == "mkdir"){
                    fscanf(file, " ");
                    inum ino;
                    int size = 0;
                    mode_t mode = 0;
                    inum ino_out;
                    fscanf(file,"parent:%lld,namelength:%d,mode:%d,ino:%lld\n",&ino,&size,&mode,&ino_out);
                    string str;
                    for(int i = 0;i < size;i++) {
                        char c = getc(file);
                        str += c;
                    }
                    //log << "log: "<< str << endl;
                    mkdir(ino,str.c_str(),mode,ino_out);
                }

                if(action == "write"){
                    fscanf(file, " ");
                    inum ino;
                    size_t size = 0;
                    off_t off;
                    int length = 0;
                    fscanf(file,"ino:%lld,size:%ld,off:%ld,datalength:%d\n",&ino,&size,&off,&length);
                    string str;
                    for(int i = 0; i < length; i++) {
                        int tmp;
                        fscanf(file, "%d ", &tmp);
                        str += (char)tmp;
                    }           
                    size_t a;
                    //log << "log: " << str << endl;
                    write(ino,size,off,str.c_str(),a);
                }

                if(action == "unlink"){
                    fscanf(file," ");
                    inum ino;
                    int length = 0;
                    fscanf(file,"parent:%lld,namelength:%d\n",&ino,&length);
                    string str;
                    for(int i = 0;i < length;i++) {
                        char c = getc(file);
                        str += c;
                    }
                    unlink(ino,str.c_str());
                }

                if(action == "symlink"){
                    fscanf(file," ");
                    inum ino;
                    int length = 0;
                    size_t size = 0;
                    inum ino_out;
                    fscanf(file,"parent:%lld,namelength:%d,linklength:%d,ino:%lld\n",&ino,&length,&size,&ino_out);
                    string str;
                    for(int i = 0;i < length;i++) {
                        char c = getc(file);
                        str += c;
                    }
                    fscanf(file,"\n");
                    string str2;
                    for (int i = 0;i < size;i++) {
                        char c = getc(file);
                        str += c;
                    }
                    symlink(ino,str.c_str(),str2.c_str(),ino_out);
                }
            bzero(tmp_action, sizeof(tmp_action));
            fscanf(file, "\n");
        }
        fclose(file);
        log << tmp << endl;
    }
    log.close();
    log.open("temp.log");

}

void
yfs_client::commit(){
    log.close();
    char old_name[] = "temp.log";
    stringstream ss;
    string tmp;
    ss << "version" << version << ".log";
    ss >> tmp;
    rename(old_name, tmp.c_str());
    log.open("temp.log");
    version ++;
}


void
yfs_client::undo(){
    version --;
    to_version(version);
}

void
yfs_client::redo(){
    version ++;
    to_version(version);
}