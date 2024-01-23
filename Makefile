GNU_CC = g++
LLVM_CC = armclang++
CFLAGS = -g3 -Wall -mcpu=native -O3 -fopenmp -lamath 
LLVMOPT = -armpl=parallel
GNUOPT = -larmpl_lp64_mp

SRC_FILES = ../src/RiemannSiegel_OMP.cpp

all: GNU_RiemannSiegel_OMP CLANG_RiemannSiegel_OMP

GNU_RiemannSiegel_OMP: GNU_RiemannSiegel.o
	$(GNU_CC) -o $@ $^ $(CFLAGS)

CLANG_RiemannSiegel_OMP: CLANG_RiemannSiegel.o
	$(LLVM_CC) -o $@ $^ $(CFLAGS)

GNU_RiemannSiegel.o: $(SRC_FILES)
	${GNU_CC} ${OPT} -c $^ -o $@ ${CFLAGS} $(GNUOPT)

CLANG_RiemannSiegel.o: $(SRC_FILES)
	${LLVM_CC} ${OPT} -c $^ -o $@ ${CFLAGS} $(LLVMOPT)

clean:
	rm -f GNU_RiemannSiegel CLANG_RiemannSiegel *.o

