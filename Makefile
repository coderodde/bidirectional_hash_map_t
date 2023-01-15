all: main.c bidirectional_hash_map.c bidirectional_hash_map.h bidirectional_hash_map_2.c bidirectional_hash_map_2.h
	gcc -o demo -O3 -Wall -Werror -Wfatal-errors 1 -Wno-error=int-to-pointer-cast -Wno-error=pointer-to-int-cast -pedantic -std=c89 main.c bidirectional_hash_map.c bidirectional_hash_map_2.c 
