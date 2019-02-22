CXX = g++
CFLAGS = -O3 -std=c++11
# CFLAGS = -g -std=c++11
EXE = char_count
OBJ = fileMgr.o main.o

all:$(OBJ)
	$(CXX) -o $(EXE) $(CFLAGS) $(OBJ)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

clean:
	rm $(OBJ) $(EXE)
