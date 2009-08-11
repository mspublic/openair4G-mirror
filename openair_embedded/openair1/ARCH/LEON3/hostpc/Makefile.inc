# Local makefile for Software developments, included from $GRLIB/designs/mydesign/Makefile

#       If you want syntax highlighting of this file (using vim editor), append the line below
#       to your <~/.vimrc> settings file:
#                 au BufNewFile,BufRead Makefile.inc setlocal filetype=make

# Local 'from_grlib_headers'
FROM_GRLIB_SOFTCONFIG_H=../../../hostpc/from_grlib_$(notdir $(SoftConfigH))
FROM_GRLIB_SOFTREGS_H=../../../hostpc/from_grlib_$(notdir $(SoftRegsH))
# Update firmware tool
updatefirmwareTOOL=$(UTOOLS)/updatefirmware/updatefw
updatefirmwareSRC=$(UTOOLS)/updatefirmware/updatefw.c

$(FROM_GRLIB_SOFTCONFIG_H): $(SoftConfigH)
	@echo "/* Automatically generated based on $(notdir $<) from Grlib . Don't edit. */" >| $@
	@echo "/* "`date`" */" >> $@
	@awk 'NR>2 {print $0}' $< >> $@
	@sed -i -e 's/#ifndef \([^ ]\{1,\}\)/#ifndef FROM_GRLIB_\1/' $@
	@#sed -i -e 's/#define \([^ ]\{1,\}\)/#define FROM_GRLIB_\1/' $@
	@sed -i -e 's/CFG_/FROM_GRLIB_CFG_/g' $@
	@sed -i -e 's/#endif \/\* \([^ ]\{1,\}\)/#endif \/\* FROM_GRLIB_\1/' $@
	@echo "  GEN   $@"

$(FROM_GRLIB_SOFTREGS_H): $(SoftRegsH)
	@echo "/* Automatically generated based on $(notdir $<) from Grlib . Don't edit. */" >| $@
	@echo "/* "`date`" */" >> $@
	@cat $< >> $@
	@sed -i -e 's/#ifndef \([^ ]\{1,\}\)/#ifndef FROM_GRLIB_\1/' $@
	@#sed -i -e 's/#define \([^ ]\{1,\}\)/#define FROM_GRLIB_\1/g' $@
	@#for i in `grep '#define' $@ | sed -e 's/#define \([^ ]\{1,\}\)/\1/'` ; do sed -i -e 's/$i/FROM_GRLIB_$i/g' ; done
	@echo "  PREP  $(notdir $<) -> $(notdir $@) (takes a few seconds)"
	@for i in `grep '#define' $@ | sed -e 's/#define \([^ ]\{1,\}\).*$$/\1/' | grep -v FROM_GRLIB_` ; do sed -i -e "s/\([^A-Za-z0-9_]\)$$i/\1FROM_GRLIB_$$i/g" $@ ; done
	@#sed -i -e 's/#endif \/\* \([^ ]\{1,\}\)/#endif \/\* FROM_GRLIB_\1/' $@
	@echo "  GEN   $@"

.PHONY: from_grlib_headers
from_grlib_headers: $(FROM_GRLIB_SOFTCONFIG_H) $(FROM_GRLIB_SOFTREGS_H)

$(updatefirmwareTOOL): $(updatefirmwareSRC)
	@gcc -I$(OAIDRV) -g $< -o $@ 2>/dev/null && echo "  GEN   $(notdir $@) tool"

.PHONY: updatefirmware
updatefirmware: $(updatefirmwareTOOL) $(firmwareEXE)
	@echo "  DOING firmware update..."
	@$(updatefirmwareTOOL) --firmware $(firmwareEXE) --stackptr $(call GetSoftConfigHInfo, $(firmwareStackAddress)) \
      $(if $(forcereboot),--forcereboot) \
      $(if $(verbose),-vvv) && echo "  DONE  firmware update"

hostpcclean:
	rm -f $(FROM_GRLIB_SOFTCONFIG_H) $(FROM_GRLIB_SOFTREGS_H)
	rm -f $(updatefirmwareTOOL)
