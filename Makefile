CXX = g++
CXXFLAGS = -Wall -O3 

SRC=src
BIN=build

# Get a list of all .c files in the src directory
SRCS = $(wildcard $(SRC)/*.cpp)

# Generate a list of corresponding binary names
BINS = $(notdir $(basename $(SRCS)))

all: $(BINS)

# Rule to build each binary from its corresponding .cpp file
%: $(SRC)/%.cpp | $(BIN)
	$(CXX) $(CXXFLAGS) -o $(BIN)/$@ $<

# Rule to create the build directory
$(BIN):
	mkdir -p $@

clean:
	rm -rf $(BIN)

.PHONY: clean