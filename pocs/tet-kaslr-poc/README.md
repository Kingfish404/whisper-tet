# TET-CC KASLR

## Overview

In this study, we report a new and novel covert channel that can accomplish transient execution attacks by only timing the transient execution without the help of the cache system or any other shared resources. Hence, we name it **Transient Execution Timing (TET)**. TET is based on the observation that the conditional jump instructions Jcc could influence the execution time of transient executions. We evaluate the proposed TET covert channel on 11 commercial processors and find that all the Intel and AMD processors are affected. Using the TET covert channel, we successfully implemented TET-Meltdown and TET-ZombieLoad attacks. Our experiments indicate that the throughput is up to 50B/s with an error rate of 3%. Furthermore, we successfully exploited the TET to break the KASLR under KPTI.

To find out the root cause, we developed a Performance Monitor Unit (PMU) toolset to reverse engineering and analyze the processor's microarchitecture level behaviors during TET attacks. Through dozens of PMU events in Intel and AMD's CPUs that can be utilized by TET, we conclude that this vulnerability comes from the design of x86 architectures, where macro instructions are decoded into several microinstructions through various components in the frontend. Therefore, when a misprediction arises in transient executions, the decoding process can lead to instruction stream conflicts and result in architecture-visible pipeline stalls that can be distinguished by timing analysis.

## Breaking KASLR

In our experiments, we found that the timing of the transient execution (ToTE)  is easily interfered with by noise. Since transient execution could be trigger by page fault by accessing illegal addresses. We found that if the visual address that triggers the exception is mapped, the ToTE could be distinguish with unmapped. Specifically, the special Jcc jump time difference cannot be distinguished. Referring to previous research [1], Intel will trigger the loading of TLB entries for mapped addresses, even for illegal access, but will not and cannot trigger the loading of TLB entries for addresses that are not mapped.

Based on this result, we can break KASLR through TET, which we call TET-KASLR. During the specific attack process, you can flush or evict the TLB, and then perform a TET probe on different addresses to find the mapped addresses. As a result, we successfully break KASLR in Intel i7-6700, i7-7700, and i9-10980XE. Notice that the i9-10980XE is a meltdown-fallout-resistant cpu. And we successfully break KASLR with KPTI in i9-10980XE.

More detail in [README.md if i9-10980XE](./i9-10980XE/README.md).

[1]: R. Hund, C. Willems, and T. Holz, “Practical Timing Side Channel Attacks against Kernel Space ASLR,” in 2013 IEEE Symposium on Security and Privacy, Berkeley, CA: IEEE, May 2013, pp. 191–205. doi: 10.1109/SP.2013.23.
