SRC    := $(wildcard *.c)
HEADER := $(notdir $(SRC:.c=.h))

DOXYGEN_GEN = doxygen
DOXYGEN_CFG = dox.cfg

.PHONY: dox_gen

dox_gen:
	$(DOXYGEN_GEN) $(DOXYGEN_CFG) $(HEADER)
