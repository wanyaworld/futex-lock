
<!-- ABOUT THE PROJECT -->
## About The Project

This is a implementation of [futex](https://man7.org/linux/man-pages/man2/futex.2.html)-based blocking synchronization. In this lock implemenetation, one first try
an atomic operation ([Compare-And-Swap](https://en.wikipedia.org/wiki/Compare-and-swap)) to test if one can get the lock immediately. If it fails, it calls futex() which makes it (probably) sleep. Later, it is waken up when
the lock holder release the lock by calling futex() with FUTEX_WAKE operation flag. Even after waking up, it checks if the lock variable represents that lock is "not-held-state" and 
retry futex() if it does not. This prevents [sprious wakeup](https://en.wikipedia.org/wiki/Spurious_wakeup) negates the correctness of synchronization.

To test the correctness of our implementation, we create N_THREADS threads and each thread 

1. calls lock() method
2. increases the global variable 
3. calls unlock() method
4. repeats \[1-3\] N_INC times.

Then checks if (N_THREADS * N_INC) is equal to the value of the global variable.
In addition to the futex-lock, we also conduct the test using dummy lock, simple Compare-And-Swap lock and pthread_mutex_lock.


## Getting Started
### Installation
```sh
make
```
### Run
```sh
./run
```

<!-- CONTRIBUTING -->
## Contributing

We will humbly welcome your contributions!




## License

Distributed under the MIT License.



<!-- CONTACT -->
## Contact
6812skiii@gmail.com





