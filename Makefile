PROJECT = SlideALama-Clone
GCC = gcc -xc++ -lstdc++ -shared-libgcc -Wall

SDL = `sdl-config --cflags --libs`

STATIC = # -static -static-libstdc++ -static-libgcc

BUILD_DIR = output
SRC = 

# different main trails.
MAIN = src/main.cpp

TEST_MAIN = 

HEADER = src/*.h

# ---

build: $(BUILD_DIR)/$(PROJECT)
$(BUILD_DIR)/$(PROJECT): $(SRC) $(MAIN) $(HEADER) $(BUILD_DIR)
	@echo "SDL build."
	$(GCC) -o $(BUILD_DIR)/$(PROJECT) $(SDL) $(SRC) $(MAIN)

test: $(SRC) $(TEST_MAIN) $(HEADER) $(BUILD_DIR)
	@echo "Test."
	$(GCC) $(STATIC) -o $(BUILD_DIR)/$(PROJECT) $(SRC) $(TEST_MAIN)

run: $(BUILD_DIR)/$(PROJECT) res/field_colours.bmp
	cd $(BUILD_DIR) && ./$(PROJECT)

res/field_colours.bmp: res/field_colours.sh
	@echo -e "Update Field Tiles (colours)."
	@bash res/field_colours.sh

.PHONY: $(BUILD_DIR)
$(BUILD_DIR):
	@mkdir -vp $(BUILD_DIR)/res
	@cp -uvr res/*.bmp $(BUILD_DIR)/res

# ---

clean:
	rm -vrf $(BUILD_DIR)

options:
	@echo "- build ........ build"
	@echo "- test ......... test"
	@echo "- clean ........ remove the built directory"
