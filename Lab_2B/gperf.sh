#!/bin/sh

rm -f ./raw.gperf
LD_PRELOAD=/u/eng/class/classihi/bin/lib/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s
pprof --text ./lab2_list ./raw.gperf >profile.out
pprof --list=thread_function_to_run_test ./lab2_list ./raw.gperf >> profile.out
rm -f ./raw.gperf
