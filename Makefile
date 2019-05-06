CXX=g++
CFLAGS=-std=c++11 -O3
LIBS=
VPATH=src
OBJ = iop.o \
main.o

all: $(OBJ)
	$(CXX) $(CFLAGS) $(OBJ) -o moochpsx $(LIBS)

clean:
	rm *.o && rm moochpsx

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<