CXX = g++
CFLAGS = -O3 -std=c++11
# CFLAGS = -g -std=c++11

EXE = loser
OBJ = main.o hash.o loser.o

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ 

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $^

clean:
	rm loser *o