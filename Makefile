all:
	g++ -o cache_sim Cache_Sim.cpp DirectMapped.cpp SetAssociative.cpp FullyAssociative.cpp

clean:
	rm -f cache_sim
