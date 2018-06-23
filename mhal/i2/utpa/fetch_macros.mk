include Makefile

setupmacro:
	echo $(CFG_2K_DEFS) $(CFG_CC_DEFS) | tr '= ' ' \n' | xargs -i echo {} | sed 's/-D/#define /' > $(PUBINC)/utopia_macros.h
