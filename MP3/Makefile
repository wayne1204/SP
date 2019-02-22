CXX = g++
CFLAGS = -O3 -Wall -std=c++11
CFLAGS = -g -Wall -std=c++11 

EXE = csie_box_server csie_box_client
OBJ = csie_box.o

all: $(EXE)

csie_box_server: server.o $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ 

csie_box_client: client.o $(OBJ)
	$(CXX) $(CFLAGS) -o $@ $^ 

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $^

clean:
	rm $(EXE) $(OBJ) server.o client.o
