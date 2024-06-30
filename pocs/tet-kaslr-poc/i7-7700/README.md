# TET-KASLR PoC on i7-7700

## Environment

```shell
$ uname -a
Linux XXX 5.4.0-150-generic #167~18.04.1-Ubuntu SMP XXX x86_64 x86_64 x86_64 GNU/Linux

$ lscpu
Architecture:        x86_64
CPU op-mode(s):      32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              8
On-line CPU(s) list: 0-7
Thread(s) per core:  2
Core(s) per socket:  4
Socket(s):           1
NUMA node(s):        1
Vendor ID:           GenuineIntel
CPU family:          6
Model:               158
Model name:          Intel(R) Core(TM) i7-7700 CPU @ 3.60GHz
Stepping:            9
CPU MHz:             900.012
CPU max MHz:         4200.0000
CPU min MHz:         800.0000
BogoMIPS:            7200.00
Virtualization:      VT-x
L1d cache:           32K
L1i cache:           32K
L2 cache:            256K
L3 cache:            8192K
NUMA node0 CPU(s):   0-7
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust bmi1 hle avx2 bmi2 erms invpcid rtm rdseed adx clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp
```

## Experiment Result
```shell
$ gcc -o tet-cc-kaslr tet-cc-kaslr.c
$ taskset -c 1 ./tet-cc-kaslr
0xffff8772fe6e21e3, argc: 0
size: 0,         max_i: 30,      max_i_index: Y
size: 1,         max_i: 32,      max_i_index: B
size: 2,         max_i: 33,      max_i_index: U
size: 3,         max_i: 31,      max_i_index: S
size: 4,         max_i: 36,      max_i_index: U
size: 5,         max_i: 31,      max_i_index: W
size: 6,         max_i: 30,      max_i_index: A
size: 7,         max_i: 27,      max_i_index: X
size: 8,         max_i: 31,      max_i_index: V
size: 9,         max_i: 26,      max_i_index: D
idx: 1
0xffff8882fe6e21e3, argc: 1486453960
size: 0,         max_i: 96,      max_i_index: T
size: 1,         max_i: 77,      max_i_index: T
size: 2,         max_i: 90,      max_i_index: T
size: 3,         max_i: 64,      max_i_index: T
size: 4,         max_i: 71,      max_i_index: T
size: 5,         max_i: 51,      max_i_index: T
size: 6,         max_i: 73,      max_i_index: T
size: 7,         max_i: 57,      max_i_index: T
size: 8,         max_i: 64,      max_i_index: T
size: 9,         max_i: 62,      max_i_index: T
idx: 2
0xffff8892fe6e21e3, argc: 1486453960
size: 0,         max_i: 27,      max_i_index: U
size: 1,         max_i: 30,      max_i_index: U
size: 2,         max_i: 29,      max_i_index: W
size: 3,         max_i: 34,      max_i_index: S
size: 4,         max_i: 28,      max_i_index: W
size: 5,         max_i: 24,      max_i_index: U
size: 6,         max_i: 24,      max_i_index: T
size: 7,         max_i: 36,      max_i_index: V
size: 8,         max_i: 35,      max_i_index: U
size: 9,         max_i: 26,      max_i_index: T
```