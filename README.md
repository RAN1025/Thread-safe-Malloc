#  Thread-Safe Malloc
   In this project, two different thread-safe versions of the malloc () and free () functions are implemented. Both of these functions use the best fit allocation policy.
   
   
   In version 1 of the thread-safe malloc/free functions, lock-based synchronization is used to prevent race conditions. While version 2 does not use locks (only the sbrk function uses locks), instead, the Thread-Local Storage is applied here. 
