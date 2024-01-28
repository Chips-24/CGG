# Compiler information
GNUCXX      = g++
GNUCXXFLAGS = -ftree-vectorize -funroll-loops -mcpu=native

LLVMCXX     = clang++
LLVMCXXFLAGS= -fvectorize

GNUCC	   = gcc
GNUCCFLAGS = -lm -DC_PROG -ftree-vectorize -funroll-loops -mcpu=native

# Common flags
CFLAGS     = -Wall -Wextra -pedantic -O3 -fopenmp

# Debug flags
DFLAGS     = -g3
ifdef DEBUG
	CFLAGS+= $(DFLAGS)
endif

# ARM specific flags
ifdef ARM
	CFLAGS+= -DARM -lamath
	GNUCXXFLAGS+= -larmpl_lp64_mp 
	LLVMCXX= armclang++
	LLVMCXXFLAGS+= -armpl=parallel -mcpu=neoverse-v2 -fsimdmath
	GNUCCFLAGS+= -larmpl_lp64_mp
endif

# Directories
SRC=src
BIN=build

BIN_GNUPP=$(BIN)/gnu++
BIN_LLVM =$(BIN)/llvm++
BIN_GNUCC=$(BIN)/gnu

# Get all source files
SRCS_CPP=$(wildcard $(SRC)/*.cpp)
SRCS_C  =$(wildcard $(SRC)/*.c)

# Generate list of coresponding binaries
BINS_CPP=$(notdir $(basename $(SRCS_CPP)))
BINS_C  =$(notdir $(basename $(SRCS_C)))

all: gnupp llvmpp gnucc

gnupp: $(addprefix $(BIN_GNUPP)/, $(BINS_CPP)) $(addprefix $(BIN_GNUPP)/, $(BINS_C))

llvmpp: $(addprefix $(BIN_LLVM)/, $(BINS_CPP)) $(addprefix $(BIN_LLVM)/, $(BINS_C))

gnucc: $(addprefix $(BIN_GNUCC)/, $(BINS_C))

# Rules to build each binary
# 	c++ GNU
$(BIN_GNUPP)/%: $(SRC)/%.cpp | $(BIN_GNUPP)
	$(GNUCXX) $(CFLAGS) $(GNUCXXFLAGS) -o $@ $<

$(BIN_GNUPP)/%: $(SRC)/%.c | $(BIN_GNUPP)
	$(GNUCXX) $(CFLAGS) $(GNUCXXFLAGS) -o $@ $<

# 	c++ LLVM
$(BIN_LLVM)/%: $(SRC)/%.cpp | $(BIN_LLVM)
	$(LLVMCXX) $(CFLAGS) $(LLVMCXXFLAGS) -o $@ $<

$(BIN_LLVM)/%: $(SRC)/%.c | $(BIN_LLVM)
	$(LLVMCXX) $(CFLAGS) $(LLVMCXXFLAGS) -o $@ $<

# 	c GNU
$(BIN_GNUCC)/%: $(SRC)/%.c | $(BIN_GNUCC)
	$(GNUCC) $(CFLAGS) $(GNUCCFLAGS) -o $@ $<

# Create build directories
$(BIN_GNUPP) $(BIN_LLVM) $(BIN_GNUCC):
	mkdir -p $@

# Clean up
clean:
	rm -rf $(BIN)

