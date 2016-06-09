# Parameter ${IFN} is interface name
.SUFFIXES:

test-af_packet : test-af_packet.c
	gcc -std=c11 $< -o $@

run : test-af_packet
	sudo ./test-af_packet ${IFN}

run-no-sudo : test-af_packet
	./test-af_packet ${IFN}

clean :
	rm test-af_packet
