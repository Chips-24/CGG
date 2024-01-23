CXX = g++
CXXFLAGS = -Wall -O3 

# Valid flags : -DSILENT
OPTFLAGS =

SRC=src
BIN=build

# Get a list of all .c files in the src directory
SRCS = $(wildcard $(SRC)/*.cpp)

# Generate a list of corresponding binary names
BINS = $(notdir $(basename $(SRCS)))

all: $(BINS)

# Rule to build each binary from its corresponding .cpp file
%: $(SRC)/%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -o $(BIN)/$@ $<

# Rule to create the build directory
$(BIN):
	mkdir -p $@

clean:
	rm -rf $(BIN)

.PHONY: clean

COMPFILES = RiemannSiegel
COMPARGS = 10 10000 100 10
# Compare rule
compare: $(addprefix run_,$(COMPFILES))

# Rule to run each binary
run_%: $(BIN)/%
	$(BIN)/$* $(COMPARGS)