.SUFFIXES:

test-af_packet : test-af_packet.c
	gcc -std=c11 $< -o $@

clean :
	rm test-af_packet
