# -------------------------------------------------
TARGET := MultiThreadCapture

# -------------------------------------------------
TOP_DIR := .

SRC_DIR := $(TOP_DIR)/src
SRC_COMMON_DIR := $(TOP_DIR)/src/common
INC_DIR := $(TOP_DIR)
LIB_DIR := $(TOP_DIR)/lib
BIN_DIR := $(TOP_DIR)/bin
OBJ_DIR := $(TOP_DIR)/obj
TEST_DIR := $(TOP_DIR)/tests
LOG_DIR := $(TOP_DIR)/tests/log

# -------------------------------------------------
SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
TEST_SRCS := $(shell find $(TEST_DIR) -name "*.cpp")

INCLUDE := -I$(SRC_DIR) -I$(SRC_COMMON_DIR) -I$(INC_DIR)
THIRD_INCLUDE := -I/usr/local/include/opencv4
TEST_INCLUDE := -I$(TEST_DIR) -I$(SRC_DIR) -I$(INC_DIR) -I/usr/local/include/opencv4

DEFINES :=
TEST_DEFINES := -DNDEBUG
#TEST_DEFINES :=

OBJS := $(SRCS:%=$(OBJ_DIR)/%.lo)
TEST_OBJS := $(TEST_SRCS:%=$(OBJ_DIR)/%.o)

LIBS :=
THIRD_LIBS := -lopencv_core -lopencv_videoio -lopencv_imgcodecs
TEST_LIBS := -l$(TARGET) -lopencv_highgui -pthread -lgtest

LINK_PATH := -L/usr/local/lib
TEST_LINK_PATH := -L$(LIB_DIR)

TARGET_LIB := lib$(TARGET).so
TARGET_BIN := a.out

# -------------------------------------------------
CXXFLAGS := -g -O3 -std=c++2a -Wall -MMD -MP

LDFLAGS := $(LINK_PATH) $(LIBS) $(THIRD_LIBS)
TEST_LDFLAGS := $(TEST_LINK_PATH) $(TEST_LIBS) $(THIRD_LIBS)

CPPFLAGS := $(DEFINES) $(INCLUDE) $(THIRD_INCLUDE)
TEST_CPPFLAGS := $(TEST_DEFINES) $(TEST_INCLUDE) $(THIRD_INCLUDE)

# -------------------------------------------------
CXX := g++ -fPIC
CXX_SHARED := g++ -shared -fPIC

MKDIR = mkdir -p
RM := rm -rvf
LDD := ldd

# -------------------------------------------------

.PHONY: clean target test run


all: target

target: $(LIB_DIR) $(OBJS)
	$(CXX_SHARED) $(CXXFLAGS) -o $(LIB_DIR)/$(TARGET_LIB) $(OBJS) $(LDFLAGS) $(LIBS) $(CPPFLAGS)

test: $(LOG_DIR) $(BIN_DIR) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$(TARGET_BIN) $(TEST_OBJS) $(LDFLAGS) $(TEST_LDFLAGS) $(LIBS) $(TEST_LIBS) $(CPPFLAGS) $(TEST_CPPFLAGS)
	@$(BIN_DIR)/$(TARGET_BIN)

clean:
	-@$(RM) $(LOG_DIR) $(BIN_DIR) $(LIB_DIR) $(OBJ_DIR)
 
ld:
	$(LDD) $(LIB_DIR)/$(TARGET_LIB)

$(OBJ_DIR)/%.cpp.lo: %.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(CPPFLAGS)

$(OBJ_DIR)/%.cpp.o: %.cpp
	@$(MKDIR) $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(TEST_CPPFLAGS)

$(LIB_DIR):
	@$(MKDIR) $(LIB_DIR)

$(BIN_DIR):
	@$(MKDIR) $(BIN_DIR)

$(LOG_DIR):
	@$(MKDIR) $(LOG_DIR)

