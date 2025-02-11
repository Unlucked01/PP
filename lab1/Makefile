CXX = g++
CXXFLAGS = -Wall -pthread -std=c++17
BUILD_DIR = build
SRCS = src/main.cpp src/config.cpp src/queue.cpp src/generator.cpp src/service.cpp
OBJS = $(SRCS:src/%.cpp=$(BUILD_DIR)/%.o)
TARGET = gas_station

# Debug configuration
ifdef DEBUG
    CXXFLAGS += -DDEBUG -g
endif

all: create_dirs $(TARGET)

create_dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p logs

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(CXXFLAGS)

$(BUILD_DIR)/%.o: src/%.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)

clean:
	rm -rf $(BUILD_DIR)
	rm -f $(TARGET)
	rm -f logs/*.log

run: $(TARGET)
	./$(TARGET)

debug: clean
	$(MAKE) DEBUG=1
	./$(TARGET)

.PHONY: all clean run debug create_dirs