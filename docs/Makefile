ADOC_FLAGS= -a nofooter -n -a linkcss
BUILD_DIR=docs

all: $(BUILD) reference $(BUILD_DIR)/index.html

$(BUILD_DIR)/index.html: index.adoc
	asciidoctor index.adoc $(ADOC_FLAGS) -o ./$(BUILD_DIR)/index.html 

reference: 
	$(MAKE) -C reference BUILD=../$(BUILD_DIR)/reference ADOC_FLAGS="$(ADOC_FLAGS)"

.PHONY: reference

clean:
	rm -rf $(BUILD_DIR)
