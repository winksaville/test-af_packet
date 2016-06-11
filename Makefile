# Parameter ${IFN} is interface name
# Parameter ${IP} is IP address
.SUFFIXES:

test-af_packet : test-af_packet.c
	gcc -Wall -Wpedantic -std=c11 $< -o $@

run : test-af_packet
	sudo ./test-af_packet ${IFN} ${IP}

run-no-sudo : test-af_packet
	./test-af_packet ${IFN} ${IP}

clean :
	@rm test-af_packet
