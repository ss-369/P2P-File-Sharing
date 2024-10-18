# Makefile for Peer-to-Peer File Transfer Assignment

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -pthread
LIBS = -lcrypto

# Directories
CLIENT_DIR = client
TRACKER_DIR = tracker

# Executables
CLIENT_EXEC = client_program
TRACKER_EXEC = tracker_program

# Source files
CLIENT_SRCS = $(wildcard $(CLIENT_DIR)/*.cpp)
TRACKER_SRCS = $(wildcard $(TRACKER_DIR)/*.cpp)

# Object files
CLIENT_OBJS = $(CLIENT_SRCS:.cpp=.o)
TRACKER_OBJS = $(TRACKER_SRCS:.cpp=.o)

all: $(CLIENT_EXEC) $(TRACKER_EXEC)

# Compile the client program
$(CLIENT_EXEC): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compile the tracker program
$(TRACKER_EXEC): $(TRACKER_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(CLIENT_OBJS) $(TRACKER_OBJS) $(CLIENT_EXEC) $(TRACKER_EXEC)

.PHONY: all clean
