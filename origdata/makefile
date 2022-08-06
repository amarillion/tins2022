# Set shell to bash instead of sh allow for brace expansion
SHELL=/bin/bash

DEST=../data
SRC=.

.PHONY: default
default: all ;

SPRITES=bonus switch teleporter raul
SHEET_FILES=$(patsubst %,$(DEST)/%.png,$(SPRITES))
	
$(SHEET_FILES): $(DEST)/%.png: $(SRC)/%.aseprite
	aseprite -b $< --sheet $@

.PHONY: all
all: $(SHEET_FILES)
