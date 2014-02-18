all:
	echo dynamic - dynamic lottery scheduler
	echo vanilla - plain lottery scheduler

dynamic: schedproc.h
	cp schedule_dynamic.c /usr/src/servers/sched/schedule.c

vanilla: schedproc.h
	cp schedule.c /usr/src/servers/sched/

schedproc.h:
	cp schedproc.h /usr/src/servers/sched

