#include "lock.h"

int* shared_data;
const unsigned int N_THREADS = 1000;
const unsigned int N_INC = 1000;
unsigned int N_BACKOFF = 10;

std::mutex mutex;
int CAS_lock_var;
int futex_lock_var;

void inc(void (*pf_lock)(), void (*pf_unlock)()) {
	for (int i = 0 ; i < N_INC ; i++) {
		pf_lock();
		(*shared_data)++;
		pf_unlock();
	}
}

void print_result(const std::string &lock_method, const Time& s, const Time &e) {
	std::cout << "[";
	std::cout << std::chrono::duration_cast<std::chrono::microseconds>(e-s).count();  
	std::cout << "] usec, ";
	std::cout << lock_method << " ";
	if (*shared_data == N_THREADS * N_INC) 
		std::cout << "correct" << std::endl;
	else
		std::cout << "NOT correct, is: " << *shared_data << " must be: " << N_THREADS * N_INC << std::endl;
}

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
	for (int i = 0 ; i < N_BACKOFF ; i++) {
		if (0 == __sync_val_compare_and_swap(&futex_lock_var, 0, 1))
			return;
	}


	while (1) {
		int futex_ret = futex(&futex_lock_var, FUTEX_WAIT, 1, NULL, NULL, 0);
		if (futex_ret == 0 && futex_lock_var != 0) { /* spurious wake-up */
			continue;
		}
		int CAS_ret = __sync_val_compare_and_swap(&futex_lock_var, 0, 1);
		if (CAS_ret == 0)
			return;
		else
			continue;
	}
}

void futex_unlock() {
	__sync_val_compare_and_swap(&futex_lock_var, 1, 0);
	futex(&futex_lock_var, FUTEX_WAKE, 1, NULL, NULL, 0);
}

void mutex_lock() {
	mutex.lock();
}

void mutex_unlock() {
	mutex.unlock();
}

void perform(void (*pf_lock)(), void (*pf_unlock)(), std::string method) {
	auto pThr = new std::thread[N_THREADS]();
	for (int i = 0 ; i < N_THREADS ; i++ )
		pThr[i] = std::thread(inc, pf_lock, pf_unlock);

	auto start = std::chrono::steady_clock::now();
	for (int i = 0 ; i < N_THREADS ; i++ )
		pThr[i].join();

	auto end = std::chrono::steady_clock::now();
	print_result(method, start, end);

}

unsigned int perform_ret(void (*pf_lock)(), void (*pf_unlock)()) {
	auto pThr = new std::thread[N_THREADS]();
	for (int i = 0 ; i < N_THREADS ; i++ )
		pThr[i] = std::thread(inc, pf_lock, pf_unlock);

	auto start = std::chrono::steady_clock::now();
	for (int i = 0 ; i < N_THREADS ; i++ )
		pThr[i].join();

	auto end = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();  
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

unsigned int do_futex_ret() {
	*shared_data = 0;

	futex_lock_var = 0;
	void (*pf_lock)() = futex_lock;
	void (*pf_unlock)() = futex_unlock;

	return perform_ret(pf_lock, pf_unlock);
}

void do_pthr_mutex() {
	*shared_data = 0;

	void (*pf_lock)() = mutex_lock;
	void (*pf_unlock)() = mutex_unlock;

	perform(pf_lock, pf_unlock, "mutex");
}
