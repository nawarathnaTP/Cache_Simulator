all:
	g++ -o cache_sim Cache_Sim.cpp DirectMapped.cpp

clean:
	rm -f cache_sim
