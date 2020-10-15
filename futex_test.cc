#include "lock.h"
#include <vector>

static const unsigned int N_CASES = 100; /* How many cases we're gonna average. */
static const unsigned int MAX_BACKOFF = N_THREADS / 10;

int main() {
	int shm_id = shmget(IPC_PRIVATE, 4096, IPC_CREAT | 0666);
	if (shm_id < 0) {
		perror("shmget");
		exit(1);
	}
	shared_data = (int*)shmat(shm_id, NULL, 0);
	auto *usec_list = new std::vector<unsigned int>(MAX_BACKOFF + 1);

	for (int i = 1 ; i <= MAX_BACKOFF || i == 1 ; i++) {
		N_BACKOFF = i;
		unsigned int accum = 0;
		std::cout << "performing ";
		std::cout << i;
		std::cout << "th test" << std::endl;
		for (int j = 0 ; j < N_CASES ; j++)
			accum += do_futex_ret();
		(*usec_list)[i] = accum / N_CASES;
	}

	std::cout << "**** Threads: ";
	std::cout << N_THREADS;
	std::cout << "****" << std::endl;
	for (int i = 1 ; i <= MAX_BACKOFF ; i++) {
		std::cout << "[";
		std::cout << i;
		std::cout << "]: ";
		std::cout << (*usec_list)[i] << std::endl;
	}
	return 0;
}
