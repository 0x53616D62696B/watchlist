# Concurrency

I have two main concurrency solutions. Asynchronous Event Loop and Thread Pool Manager. Both have their use and may be used
simultaneously.

EventLoopCoroutine and EventLoopGenerator are only pre solution tests with examples.

## What is concurrency?

Concurrency is the ability of a system to handle multiple tasks or processes at the same time, or seemingly at the same time,
by interleaving their execution. It doesn't necessarily mean tasks are running truly simultaneously (parallelism),
but rather that they are all in progress concurrently, potentially sharing resources.

## AsynchronousEventLoop

Used for Multithreaded (in a way) asynchronous event loops. A worker manager that guides one or more threads to handle their own
events without paralellism.

### TODO

- Currentl worker does not behave exactly as described above.

## ThreadPoolManager

Worker manager which uses a pool to manage threads. Usefull for plenty of short-living threads.

## TODOO

- Merge Asynchronous Event Loop and Thread Pool Manager into one solution with single worker manager.
