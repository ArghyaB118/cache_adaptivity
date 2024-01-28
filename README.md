# Cache-adaptivity test
### Setting up stxxl
Run `./setup_stxxl.sh`. It should not need sudo.

opening nullbytes need sudo.

### Longest Common Subsequence (LCS)
We test three algorithms for LCS.
- Classic DP algorithm (Classic)
- Hirschberg's algorithm (Hirschberg)
- Chowdhury and Ramachandran's cache-oblivious algorithm (Oblivious)

Bender et al. proved that Oblivious is cache adaptive.

Run with `sudo` because the oom_disabler will require that. The details of measured time (user time, system time) is in [this blog](https://stackoverflow.com/questions/556405/what-do-real-user-and-sys-mean-in-the-output-of-time1).

### Median
The algorithms and their implementations are given by Prof. Rezaul A. Chowdhury. To run the programs, run `make med`, then `./med -if inputs/seq_128.in`.

### Reference implementations
The reference implementations of all algorithms for lcs and median problems are taken from Prof. Rezaul A. Chowdhury.
Reference:
1. Chowdhury, Rezaul Alam and Vijaya Ramachandran. “Cache-oblivious dynamic programming.” ACM-SIAM Symposium on Discrete Algorithms (SODA 2006).
2. Rezaul Alam Chowdhury. "Experimental Evaluation of an Efficient Cache-Oblivious LCS Algorithm." UTCS Technical Report TR-05-43. October 05, 2005.
3. Chowdhury RA, Le HS, Ramachandran V. Cache-oblivious dynamic programming for bioinformatics. IEEE/ACM Trans Comput Biol Bioinform. 2010 Jul-Sep;7(3):495-510. doi: 10.1109/TCBB.2008.94. PMID: 20671320.

### Reading diskstats
Fields of `/proc/diskstats`
1. Number of reads completed
    This is the total number of reads completed successfully.

2. Number of reads merged
    Reads and writes which are adjacent to each other may be merged for
    efficiency.  Thus two 4K reads may become one 8K read before it is
    ultimately handed to the disk, and so it will be counted (and queued)
    as only one I/O.  This field lets you know how often this was done.

3. Number of sectors read
    This is the total number of sectors read successfully.

4. Number of milliseconds spent reading
    This is the total number of milliseconds spent by all reads (as
    measured from `__make_request()` to end_that_request_last()).

5. Number of writes completed
    This is the total number of writes completed successfully.

6. Number of writes merged
    See the description of field 2.

7. Number of sectors written
    This is the total number of sectors written successfully.

8. Number of milliseconds spent writing
    This is the total number of milliseconds spent by all writes (as
    measured from `__make_request()` to `end_that_request_last()`).

9. Number of I/Os currently in progress
    The only field that should go to zero. Incremented as requests are
    given to appropriate struct request_queue and decremented as they finish.

10. Number of milliseconds spent doing I/Os
    This field increases so long as field 9 is nonzero.

11. weighted number of milliseconds spent doing I/Os
    This field is incremented at each I/O start, I/O completion, I/O
    merge, or read of these stats by the number of I/Os in progress
    (field 9) times the number of milliseconds spent doing I/O since the
    last update of this field.  This can provide an easy measure of both
    I/O completion time and the backlog that may be accumulating.

12. Number of discards completed
    This is the total number of discards completed successfully.

13. Number of discards merged
    See the description of field 2

14. Number of sectors discarded
    This is the total number of sectors discarded successfully.

15. Number of milliseconds spent discarding
    This is the total number of milliseconds spent by all discards (as
    measured from `__make_request()` to `end_that_request_last()`).

### Cgroup related workaround
The cgroup may act dirty and kill the process. To check build the file, run it without cgroup; it should run fine. The workaround is to kill the cgroup and make a new one.
`sudo cgdelete memory:cache-test-arghya
sudo cgcreate -g memory:cache-test-arghya -t arghya:arghya
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes
sudo bash -c "echo 4194304 > /sys/fs/cgroup/memory/cache-test-arghya/memory.limit_in_bytes"
cat /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control
sudo bash -c "echo 1 > /sys/fs/cgroup/memory/cache-test-arghya/memory.oom_control"`
