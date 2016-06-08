test-af_packet : test-af_packet.c
	gcc $< -o $@

clean :
	rm test-af_packet
