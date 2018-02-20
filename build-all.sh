#!/bin/bash

benchmarks=(hello 400.perlbench.test 401.bzip2.test 403.gcc.test 429.mcf.test)

for b in ${benchmarks[@]}; do
  make -C riscv-hpmcounters
  cp riscv-hpmcounters/hpm_counters $b/root/
  rm -rf buildroot/output/target/root
  rm -rf buildroot-overlay
  cp -r $b buildroot-overlay
  ./build.sh
  mv bbl-vmlinux bblvmlinux-$b
done
