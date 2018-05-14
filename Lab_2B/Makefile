#NAME:Liuyi Shi
#EMAIL:liuyi.shi@outlook.com
#ID:904801945

.SILENT:


profile.out: 
	-rm -f ./raw/gperf
	LD_PRELOAD= /usr/lib64/libprofiler.so.0 CPUPROFILE=./raw.gperf ./lab2_list --thread=12 --iterations=1000 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=worker_thread ./lab2_list ./raw.gperf >>$@
	rm -f ./raw.gperf

profile: profile.out
	cat profile >> profile.out
