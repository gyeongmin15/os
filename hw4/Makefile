all:
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -o dinning_deadlock_original dinning_deadlock_original.c -pthread
	gcc -o dinning_deadlock dinning_deadlock.c -pthread
	gcc -o abba abba.c -pthread
	gcc -o ddchck ddchck.c -pthread
