// the lock server implementation

#include "lock_server.h"
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

pthread_mutex_t lock_server::mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t lock_server::cond = PTHREAD_COND_INITIALIZER;

lock_server::lock_server():
  nacquire (0)
{
}

lock_protocol::status
lock_server::stat(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
  printf("stat request from clt %d\n", clt);
  r = nacquire;
  return ret;
}

lock_protocol::status
lock_server::acquire(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  printf("test lock server acquire lock %d!\n",lid);
  pthread_mutex_lock(&mutex);

  if(lock_status.find(lid) == lock_status.end()){ //lid is unlocked
    lock_status[lid] = true;
    nacquire++;
  }
  else{ //lid is locked
    while(lock_status[lid]){
      pthread_cond_wait(&cond,&mutex);
    }
    lock_status[lid] = true;
    nacquire++;
  }
  pthread_mutex_unlock(&mutex);
  return ret;
}

lock_protocol::status
lock_server::release(int clt, lock_protocol::lockid_t lid, int &r)
{
  lock_protocol::status ret = lock_protocol::OK;
	// Your lab4 code goes here
  printf("test lock server release lock %d!\n",lid);
  pthread_mutex_lock(&mutex);

  if(lock_status.find(lid) == lock_status.end()){
    ret = lock_protocol::RETRY;
  }
  else{
    lock_status[lid] = false;
    nacquire--;
    pthread_cond_signal(&cond);
  }
  pthread_mutex_unlock(&mutex);
  return ret;
}
