#-------------------------------------------------------------------- settings
FIND          := find
DIR           := $(PWD)/examples
CRITERIA      := \( -name "*.ino" -o -name "*.pde" \)
EACH_EXAMPLE  := $(FIND) $(DIR) $(CRITERIA) -exec
BUILD         := pio ci --verbose
LIB           := "."

#--------------------------------------------------------------------- targets
clean_docs:
	-rm -rf docs

docs:
	@doxygen
	@open docs/html/index.html

# update .travis.yml if target boards added
all: uno megaatmega1280 megaatmega2560 micro leonardo huzzah

uno megaatmega1280 megaatmega2560 micro leonardo huzzah:
	PLATFORMIO_BOARD=$@ $(MAKE) build

build:
	$(EACH_EXAMPLE) $(BUILD) --board=$(PLATFORMIO_BOARD) --lib=$(LIB) {} \;

.PHONY: all uno megaatmega1280 megaatmega2560 micro leonardo huzzah build