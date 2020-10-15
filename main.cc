#include <errno.h>
#include <linux/futex.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <mutex>
#include <string>

int* shared_data;
static const unsigned int N_THREADS = 100;
static const unsigned int N_INC = 1000;

std::mutex mutex;
int CAS_lock_var;
int futex_lock_var;

int futex(int* uaddr, int futex_op, int val, const struct timespec* timeout,
    int* uaddr2, int val3) {
  return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
}

void dummy_func() {}
void CAS_lock() {
  while (1) {
    int ret = __sync_val_compare_and_swap(&CAS_lock_var, 0, 1);
    if (ret == 0) break;
  }
}

void CAS_unlock() {
  __sync_val_compare_and_swap(&CAS_lock_var, 1, 0);
}

void futex_lock() {
  if (0 == __sync_val_compare_and_swap(&futex_lock_var, 0, 1))
    return;

  while (1) {
    int ret = futex(&futex_lock_var, FUTEX_WAIT, 0, NULL, NULL, 0);
    if (ret == -1) { } // retry futex
    else if (ret == 0) {
      if (futex_lock_var == 0) {
        int CAS_ret = __sync_val_compare_and_swap(&futex_lock_var, 0, 1);
        if (CAS_ret == 0)
          return;
        else
          continue;
      }
      else {
        //std::cout << "sprious wakeup" << std::endl;
        continue;
      }
    }
  }
}

void futex_unlock() {
  __sync_val_compare_and_swap(&futex_lock_var, 1, 0);
  int ret = futex(&futex_lock_var, FUTEX_WAKE, 1, NULL, NULL, 0);
}

void mutex_lock() {
  mutex.lock();
}

void mutex_unlock() {
  mutex.unlock();
}

void inc(void (*pf_lock)(), void (*pf_unlock)()) {
  for (int i = 0 ; i < N_INC ; i++) {
    pf_lock();
    (*shared_data)++;
    pf_unlock();
  }
}

void print_result(std::string lock_method) {
  std::cout << lock_method << " ";
  if (*shared_data == N_THREADS * N_INC) 
    std::cout << "correct" << std::endl;
  else
    std::cout << "NOT correct, is: " << *shared_data << " must be: " << N_THREADS * N_INC << std::endl;
}

void perform(void (*pf_lock)(), void (*pf_unlock)(), std::string method) {
  auto pThr = new std::thread[N_THREADS]();
  for (int i = 0 ; i < N_THREADS ; i++ )
    pThr[i] = std::thread(inc, pf_lock, pf_unlock);

  for (int i = 0 ; i < N_THREADS ; i++ )
    pThr[i].join();

  print_result(method);
}

void do_dummy() {
  *shared_data = 0;

  void (*pf_lock)() = dummy_func;
  void (*pf_unlock)() = dummy_func;

  perform(pf_lock, pf_unlock, "dummy lock");
}

void do_CAS() {
  *shared_data = 0;

  CAS_lock_var = 0;
  void (*pf_lock)() = CAS_lock;
  void (*pf_unlock)() = CAS_unlock;

  perform(pf_lock, pf_unlock, "CAS");
}

void do_futex() {
  *shared_data = 0;

  futex_lock_var = 0;
  void (*pf_lock)() = futex_lock;
  void (*pf_unlock)() = futex_unlock;

  perform(pf_lock, pf_unlock, "futex");
}

void do_pthr_mutex() {
  *shared_data = 0;

  void (*pf_lock)() = mutex_lock;
  void (*pf_unlock)() = mutex_unlock;

  perform(pf_lock, pf_unlock, "mutex");
}

int main(int argc, char** argv) {
  int shm_id = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0666);
  if (shm_id < 0) {
    perror("shmget");
    exit(1);
  }
  shared_data = (int*)shmat(shm_id, NULL, 0);

  do_dummy();
  do_CAS();
  do_pthr_mutex();
  do_futex();

  return 0;
}
