# simple makefile

MKDIR_P = mkdir -p

# main directories
BIN_DIR = ./bin
OBJ_DIR = ./build
SRC_DIR = ./src
INC_DIR = ./include
LIB_DIR = ./libs

# lexer options
LEXER = reflex
LEXER_FLAGS =

PARSER_DIR = $(SRC_DIR)/parser
BL_PARSER_LEX = $(PARSER_DIR)/blif_parser.l
BL_PARSER_CPP = $(SRC_DIR)/parser/blif_parser.cpp
BL_PARSER_INC = $(INC_DIR)/parser/blif_parser.h


# sub project directories
INC_DIRS = $(shell find $(INC_DIR) -type d)

# compiler and linker options
EXE_NAME = cad

CXX = g++

CXX_FLAGS = -W -Wall -Wextra -s -g -O3 -static -std=c++17 -DABC_NAMESPACE=ABC -DLIN64
INC_FLAGS = $(addprefix -I,$(INC_DIRS))
LD_FLAGS = -static
# LIB_FLAGS = -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lreflex -lm -ldl -rdynamic -lz
LIB_FLAGS = -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -lreflex -lm -ldl -rdynamic

# collect sources ...
SRCS = $(shell find $(SRC_DIR) -name "*.cpp")
OBJS = $(SRCS:%.cpp=$(OBJ_DIR)/%.o)
#DEPS = $(OBJS:.o=.d)


# rules for c++ files
$(OBJ_DIR)/%.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(INC_FLAGS) $(CXX_FLAGS) -c $< -o $@

#rules for target
$(BIN_DIR)/$(EXE_NAME): $(OBJS)
	$(MKDIR_P) $(BIN_DIR)
	$(CXX) $(LD_FLAGS) $(OBJS) $(LIBS) $(LIB_FLAGS) -o $@


.PHONY: parser clean

parser:
	$(MKDIR_P) $(PARSER_DIR)
	$(LEXER) -o $(BL_PARSER_CPP) --header-file=$(BL_PARSER_INC) $(BL_PARSER_LEX)
#	sed -i 's/reflex_code_INITIAL/bl_reflex_code_INITIAL/g' $(BL_PARSER_CPP)

clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN_DIR)/*
