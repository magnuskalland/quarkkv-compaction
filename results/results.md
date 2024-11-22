


|  | 8 GiB write, ramfs | 8 GiB write, QuarkStore | Improvement |
| :- | -: | -: | -: |
| Average operation latency     | 67.308 µs | 33.901 µs | 1.985x |
| IOPS                          | 14.683 K | 28.9 K | 1.968x |
| Write amplification           | 7.562 (~60 GiB) | 0 | - |
| **Writes**
| Average write latency         | 67.308 µs | 33.901 µs | 1.985x |
| Median write latency          | 0.949 µs | 0.927 µs | 1.023x |
| Write latency P75             | 1.132 µs | 1.082 µs | 1.046x |
| Write latency P90             | 1.344 µs | 1.316 µs | 1.021x |
| Write latency P99             | 2.202 µs | 2.068 µs | 1.064x |
| Write latency P99.9           | 3.462 µs | 3.450 µs | 1.003x |
| Write latency P99.99          | 1,880.083 µs (1.8 ms) | 1,558.591 µs (1.5 ms) | 1.206x |
| Write latency P99.999         | 1,212,018 µs (1.2 s) | 564,366 µs (0.5 s) | 2.147x |
| **Compactions**
| Average compaction latency    | 1.959 s | 0.904 s | 2.167x |
| Median compaction latency     | 0.832 s | 0.362 s | 2.298x |
| Compaction latency P75        | 1.319 s | 0.596 s | 2.213x |
| Compaction latency P90        | 6.163 s | 2.870 s | 2.147x |
| Compaction tail latency       | 15.586 s | 7.154 s | 2.178x |