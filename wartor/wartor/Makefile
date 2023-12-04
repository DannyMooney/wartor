CXX = g++
CPPFILES = main.cpp graph.cpp
CFLAGS = -std=c++11 -Wall -O2 -fopenmp
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -lgomp
EXE = wator

SRCS = main.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

%.o: %.cpp 
	$(CXX) -c -o $@ $< $(CPPFLAGS) $(DEBUGFLAGS)

ALL: $(OBJS)
	$(CXX) $(OBJS) -o $(EXE) $(LDFLAGS)
