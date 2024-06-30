# TET-KASLR PoC on i7-6700

## Environment

```shell
$ uname -a
Linux XXX 4.15.0-142-generic #146~16.04.1-Ubuntu SMP XXX x86_64 x86_64 x86_64 GNU/Linux

$ lscpu 
Architecture:        x86_64
CPU op-mode(s):       32-bit, 64-bit
Byte Order:          Little Endian
CPU(s):              8
On-line CPU(s) list: 0-7
Thread(s) per core:     2
Core(s) per socket:       4
Socket(s):           1
NUMA node:          1
Vendor ID:            GenuineIntel
CPU family:           6
Model:               94
Model name:          Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
Stepping:               3
CPU MHz:            900.082
CPU max MHz:         4000.0000
CPU min MHz:         800.0000
BogoMIPS:            6799.81
Virtualization:             VT-x
L1d:           32K
L1i:           32K
L2:            256K
L3:            8192K
NUMA node0 CPU(s):   0-7
Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 movbe popcnt aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb invpcid_single tpr_shadow vnmi flexpriority ept vpid fsgsbase tsc_adjust bmi1 hle avx2 bmi2 erms invpcid rtm rdseed adx clflushopt intel_pt xsaveopt xsavec xgetbv1 xsaves dtherm ida arat pln pts hwp hwp_notify hwp_act_window hwp_epp
```

## Experiment Result
```shell
# fault with a mapping address
# e.g. 0xffffffff8127e420 t trace_raw_output_do_sys_open
size: 0,         max_i: 1,       max_i_index: B
size: 1,         max_i: 1,       max_i_index: D
size: 2,         max_i: 1,       max_i_index: S
size: 3,         max_i: 1,       max_i_index: L
size: 4,         max_i: 1,       max_i_index: G
size: 5,         max_i: 1,       max_i_index: T
size: 6,         max_i: 1,       max_i_index: C
size: 7,         max_i: 0,       max_i_index: A
size: 8,         max_i: 1,       max_i_index: N
size: 9,         max_i: 0,       max_i_index: A

# fault with a non-mapping address
# e.g. 0xffff870000000000
size: 0,         max_i: 14,      max_i_index: S
size: 1,         max_i: 15,      max_i_index: S
size: 2,         max_i: 5,       max_i_index: S
size: 3,         max_i: 15,      max_i_index: S
size: 4,         max_i: 13,      max_i_index: S
size: 5,         max_i: 5,       max_i_index: S
size: 6,         max_i: 19,      max_i_index: S
size: 7,         max_i: 11,      max_i_index: S
size: 8,         max_i: 12,      max_i_index: S
size: 9,         max_i: 17,      max_i_index: S
```