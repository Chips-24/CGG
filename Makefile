# Compiler informations
GNU_CC = g++
GNUOPT = -larmpl_lp64_mp

LLVM_CC = armclang++
LLVMOPT = -armpl=parallel -fsimdmath

# Commons flags
CFLAGS = -Wall -mcpu=native -O3 -fopenmp -lamath

# Activate debug flags
ifdef DEBUG
	CFLAGS += -g3
endif

# Directories
SRC=src
BIN=build
BIN_GCC   = $(BIN)/gcc
BIN_CLANG = $(BIN)/clang

# Get a list of all .c files in the src directory
SRCS = $(wildcard $(SRC)/*.cpp)

# Generate a list of corresponding binary names
BINS = $(notdir $(basename $(SRCS)))

all: gcc clang

gcc:   $(addprefix $(BIN_GCC)/, $(BINS))

clang: $(addprefix $(BIN_CLANG)/, $(BINS))

# Rule to build each binary from its corresponding .cpp file
$(BIN_GCC)/%: $(SRC)/%.cpp | $(BIN_GCC)
	$(GNU_CC) $(CFLAGS) $(GNUOPT) $< -o $@

$(BIN_CLANG)/%: $(SRC)/%.cpp | $(BIN_CLANG)
	$(LLVM_CC) $(CFLAGS) $(LLVMOPT) $< -o $@

# Rule to create the build directory
$(BIN_GCC) $(BIN_CLANG):
	mkdir -p $@

clean:
	rm -rf $(BIN)

.PHONY: clean