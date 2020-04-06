PROJECT = SlideABlob
GCC = gcc -xc++ -lstdc++ -shared-libgcc -Wall

SDL = `sdl-config --cflags --libs`

STATIC = # -static -static-libstdc++ -static-libgcc

BUILD_DIR = output
SRC = 

# different main trails.
MAIN = src/main.cpp

TEST_MAIN = 

HEADER = src/*.h

ICON=res/blobs_icon-alpha.bmp

# ---

build: $(BUILD_DIR)/$(PROJECT) $(BUILD_DIR)/$(PROJECT).desktop

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

$(BUILD_DIR)/$(PROJECT).desktop: $(BUILD_DIR) $(BUILD_DIR)/$(PROJECT).ico
	@echo -e '[Desktop Entry]' > $(BUILD_DIR)/SlideABlob.desktop
	@echo -e 'Name=Slide a Blob' >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e 'Comment=Slide a Lama Clone, made by Nox' >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e 'Encoding=UTF-8' >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e 'Type=Application' >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e 'Categories=Game;' >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e "Path=${PWD}/$(BUILD_DIR)" >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e "Exec=${PWD}/$(BUILD_DIR)/$(PROJECT)" >> $(BUILD_DIR)/SlideABlob.desktop
	@echo -e "Icon=${PWD}/$(BUILD_DIR)/$(PROJECT).ico" >> $(BUILD_DIR)/SlideABlob.desktop

$(BUILD_DIR)/$(PROJECT).ico: $(BUILD_DIR)
	@convert res/blobs_icon.bmp -transparent '#ff00ff' $(BUILD_DIR)/$(PROJECT).ico

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
