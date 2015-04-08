all: monte-carlo.cpp
	@echo "Making file..."
	g++ monte-carlo.cpp -pthread -o ../monte-carlo -w
	@echo "done"
	@echo "*** sample usage:  monte-carlo -n 1000 -i 10 -p 1,2,3,4 ***"

clean:
	@$(RM) ../monte-carlo
