all:
	@echo "		vanilla - install the vanilla scheduler" 
	@echo "		dynamic - install the dynamic scheduler"	
vanilla:
	cp schedule_vanilla.c /usr/src/servers/sched/schedule.c
	cp schedproc.h /usr/src/servers/sched/schedproc.h
	cd /usr/src/tools && make install
dynamic:
	cp schedule_dynamic.c /usr/src/servers/sched/schedule.c
	cp schedproc.h /usr/src/servers/sched/schedproc.h
	cd /usr/src/tools && make install

