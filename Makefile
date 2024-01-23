GNU_CC = g++
LLVM_CC = armclang++
CFLAGS = -g3 -Wall -mcpu=native -O2 -fopenmp 
LLVMOPT = -armpl=parallel
GNUOPT = -larmpl_lp64_mp

SRC_FILES = ../src/RiemannSiegel.cpp

all: GNU_RiemannSiegel CLANG_RiemannSiegel

GNU_RiemannSiegel: GNU_RiemannSiegel.o
	$(GNU_CC) -o $@ $^ $(CFLAGS)

CLANG_RiemannSiegel: CLANG_RiemannSiegel.o
	$(LLVM_CC) -o $@ $^ $(CFLAGS)

GNU_RiemannSiegel.o: $(SRC_FILES)
	${GNU_CC} ${OPT} -c $^ -o $@ ${CFLAGS} $(GNUOPT)

CLANG_RiemannSiegel.o: $(SRC_FILES)
	${LLVM_CC} ${OPT} -c $^ -o $@ ${CFLAGS} $(LLVMOPT)

clean:
	rm -f GNU_RiemannSiegel CLANG_RiemannSiegel *.o

