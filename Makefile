CPP=clang++ -std=c++14 -g -O0 -ferror-limit=1
I=-I.
W=-Wall -Wno-logical-op-parentheses -Wno-missing-braces -Wno-unused-function
SRCS=$(wildcard *.cpp)
EXE=dom

.PHONY: all
all: $(EXE)

.PHONY: clean
clean:
	rm -f *.o $(EXE)

$(EXE): $(SRCS:.cpp=.o)
	$(CPP) -o $@ $^

%.d: %.cpp
	$(CPP) $(I) -MMD -MP $(W) -c $<

include $(SRCS:.cpp=.d)

%.o: %.cpp
	$(CPP) $(I) $(W) -c $<
