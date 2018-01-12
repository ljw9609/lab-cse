#ifndef yfs_client_h
#define yfs_client_h

#include <string>
#include "lock_protocol.h"
#include "lock_client.h"

//#include "yfs_protocol.h"
#include "extent_client.h"
#include <vector>
#include <fstream>
using namespace std;


class yfs_client {
  extent_client *ec;
  lock_client *lc;
  ofstream log;

 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST };
  /*
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST,
        NOPEM, ERRPEM, EINVA, ECTIM, ENUSE };
  */
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };

  typedef struct fileinfo symlinkinfo;

  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };

 private:
  int version;
  static std::string filename(inum);
  static inum n2i(std::string);

  int lookup_helper(inum, const char *, bool &, inum &);
  int readdir_helper(inum, std::list<dirent> &);
  int write_helper(inum, size_t, off_t, const char *, size_t &);

  void to_version(int v);
  
 public:

  //yfs_client();
  yfs_client(std::string, std::string);
  //yfs_client();
  //yfs_client(std::string, std::string, const char*);

  bool isfile(inum);
  bool isdir(inum);
  bool issymlink(inum);
  
  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);
  int getsymlink(inum, symlinkinfo &);


  int setattr(inum, size_t);
  int lookup(inum, const char *, bool &, inum &);
  int create(inum, const char *, mode_t, inum &);
  int readdir(inum, std::list<dirent> &);
  int write(inum, size_t, off_t, const char *, size_t &);
  int read(inum, size_t, off_t, std::string &);
  int unlink(inum,const char *);
  int mkdir(inum , const char *, mode_t , inum &);

  //int verify(const char* cert_file, unsigned short*);
  
  /** you may need to add symbolic link related methods here.*/
  int symlink(inum , const char * , const char * , inum &);
  int readlink(inum, std::string &);

  // lab5 code here
  void commit();
  void undo();
  void redo();
  
};

#endif 
