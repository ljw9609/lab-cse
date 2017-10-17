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

//#define DEBUG = 0;

/*
yfs_client::yfs_client()
{
    ec = new extent_client();

}*/

yfs_client::yfs_client(std::string extent_dst)
{
    ec = new extent_client(extent_dst);
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
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

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    printf("isfile: %lld is a dir or a symlnk\n", inum);
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

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_DIR) {
        printf("isfile: %lld is a dir\n", inum);
        return true;
    } 
    printf("isfile: %lld is a file or a symlnk\n", inum);
    return false;
}

bool
yfs_client::issymlink(inum inum)
{
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_SYMLNK) {
        printf("issymlink: %lld is a symlink\n", inum);
        return true;
    } 
    printf("issymlink: %lld is a file or a dir\n", inum);
    return false;
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

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
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

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
    return r;
}

int
yfs_client::getsymlink(inum inum, symlinkinfo &sin)
{
    int r = OK;

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
    //printf("test setattr %016llx,new size=%d",ino,size);

    string buf;

    extent_protocol::attr attr;

    if(ec->getattr(ino, attr) != extent_protocol::OK){
        //printf("get attr error\n");
        r = IOERR;
        return r;
    }

    if(ec->get(ino,buf) != extent_protocol::OK){
        //printf("read file error\n");
        r = IOERR;
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
    //printf("test create\n");
    bool found = false;

    r = lookup(parent,name,found,ino_out);
    // already exist
    if(found){
        //printf("already exit\n");
        r = EXIST;
        return r;
    }

    // not exist,then create
    if(ec->create(extent_protocol::T_FILE,ino_out) != OK){
        //printf("create file error\n");
        r = IOERR;
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
   //printf("test mkdir,parent dir=%016llx,dir name=%s\n",parent,name);

    bool found = false;
    r = lookup(parent,name,found,ino_out);
    // exist
    if(found){
        //printf("dir already exist\n");
        r = EXIST;
        return r;
    }

    // if not exist
    if(ec->create(extent_protocol::T_DIR,ino_out) != OK){
        //printf("create dir error\n");
        r = IOERR;
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
    return r;
}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */
    //printf("test lookup name=%s\n",name);
    list<dirent> direnties;
    dirent entry;
    found = false;

    /*
    extent_protocol::attr attr;

    ec->getattr(parent, attr);
    if (attr.type != extent_protocol::T_DIR){
        r = IOERR;
        return r;
    } */

    r = readdir(parent,direnties);
    if(r != OK){
        //r = IOERR;
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

    /* done!
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */
    //printf("test readdir %016llx\n",dir);

    dirent entry;
    string buf;
    //read dir
    if(ec->get(dir,buf) != extent_protocol::OK){
        //printf("readdir error\n");
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
    //printf("finish readdir\n");
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
    //printf("test read,ino=%016llx,read size=%d,offset=%d\n",ino,size,off);

    // read whole file to buffer
    string buf;
    if(ec->get(ino,buf) != extent_protocol::OK){
        //printf("read file error\n");
        r = IOERR;
        return r;
    }
    // read file start from offset
    /*
    if(off > buf.size()){
        data = "";
    }
    else{
        if(off + size > buf.size()){
            data = buf.substr(off,buf.size()-size);
        }else{
            data = buf.substr(off,size);
        }
    }*/
    
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
    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;

    /* done!
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    printf("test write,ino=%016llx,write siez=%d,offset=%d,data=%s\n",ino,size,off,data);

    // read whole file to buffer
    string buf;
    if(ec->get(ino,buf) != extent_protocol::OK){
        //printf("read file error\n");
        r = IOERR;
        return r;
    }
    
    if(off >= buf.size()){   //off >= length of orignal file
        bytes_written = size + off - buf.size();
        buf.resize(off + size,'\0');
        //printf("here3,bytes_written=%d\n",bytes_written);
    }
    else{   //off < length of orignal file
        if(off + size > buf.size()){
            buf.resize(off + size,'\0');
            bytes_written = size;
            //printf("here1,bytes_written=%d\n",bytes_written);
        }
        else{
            bytes_written = size;
            //printf("here2,bytes_written=%d\n",bytes_written);
        }
    }
    /*
    if (off >= buf.size())
        bytes_written = size + off - buf.size();
    else
        bytes_written = size;
    if (off + size > buf.size())
        buf.resize(off + size, '\0');
        */
    buf.replace(off,size,data,size);
    ec->put(ino,buf);

    #ifdef DEBUG
    printf("write after:\n");
    for (int i = 0; i < buf.size(); i++) {
        printf("%c", buf[i]);
    }
    printf("\n");
    #endif
    #ifdef DEBUG
    printf("finish write,ino=%016llx,bytes_written=%d",ino,bytes_written);
    #endif

    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
    //printf("test unlink,parent ino = %016llx,name=%s\n",parent,name);

    bool found = false;
    inum ino_out = 0;
    r = lookup(parent,name,found,ino_out);
    if (r == IOERR) {
        return r;
    }
    // file not exist
    if(!found){
        r = NOENT;
        return r;
    }
    // file exist
    if(ec->remove(ino_out) != extent_protocol::OK){
        r = IOERR;
        return r;
    }

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
    return r;
}

int yfs_client::symlink(inum parent, const char *name, const char *link, inum &ino_out)
{
    int r = OK;
    //printf("test symlink,parent ino=%016llx,%s--->%s\n",parent,name,link);

    bool found = false;
    lookup(parent,name,found,ino_out);
    if(found){
        //printf("already exist\n");
        r = EXIST;
        return r;
    }
    // create file type symlnk
    if(ec->create(extent_protocol::T_SYMLNK,ino_out) != extent_protocol::OK){
        //printf("symlnk error\n");
        r = IOERR;
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

    size_t bytes_written;
    write(ino_out, strlen(link), 0, link, bytes_written);

    #ifdef DEBUG
    printf("finish symlnk,bytes_written=%d\n",bytes_written);
    #endif
    return r;
}


int yfs_client::readlink(inum ino,string &link)
{
    int r = OK;

    //printf("test readlink,inum=%016llx\n", ino);

    if(ec->get(ino,link) != extent_protocol::OK){
        //printf("read lnk error\n");
        r = IOERR;
    }
    return r;
}
