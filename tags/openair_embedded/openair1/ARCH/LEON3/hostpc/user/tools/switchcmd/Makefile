
FILENAME=RFswitch set_spi_tx set_spi_rx

CFLAGS+=-I$(OPENAIR)/ARCH/CBMIMO1/DEVICE_DRIVER/ -I$(LEON3)/hostpc/
ifeq ($(debug),yes)
CFLAGS+=-g
endif

all: from_grlib_headers $(FILENAME)

.PHONY: from_grlib_headers
from_grlib_headers:
	@$(MAKE) -C $(PT2) from_grlib_headers

clean:
	rm -Rf $(FILENAME)
	rm -Rf $(addsuffix .[iso],$(FILENAME))
