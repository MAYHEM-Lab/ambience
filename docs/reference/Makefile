all: entries

BUILD ?= build
SRC = $(wildcard *.adoc)
OBJ = $(patsubst %.adoc, $(BUILD)/%.html, $(SRC))

$(BUILD): 
	mkdir -p $(BUILD)

entries: $(BUILD) $(OBJ)

$(BUILD)/%.html: %.adoc
	asciidoctor $< $(ADOC_FLAGS)  -o $@

