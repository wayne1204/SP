CXX = g++
CFLAGS = -O3 -std=c++11
CFLAGS = -g -std=c++11 

EXE = loser_peer
OBJ = main.o hash.o peerMgr.o peer.o commit.o

all: $(EXE)

$(EXE): $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ 

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $^

clean:
	rm $(EXE) $(OBJ)
