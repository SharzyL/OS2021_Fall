# Memory

## Q1

Use FIFO algorithm

| test case | time (ms) | miss rate (%) |
|-----------|----------:|--------------:|
| task1     |      31.6 |             0 |
| task2     |    1222.8 |        0.0003 |
| task3     |      92.9 |          28.8 |

## Q2

Use clock algorithm

| test case | time (ms) | miss rate (%) |
|-----------|----------:|--------------:|
| task1     |      32.2 |             0 | 
| task2     |    1215.9 |        0.0003 |
| task3     |      79.5 |          23.7 |

It is observed that on task 1 and task 2, clock algorithm and FIFO performs almost the same. This is because task 1 and task 2 are both sequentially memory access, thus all page fault are caused by compulsory miss. 

## Q3

Vary the number of pages

| number of pages | time (ms) | miss num |
|-----------------|----------:|---------:|
| 1               |    1254.8 |      100 | 
| 2               |    1219.5 |      100 | 
| 3               |    1289.3 |      100 | 
| 4               |    1322.8 |      100 | 
| 5               |    1283.0 |      100 | 
| 6               |    1242.5 |      100 | 
| 7               |    1237.5 |      100 | 
| 8               |    1227.6 |      100 | 
| 9               |    1235.7 |      100 | 
| 10              |    1283.4 |      100 | 

It is observed that increasing the number has almost no impact on performance. Because task 2 is totally sequential access and all page fault are caused by compulsory miss. 

## Q4

| number of threads | time (ms) | miss num |
|-------------------|----------:|---------:|
| 10                |     289.8 |       12 |
| 11                |     316.6 |       24 |
| 12                |     377.2 |       14 |
| 13                |     371.1 |       17 |
| 14                |     425.3 |       36 |
| 15                |     408.1 |       12 |
| 16                |     361.4 |       14 |
| 17                |     386.0 |       19 |
| 18                |     401.9 |       22 |
| 19                |     430.9 |       38 |
| 20                |     449.6 |       19 |

It is observed that increasing the number of threads sublinearly. This is because that CPU intense part is not parallelized but I/O part is parallelized. 