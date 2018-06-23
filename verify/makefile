-include ../../project/configs/current.configs
-include ../configs/current.configs
PROJ_ROOT?=$(PDIR)
export PROJ_ROOT

all:
	$(MAKE) -C module all
	$(MAKE) -C feature all

install:
	$(MAKE) -C module install
	$(MAKE) -C feature install

clean:
	$(MAKE) -C module clean
	$(MAKE) -C feature clean
