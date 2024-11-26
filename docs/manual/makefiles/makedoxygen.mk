DOXYGEN=../../src

.PHONY: $(DOXYGEN) clean

all: $(DOXYGEN)

$(DOXYGEN):
	make -C $@ dox_gen
	make -C dox/$(notdir $@)/latex
	mv dox/$(notdir $@)/latex/refman.pdf $(PROJECT_NAME)_$(notdir $@)_dox.pdf

clean:
	rm -rf dox/$(notdir ${DOXYGEN}) $(addprefix $(PROJECT_NAME)_,$(addsuffix _dox.pdf,$(notdir ${DOXYGEN})))
