GNU_CC = g++
LLVM_CC = armclang++
CFLAGS = -g3 -Wall
OPT = -mcpu=native -O2

SRC_FILES = ../src/RiemannSiegel.cpp

all: GNU_RiemannSiegel CLANG_RiemannSiegel

GNU_RiemannSiegel: GNU_RiemannSiegel.o
	$(GNU_CC) -o $@ $^ $(CFLAGS)

CLANG_RiemannSiegel: CLANG_RiemannSiegel.o
	$(LLVM_CC) -o $@ $^ $(CFLAGS)

GNU_RiemannSiegel.o: $(SRC_FILES)
	${GNU_CC} ${OPT} -c $^ -o $@ ${CFLAGS}

CLANG_RiemannSiegel.o: $(SRC_FILES)
	${LLVM_CC} ${OPT} -c $^ -o $@ ${CFLAGS}

clean:
	rm -f GNU_RiemannSiegel CLANG_RiemannSiegel *.o

