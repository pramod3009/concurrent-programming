#!/bin/sh
perf stat -d ./concurrent-bst --lock=fg --contention=low -t 12 -i 100
perf stat -d ./concurrent-bst --lock=fg --contention=high -t 12 -i 100 
perf stat -d ./concurrent-bst --lock=fg --contention=low -t 12 -i 1000
perf stat -d ./concurrent-bst --lock=fg --contention=high -t 12 -i 1000

perf stat -d ./concurrent-bst --lock=rw --contention=low -t 12 -i 100
perf stat -d ./concurrent-bst --lock=rw --contention=high -t 12 -i 100 
perf stat -d ./concurrent-bst --lock=rw --contention=low -t 12 -i 1000
perf stat -d ./concurrent-bst --lock=rw --contention=high -t 12 -i 1000
