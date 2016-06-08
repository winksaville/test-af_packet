.SUFFIXES:

test-af_packet : test-af_packet.c
	gcc -std=c11 $< -o $@

run : test-af_packet
	sudo ./test-af_packet

run-no-sudo : test-af_packet
	./test-af_packet

clean :
	rm test-af_packet
