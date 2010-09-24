VER=1.0.10
GVER=1010
CC=gcc
SED=sed

VLIB = vlib
VCOM = vcom -quiet -93 -work
VLOG = vlog -quiet -work

NCVHDL = ncvhdl -nowarn DLCPTH -NOVITALCHECK -linedebug -v93 -nocopyright -work
NCVLOG = ncvlog -nowarn DLCPTH -nocopyright -work

DCVHDL = analyze -f VHDL -library
DCVLOG = analyze -f VERILOG -library
DCSCRIPT=$(TOP).dc
DCSKIP = unisim clkgen_xilinx clkgen_altera clkbuf_xilinx altera_gen xilinx_gen lattice_gen

RTLCVHDL = read_hdl -vhdl -lib
RTLCVLOG = read_hdl
RCSCRIPT=$(TOP).rc

SYNPVHDL = add_file -vhdl -lib
SYNPVLOG = add_file -verilog

#PRECVHDL = add_input_file

XSTVHDL = elaborate -ifmt vhdl -work_lib
XSTVLOG = elaborate -ifmt verilog -work_lib
XSTLIBSKIP = unisim | core1553* | gr1553 | altera* | actel | atmel | virage | umc | \
	axcelerator | rh_lib18t* | ihp25 | synplify | ec | fmf | topnet | codeclib | \
	micron | cypress | spacelib | spw2lib | apa | apa3
XSTSKIP = unisim | grfpu* | fpmul* | grfpc* | *altera* | *atc18* | *virage* | *actel* | \
	*umc_gen* | *lib18t* | *1553* | *ihp25* | *lattice* | *synplify*

PRECLIBSKIP = unisim | core1553* | gr1553 | altera* | actel | atmel | virage | umc | \
	axcelerator | rh_lib18t* | ihp25 | synplify | ec | fmf | topnet | codeclib | \
	micron | cypress | spacelib | spw2lib | apa | apa3
PRECSKIP = unisim | *atc18* | *virage* | *umc_gen* | *lib18t* | *1553* | *ihp25* 


GHDL = ghdl -a --ieee=standard
GHDLE = ghdl -e --ieee=standard
GHDLM = ghdl --gen-makefile --ieee=standard
GHDLSKIP = ahb2hpi grhcan grpulse orca.vhd

SYNPLIFY = synplify_pro

DESIGNER = designer
OS = `uname`

UNISIM = $(XILINX)/vhdl/src/unisims/unisim_VPKG.vhd  \
	$(XILINX)/vhdl/src/unisims/unisim_VCOMP.vhd \
	$(XILINX)/vhdl/src/unisims/unisim_VITAL.vhd

SIMPRIM = $(XILINX)/vhdl/src/simprims/simprim_Vpackage.vhd  \
	$(XILINX)/vhdl/src/simprims/simprim_Vcomponents.vhd \
	$(XILINX)/vhdl/src/simprims/simprim_VITAL.vhd


ifeq ("$(QUARTUS_ROOTDIR)","")
QUARTUS_MAP = quartus_map
QUARTUS_FIT = quartus_fit
QUARTUS_ASM = quartus_asm
QUARTUS_TAN = quartus_tan
else
QUARTUS_MAP = $(QUARTUS_ROOTDIR)/bin/quartus_map
QUARTUS_FIT = $(QUARTUS_ROOTDIR)/bin/quartus_fit
QUARTUS_ASM = $(QUARTUS_ROOTDIR)/bin/quartus_asm
QUARTUS_TAN = $(QUARTUS_ROOTDIR)/bin/quartus_tan
endif
QUARTUS_SKIP =  actel_gen atc18_gen virage_gen umc_gen axcel ramon lib18t_gen \
	core1553b gr1553 b1553 clkgen_actel clkbuf_actel mem_ihp25_gen.vhd \
	mem_lattice_gen.vhd mem_xilinx_gen.vhd pad_xilinx_gen.vhd \
	pad_ihp25_gen.vhd clkgen_xilinx.vhd tap_xilinx_gen.vhd

SYN=synplify
SIM=vsim
PR=ise

PRECISION=precision

ifeq ("$(SIMTOP)","")
SIMTOP=$(TOP)
endif

all: help

help:
	@echo -n  "Help from Gaisler's GRLIB, or Eurecom design's help ? (g/e) "
	@(read helpans ; \
	if [ $$helpans = g -o  $$helpans = G ] ; then \
	   make -s gaislerhelp ; \
	else make -s localhelp; fi )

gaislerhelp:
	@echo
	@echo " interactive targets:"
	@echo
	@echo " make vsim-launch          : start modelsim"
	@echo " make ncsim-launch         : compile design using ncsim"
	@echo " make actel-launch-synp    : start Actel Designer for current project"
	@echo " make ise-launch           : start ISE project navigator for XST project"
	@echo " make ise-launch-synp      : start ISE project navigator for synplify project"
	@echo " make quartus-launch       : start Quartus for current project"
	@echo " make quartus-launch-synp  : start Quartus for synplify project"
	@echo " make synplify-launch      : start synplify"
	@echo " make xgrlib               : start grlib GUI"
	@echo
	@echo " batch targets:"
	@echo
	@echo " make vsim         : compile design using modelsim"
	@echo " make ncsim        : compile design using ncsim"
	@echo " make ghdl         : compile design using GHDL"
	@echo " make actel        : synthesize with synplify, place&route Actel Designer"
	@echo " make ise          : synthesize and place&route with Xilinx ISE"
	@echo " make ise-map      : synthesize design using Xilinx XST"
	@echo " make ise-prec     : synthesize with precision, place&route with Xilinx ISE"
	@echo " make ise-synp     : synthesize with synplify, place&route with Xilinx ISE"
	@echo " make isp-synp     : synthesize with synplify, place&route with ISPLever"
	@echo " make quartus      : synthesize and place&route using Quartus"
	@echo " make quartus-map  : synthesize design using Quartus"
	@echo " make quartus-synp : synthesize with synplify, place&route with Quartus"
	@echo " make precision    : synthesize design using precision"
	@echo " make synplify     : synthesize design using synplify"
	@echo " make scripts      : generate compile scripts only"
	@echo " make clean        : remove all temporary files except scripts"
	@echo " make distclean    : remove all temporary files"
	@echo

make xgrlib:
	$(GRLIB)/bin/xgrlib.tcl $(TOP) $(TECHNOLOGY) $(DEVICE) $(BOARD)

############  AHB ROM Geneartion     ########################

FILE=$(promEXE)

ahbrom: $(GRLIB)/bin/ahbrom.c
	@$(CC) $(GRLIB)/bin/ahbrom.c -o ahbrom

ahbrom.vhd: ./ahbrom $(FILE)
	@sparc-elf-objcopy -O binary $(FILE) ahbrom.bin
	@./ahbrom ahbrom.bin ahbrom.vhd

#########    Generic simulation target ###############

sim:
	make $(SIM)

sim-run:
	make $(SIM)-run

sim-launch:
	make $(SIM)-launch

#########    Modelsim targets   ############

#vsim commented here, because simulation now handled through $GRLIB/designs/leon3-eurecom-platform-2/sim/Makefile.inc
#vsim: make.work
#	@make -f make.work

make.work: compile.vsim modelsim
	@make -f make.vsim
	@echo "" > make.work
	@for i in `cat libs.txt`; do vmake $$i >> make.work ; done

modelsim: compile.vsim
	@vsim -c -quiet -do "do libs.do; quit"

vsim-grlib: modelsim
	make vsim
#	@for i in `cat libs.txt`; do \
#	  make -f modelsim/make.$$i ; \
#	done ;

vsim-run: vsim
	@vsim -c $(SIMTOP) -do $(vsimBATCH)

vsim-launch: scripts modelsim
	@vsim -gui -do "vsim $(SIMTOP); do  $(vsimGUI)" &

vsim-unisim: modelsim
	$(VCOM) unisim $(UNISIM)
#	-vlib modelsim/simprim
#	$(VCOM) simprim -ignorevitalerrors $(SIMPRIM)

vsim-clean:
	-rm -rf modelsim transcript *.mti stdout.log vsim.wlf \
	$(SIMTOP).mpf.bak $(SIMTOP).mti *.mpf

#########   GHDL targets   ############

ghdl $(SIMTOP): make.ghdl
	make -f make.ghdl GHDLFLAGS="--workdir=gnu/work --work=work `cat ghdl.path`"

make.ghdl: compile.ghdl
	-rm -rf gnu
	make -f tmpmake.ghdl ghdl
	$(GHDLE) $(VHDLOPT) --workdir=gnu/work --work=work `cat ghdl.path` $(SIMTOP)
	$(GHDLM) $(VHDLOPT) --workdir=gnu/work --work=work `cat ghdl.path` $(SIMTOP) > make.ghdl
	cat tmpmake.ghdl >> make.ghdl

ghdl-run ghdl-launch: $(TOP)
	./$(SIMTOP)

ghdl-clean:
	-rm -rf gnu $(SIMTOP) make.ghdl

#########   NcSim targets   ############

#ncsim commented here, because simulation now handled through $GRLIB/designs/leon3-eurecom-platform-2/sim/Makefile.inc
#ncsim: xncsim
#	ncupdate -VERBOSE $(SIMTOP)

ncsim-run: ncsim
	ncelab $(SIMTOP)
	ncsim -input $(ncsimBATCH) $(SIMTOP)

ncsim-launch: ncsim
	ncelab $(SIMTOP)
	ncsim -gui -input $(ncsimGUI) $(SIMTOP) &

xncsim: compile.ncsim
	-rm -rf xncsim
	make -f make.ncsim

ncsim-clean:
	-rm -rf xncsim nc*.log ncsim.key

#########   Xilinx ISE targets   ############

ise: checkboardvers $(UCF) $(TOP).ngc
	$(GRLIB)/bin/route_ngc $(TOP) $(UCF) $(DEVICE) $(EFFORT) $(BITGEN) $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)
	@{ endtime=`date +%s` ; time=$$(($$endtime - $(starttime))) ; \
       hours=$$(($$time / 3600)) ; \
       minutes=$$((($$time - ($$hours * 3600))/60)) ; \
       echo ; echo ; echo Total design flow took $${hours}h:$${minutes}min ; }

ise-synp: checkboardvers $(UCF) $(TOP)_synplify.prj synplify/$(TOP).edf
	$(GRLIB)/bin/route $(TOP) $(UCF) $(DEVICE) $(EFFORT) $(BITGEN) synplify \
	$(GRLIB)/netlists/xilinx/$(TECHNOLOGY)
	@{ endtime=`date +%s` ; time=$$(($$endtime - $(starttime))) ; \
       hours=$$(($$time / 3600)) ; \
       minutes=$$((($$time - ($$hours * 3600))/60)) ; \
       echo ; echo ; echo Total design flow took $${hours}h:$${minutes}min ; }

ise-prec: checkboardvers $(UCF) $(TOP).psp precision/$(TOP).edf
	$(GRLIB)/bin/route $(TOP) $(UCF) $(DEVICE) $(EFFORT) $(BITGEN) precision $(BOARD_VERSION) \
	$(GRLIB)/netlists/xilinx/$(TECHNOLOGY)
	@{ endtime=`date +%s` ; time=$$(($$endtime - $(starttime))) ; \
       hours=$$(($$time / 3600)) ; \
       minutes=$$((($$time - ($$hours * 3600))/60)) ; \
       echo ; echo ; echo Total design flow took $${hours}h:$${minutes}min ; }

# This was my Precision support (KMK)
#ise-prec: $(UCF) $(TOP).edf
#	@$(GRLIB)/bin/route $(TOP) $(UCF) $(DEVICE) $(EFFORT) $(BITGEN) . \
#	  $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)

ise-launch xst-launch: $(TOP).ise
	ise $(TOP).ise>& ise.err&

ise7-launch xst7-launch: $(TOP).ise
	ise $(TOP).ise>& ise.err&

ise-launch-synp: $(TOP).npl synplify/$(TOP).edf
	ise $(TOP)_synplify.npl>& ise.err&

$(TOP).xst: compile.xst
	@touch $(TOP).xst; rm $(TOP).xst
	@for i in $(VHDLSYNFILES); do $(GRLIB)/bin/xstvhdl $$i >> $(TOP).xst ; done
#	@for i in $(VERILOGSYNFILES); do $(GRLIB)/bin/xstverilog $ii >> $(TOP).xst; done
	@$(GRLIB)/bin/xstrun $(TOP) $(TopVHDL) $(DEVICE) $(XSTOPT) >> $(TOP).xst
	@echo  $(TOP).xst

ise-xstmods xstmods: verilog.txt
	@cat verilog.txt | sed -e s/'\.v'// > xstmods
	@for i in `cat xstmods`; do xmod=`basename $$i`; \
	    $(GRLIB)/bin/xstmod $$xmod $$i.v $(DEVICE) verilog  > tmp.xst; \
	    xst -ifn tmp.xst; \
	done

$(TOP).bit: $(UCF) $(TOP).ngc
	$(GRLIB)/bin/route_ngc $(TOP) $(UCF) $(DEVICE) $(EFFORT) $(BITGEN) $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)

ise-map xst-map ise-xst xst: $(TOP).ngc

ifdef resyn
$(TOP).ngc: $(TOP).xst $(VHDLSYNFILES) $(VERILOGSYNFILES) compile.xst
	-rm -rf xst
	xst -ifn compile.xst
	xst -ifn $(TOP).xst
else
# To avoid make to systematically launch xst again each time we modify devhard/configmore.vhd, we remove it out from
# the prerequisites of $(TOP).ngc.
# SO update devhard/configmore.vhd manually before each syntesis (by doing a 'make local')
VHDLSYNFILES_WO_CONFIGMORE=$(subst $(ConfigMoreVHDL),,$(VHDLSYNFILES))
#$(TOP).ngc: $(TOP).xst $(VHDLSYNFILES) $(VERILOGSYNFILES) compile.xst
$(TOP).ngc: $(TOP).xst $(VHDLSYNFILES_WO_CONFIGMORE) $(VERILOGSYNFILES) compile.xst
	-rm -rf xst
	xst -ifn compile.xst
	xst -ifn $(TOP).xst
endif

ise-prom:
ifeq ("$(PROMGENPAR)","")
	@echo "not prom programming support for this board"
	@exit 1
else
	promgen $(PROMGENPAR)
endif

ise-cp-ref:
	cp bitfiles/*.* .

ise-prog-prom: ise-prom
	impact -batch $(GRLIB)/boards/$(BOARD)/prom.cmd

ise-prog-prom-ref: ise-cp-ref ise-prom
	impact -batch $(GRLIB)/boards/$(BOARD)/prom.cmd

ise-prog-fpga:
	cp $(TOP).bit $(BOARD).bit
	cp $(TOP).msk $(BOARD).msk
	impact -batch $(GRLIB)/boards/$(BOARD)/fpga.cmd

ise-prog-fpga-ref: ise-cp-ref
	cp $(TOP).bit $(BOARD).bit
	cp $(TOP).msk $(BOARD).msk
	impact -batch $(GRLIB)/boards/$(BOARD)/fpga.cmd

ise-clean xst-clean:
	@if [ -f $(TOP).srp ] ; then echo "Saving $(TOP).srp as $(TOP)_srp.sav" ; mv $(TOP).srp $(TOP)_srp.sav ; fi
	@if [ -f $(TOP).mrp ] ; then echo "Saving $(TOP).mrp as $(TOP)_mrp.sav" ; mv $(TOP).mrp $(TOP)_mrp.sav ; fi
	@if [ -f $(TOP).par ] ; then echo "Saving $(TOP).par as $(TOP)_par.sav" ; mv $(TOP).par $(TOP)_par.sav ; fi
	@if [ -f $(TOP).twr ] ; then echo "Saving $(TOP).twr as $(TOP)_twr.sav" ; mv $(TOP).twr $(TOP)_twr.sav ; fi
	-rm -rf xst *.srp xstmods *.twr *.pad _impact* coregen.log \
	tmp.xst *.ngc __projnav* *.stx *.lso *.dhp automake.log \
	$(TOP).prj  $(TOP)_vhdl.prj  coregen* *.bit *.bgn *.bld \
	*.cmd_log *.ll *.mrp *.msk *.ncd *.ngd *.syr *.xpi *.csv \
	*pad.txt *.ngm *.lst *.drc *.par *.pcf *.ngo *.mcs *.prm \
	*.sig *.rba *.rbb *.rbd *.rbt *.msd _ngo _impact* stdout.log \
	ngd2vhdl.log ise.txt ise_win2.txt ise_win32.txt $(TOP).ise* \
	_xmsgs tmp.txt $(TOP).unroutes $(TOP)_usage.xml
	@if [ -f $(TOP)_srp.sav ] ; then echo "Restoring $(TOP)_srp.sav as $(TOP).srp" ; mv $(TOP)_srp.sav $(TOP).srp ; fi
	@if [ -f $(TOP)_mrp.sav ] ; then echo "Restoring $(TOP)_mrp.sav as $(TOP).mrp" ; mv $(TOP)_mrp.sav $(TOP).mrp ; fi
	@if [ -f $(TOP)_par.sav ] ; then echo "Restoring $(TOP)_par.sav as $(TOP).par" ; mv $(TOP)_par.sav $(TOP).par ; fi
	@if [ -f $(TOP)_twr.sav ] ; then echo "Restoring $(TOP)_twr.sav as $(TOP).twr" ; mv $(TOP)_twr.sav $(TOP).twr ; fi

#########   Altera Quartus targets   ############

quartus:
	make quartus-map quartus-route

quartus-synp:  $(TOP)_synplify.prj synplify/$(TOP).edf
	-@case $(TECHNOLOGY) in \
	CYCLONEII) \
		cp  $(GRLIB)/netlists/altera/cyclone2/*.vqm . ;;\
	STRATIXII) \
		cp  $(GRLIB)/netlists/altera/stratix2/*.vqm . ;;\
	esac
	$(QUARTUS_MAP) --import_settings_files=on  --export_settings_files=off $(TOP)_synplify -c $(TOP)_synplify
	$(QUARTUS_FIT) --import_settings_files=off --export_settings_files=off $(TOP)_synplify -c $(TOP)_synplify
	$(QUARTUS_ASM) --import_settings_files=off --export_settings_files=off $(TOP)_synplify -c $(TOP)_synplify
	$(QUARTUS_TAN) --import_settings_files=off --export_settings_files=off $(TOP)_synplify -c $(TOP)_synplify --timing_analysis_only

quartus-launch: $(TOP).qsf
	quartus  $(TOP).qpf

quartus-launch-synp:  $(TOP)_synplify.prj synplify/$(TOP).edf
	quartus $(TOP)_synplify.qpf

quartus-map: $(TOP).qsf
	$(QUARTUS_MAP) --import_settings_files=on  --export_settings_files=off $(TOP) -c $(TOP)

quartus-route: $(TOP).qsf
	$(QUARTUS_FIT) --import_settings_files=off --export_settings_files=off $(TOP) -c $(TOP)
	$(QUARTUS_ASM) --import_settings_files=off --export_settings_files=off $(TOP) -c $(TOP)
	$(QUARTUS_TAN) --import_settings_files=off --export_settings_files=off $(TOP) -c $(TOP) --timing_analysis_only

quartus-clean:
	-rm -rf db *.syr *.qws automake.log dumpdata.txt \
	*.rpt *.done *.eqn *.pof *.summary *.ttf *.pin *.sof \
	*.jam *.jbc cmp_state.ini simulation undo_redo.txt *.vqm *.qdf \
	*.csf *.psf *.quartus

########   Synplify targets   ########################

synplify synplify-map: $(TOP)_synplify.prj synplify/$(TOP).edf

synplify-launch: $(TOP)_synplify.prj
	$(SYNPLIFY) $(TOP)_synplify.prj&
	-@mkdir synplify>& tmp.err; touch synplify/dummy.mif
	-@mv synplify/*.mif .

synplify/$(TOP).edf: $(VHDLSYNFILES) $(VERILOGSYNFILES)
	$(SYNPLIFY) -batch $(TOP)_synplify.prj
	@touch synplify/dummy.mif
	-@mv synplify/*.mif .

$(TOP)_synplify.prj: compile.synp
	@echo source compile.synp > $(TOP)_synplify.prj
	@for i in $(VHDLSYNFILES); do echo add_file "-vhdl -lib work" $$i >> $(TOP)_synplify.prj; done
#	@for i in $(VERILOGSYNFILES); do echo add_file "-verilog -lib work" $ii >> $(TOP)_synplify.prj; done
	@for i in $(SDCFILE); do echo add_file "-constraint " $$i >> $(TOP)_synplify.prj; done
	@cat $(GRLIB)/bin/synplify.prj | sed -e s/TOP/$(TOP)/ \
	-e s/TECHNOLOGY/$(TECHNOLOGY)/ -e s/PACKAGE/$(PACKAGE)/ \
	-e s/PART/$(PART)/ -e s/SPEED/$(SPEED)/ -e s/SYNFREQ/$(SYNFREQ)/ >> $(TOP)_synplify.prj
	@echo  $(TOP)_synplify.prj

synplify-clean:
	-rm -rf synplify *.prd stdout.log *.mif syntmp.*

########   Precision targets   ########################

precision precision-map: $(TOP)_precision.tcl $(TOP).psp precision/$(TOP).edf

$(TOP).psp: $(TOP)_precision.tcl
	$(PRECISION) -shell -file $(TOP)_precision.tcl

precision-launch: $(TOP).psp
	$(PRECISION)  -project $(TOP).psp

precision/$(TOP).edf: $(TOP)_precision.tcl  $(VHDLSYNFILES) $(VERILOGSYNFILES)
	$(PRECISION) -shell -file $(TOP)_precrun.tcl

precision-clean:
	-rm -rf *.psp stdout.log  $(TOP)_prec* precision* $(TOP)_temp*

# This was my Precision support (KMK)
#$(TOP).edf: $(VHDLSYNFILES) $(VERILOGSYNFILES) $(TOP)_precision.tcl
#	$(PRECISION) -shell -file $(TOP)_precision.tcl
#	#@touch synplify/dummy.mif
#	#-@mv synplify/*.mif .

#########   Actel Designer targets    ############################

actel: $(TOP)_synplify.prj synplify/$(TOP).edf $(TOP)_designer.tcl
	-mkdir ./actel
	$(DESIGNER) script:$(TOP)_designer.tcl

actel-launch-synp: $(TOP)_synplify.prj synplify/$(TOP).edf $(TOP)_designer.tcl $(TOP).adb
	$(DESIGNER) $(TOP).adb &

$(TOP).adb:
	$(DESIGNER) script:$(TOP)_designer_act.tcl

$(TOP)_designer.tcl:
	@echo "new_design -name \""$(TOP)\"" -family \""$(TECHNOLOGY)\"" " > $(TOP)_designer.tcl
	@echo "set_device -die \""$(PART)\"" -package \""$(DESIGNER_PINS) $(DESIGNER_PACKAGE)\"" -speed \""$(SPEED)\"" -voltage \"1.5\" -iostd \"LVTTL\" -jtag \"yes\" -probe \"yes\" -trst \"yes\" -temprange \""$(DESIGNER_VOLTAGE)\"" -voltrange \""$(DESIGNER_VOLTAGE)\""" >> $(TOP)_designer.tcl
	@echo "import_source -format \"edif\" -edif_flavor \"GENERIC\" " -merge_physical \"no\" -merge_timing \"no\"" {"synplify/$(TOP)".edf"} >> $(TOP)_designer.tcl
	@cp $(TOP)_designer.tcl $(TOP)_designer_act.tcl
	@echo "save_design {"$(TOP).adb"}" >> $(TOP)_designer_act.tcl
	@echo "compile -combine_register 1" >> $(TOP)_designer.tcl
	@echo "if {[file exist "$(PDC)"]} {" >> $(TOP)_designer.tcl
	@echo "   import_aux -format \"pdc\" -abort_on_error \"no\" {"$(PDC)"}" >> $(TOP)_designer.tcl
	@echo "   pin_commit" >> $(TOP)_designer.tcl
	@echo "} else {" >> $(TOP)_designer.tcl
	@echo "   puts \"WARNING: No PDC file imported.\"" >> $(TOP)_designer.tcl
	@echo "}" >> $(TOP)_designer.tcl
	@echo "if {[file exist "$(SDC)"]} {" >> $(TOP)_designer.tcl
	@echo "   import_aux -format \"sdc\" -merge_timing \"no\" {"$(SDC)"}" >> $(TOP)_designer.tcl
	@echo "} else {" >> $(TOP)_designer.tcl
	@echo "   puts \"WARNING: No SDC file imported.\"" >> $(TOP)_designer.tcl
	@echo "}" >> $(TOP)_designer.tcl
	@echo "report -type status {./actel/report_status_pre.log}" >> $(TOP)_designer.tcl
	@echo "layout -effort_level 5 -timing_driven -incremental \"OFF\"" >> $(TOP)_designer.tcl
	@echo "backannotate -dir {./actel} -name \"$(TOP)\" -format \"SDF\" -language \"VHDL93\" -netlist"  >> $(TOP)_designer.tcl
	@echo "report -type \"timer\" -sortby \"actual\" -maxpaths \"100\" -case \"worst\" -path_selection \"critical\" -setup_hold \"on\" -expand_failed \"off\" -clkpinbreak \"off\" -clrpinbreak \"on\" -latchdatapinbreak \"off\" -slack  {./actel/report_timer_worst.txt}" >> $(TOP)_designer.tcl
	@echo "report -type \"timer\" -sortby \"actual\" -maxpaths \"100\" -case \"best\"  -path_selection \"critical\" -setup_hold \"on\" -expand_failed \"off\" -clkpinbreak \"off\" -clrpinbreak \"on\" -latchdatapinbreak \"off\" -slack  {./actel/report_timer_best.txt}" >> $(TOP)_designer.tcl
	@echo "report -type \"pin\" -listby \"name\" {./actel/report_pin_name.log}" >> $(TOP)_designer.tcl
	@echo "report -type \"pin\" -listby \"number\" {./actel/report_pin_number.log}" >> $(TOP)_designer.tcl
	@echo "export -format \"AFM-APS2\" {./actel/$(TOP).afm}"  >> $(TOP)_designer.tcl
	@echo "export -format log -diagnostic {./actel/report_log.log}" >> $(TOP)_designer.tcl
	@echo "report -type status {./actel/report_status_post.log}" >> $(TOP)_designer.tcl
	@echo "save_design {"$(TOP).adb"}" >> $(TOP)_designer.tcl

actel-clean:
	-rm -rf *.adb report*.log ./actel

############  Synopsys DC targets   ########################

dc-launch: $(TOP).dc
	-mkdir synopsys
	design_compiler&

dc: $(TOP).dc
	-mkdir synopsys
	dc_shell -f $(DCSCRIPT)

$(TOP).dc: compile.dc
	@cp $(GRLIB)/bin/top.dc $(TOP).dc
	@for i in $(VHDLSYNFILES); do echo "analyze -f VHDL -library work" $$i >> $(TOP).dc; done
#	for i in $(VERILOGSYNFILES); do echo "analyze -f verilog -library work" $$i >> $(TOP).dc; done
	@echo elaborate $(TOP) >> $(TOP).dc
	@echo uniquify >> $(TOP).dc
	@echo "write -f db -hier " $(TOP) "-output" $(TOP).db >> $(TOP).dc
	@echo  $(TOP).dc

dc-clean:
	-rm -rf synopsys view_command.log command.log dumpdata.txt filenames.log \
	*.db dc.log

############  Cadence RTL Compiler   ########################

rc: $(TOP).rc
	-mkdir rtlc
	rc -files $(RCSCRIPT)

$(TOP).rc: compile.rc
	@cp $(GRLIB)/bin/top.rc $(TOP).rc
	@for i in $(VHDLSYNFILES); do echo "read_hdl -vhdl -lib work" $$i >> $(TOP).rc; done
#	@for i in $(VERILOGSYNFILES); do echo "read_hdl " $$i >> $(TOP).rc; done
	@echo elaborate $(TOP) >> $(TOP).rc
	@echo  $(TOP).rc

rc-clean:
	-rm -rf rtlc rc.log rc.cmd

########## Generation of compile scripts ###############

#$(TOP)_synplify.prj $(TOP).dc $(TOP).rc $(TOP).xst $(TOP).npl $(TOP).qsf : \
#	compile.dc compile.synp compile.vsim compile.xst compile.ncsim compile.rc

scripts: compile.dc compile.synp compile.vsim compile.xst compile.ncsim compile.rc \
	$(TOP)_synplify.prj $(TOP).dc $(TOP).rc $(TOP).xst $(TOP).npl $(TOP).qsf \
	$(TOP)_designer.tcl $(TOP).ise $(TOP)_precision.tcl

verilog.txt compile.dc compile.synp compile.vsim compile.xst compile.ncsim compile.rc compile.ghdl $(TOP).npl $(TOP).qsf $(TOP).ise $(TOP)_precision.tcl: $(UCF)
	@touch libs.txt; rm libs.txt;
	@cp $(GRLIB)/bin/cds.lib cds.lib; touch hdl.var;
	@for i in vsim synp xst dc ncsim ghdl rc; do \
	    touch compile.$$i; rm compile.$$i ; \
	done;
	@echo -e \\tmkdir xncsim > compile.ncsim
	@echo sh mkdir synopsys > compile.dc
	@echo vlib modelsim > libs.do
	@echo -e \\tmkdir gnu > compile.ghdl
	@echo set_attribute input_pragma_keyword \"cadence synopsys get2chip g2c fast ambit pragma\" > compile.rc
	@echo "[Library]" > modelsim.ini;
	@echo "" > tmp.mpf;
	@echo "" > verilog.txt;
	@echo JDF G > $(TOP).npl
	@echo PROJECT $(TOP) >> $(TOP).npl
	@echo DESIGN $(TOP) >> $(TOP).npl
	@echo DEVFAM $(TECHNOLOGY) >> $(TOP).npl
	@echo DEVICE $(PART) >> $(TOP).npl
	@echo DEVSPEED $(SPEED) >> $(TOP).npl
	@echo DEVPKG $(PACKAGE) >> $(TOP).npl
	@cp $(TOP).npl $(TOP)_synplify.npl
	@echo DEVTOPLEVELMODULETYPE HDL >> $(TOP).npl
	@echo DEVTOPLEVELMODULETYPE EDIF >> $(TOP)_synplify.npl
	@cat  $(GRLIB)/bin/def.npl >> $(TOP).npl
	@cat  $(GRLIB)/bin/def.npl >> $(TOP)_synplify.npl
	@touch tmp.npl; rm tmp.npl
	@#This was my Precision support (KMK)
	@#cat $(GRLIB)/bin/precision.tcl > $(TOP)_precision.tcl
	@#echo -e "\n\n## Input File Settings" >> $(TOP)_precision.tcl
	@for i in $(VHDLSYNFILES); do \
	   echo SOURCE $$i >> tmp.npl; \
	done; \
	echo BEGIN_ISE_DATA > ise.txt; \
	echo "	version: 1.0" >> ise.txt; \
	echo "	ise_file: $(TOP).ise" >> ise.txt; \
	echo "	BEGIN_CLIENT" >> ise.txt; \
	echo "		client_name: ProjectNavigator" >> ise.txt; \
	echo "		BEGIN_SECTION" >> ise.txt; \
	echo "			section_path: /ProjectNavigator/" >> ise.txt; \
	echo "			keyname: 0" >> ise.txt; \
	echo "			keyval:  JDF H" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 1" >> ise.txt; \
	echo "			keyval:  // Created by Project Navigator ver 1.0" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 2" >> ise.txt; \
	echo "			keyval:  DEVFAM $(TECHNOLOGY)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 3" >> ise.txt; \
	echo "			keyval:  DEVICE $(PART)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 4" >> ise.txt; \
	echo "			keyval:  DEVPKG $(PACKAGE)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 5" >> ise.txt; \
	echo "			keyval:  DEVSPEED $(SPEED)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 6" >> ise.txt; \
	echo "			keyval:  DEVTOPLEVELMODULETYPE HDL" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 7" >> ise.txt; \
	echo "			keyval:  DEVSYNTHESISTOOL XST (VHDL/Verilog)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	cp $(GRLIB)/bin/quartus.qsf_head $(TOP).qsf; \
	cp $(GRLIB)/bin/quartus.qsf_head $(TOP)_synplify.qsf; \
	echo "set_global_assignment -name VQM_FILE" "synplify/$(TOP).edf" >> $(TOP)_synplify.qsf; \
	if test -r "$(QSF)"; then cat $(QSF) >> $(TOP)_synplify.qsf; fi; \
	cp $(GRLIB)/bin/quartus.qpf $(TOP).qpf; \
	cp $(TOP).qpf $(TOP)_synplify.qpf; \
	echo PROJECT_REVISION = $(TOP) >> $(TOP).qpf; \
	echo PROJECT_REVISION = $(TOP)_synplify >> $(TOP)_synplify.qpf; \
	ppath="gnu"; nfiles=0; xfiles=8;\
	echo "" > ise2.txt; \
	for i in $(VHDLSYNFILES); do \
	   echo "			keyname: $$xfiles" >> ise2.txt; \
	   echo "			keyval:  SOURCE $$i" >> ise2.txt; \
	   echo "			keytype: STRING" >> ise2.txt; \
	   xfiles=`expr $$xfiles + 1`; \
	done; \
	echo "open_project ./$(TOP).psp"> $(TOP)_precrun.tcl; \
	echo compile >> $(TOP)_precrun.tcl; echo synthesize >> $(TOP)_precrun.tcl; \
	echo save_impl >> $(TOP)_precrun.tcl; \
	echo "new_project -name $(TOP) -folder . -createimpl_name precision"> $(TOP)_precision.tcl; \
	echo "setup_design -manufacturer $(MANUFACTURER) -family $(MGCTECHNOLOGY) -part $(MGCPART) -package $(MGCPACKAGE) -speed $(SPEED)" >> $(TOP)_precision.tcl; \
	echo "set_input_dir ." >> $(TOP)_precision.tcl ;\
	if ((test -r $(GRLIB)/lib/fpu/ncsim/grfpu.pk) && (!(test -r $(GRLIB)/lib/fpu/grfpu))) ; then \
	   ncsimfpu=1; else ncsimfpu=0; \
	fi;   \
	if ((test -r $(GRLIB)/lib/fpu/modelsim) && (!(test -r $(GRLIB)/lib/fpu/grfpu))); then \
	   vsimfpu=1; else vsimfpu=0; \
    fi; \
	echo "Scanning libraries" ; \
	if test $$ncsimfpu = 1; then \
	   echo -e \\tncrelocate -install -overwrite -directory xncsim $(GRLIB)/lib/fpu/ncsim/grfpu.pk >> compile.ncsim; \
	  echo "DEFINE grlibx xncsim/grlibx" >> cds.lib ; \
	fi; \
	for j in `cat $(GRLIB)/lib/libs.txt  $(GRLIB)/lib/*/libs.txt` work ; do \
	  bn=`basename $$j` ; \
          ppath="$$ppath -Pgnu/$$bn"; \
	  k=$(GRLIB)/lib/$$j; \
	  if test -r $$k; then \
	    echo -e "  "$$bn ""; \
	    if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
                echo -n $$bn "" >> libs.txt; echo $$bn = modelsim/$$bn >> modelsim.ini; \
	    fi; \
	    echo SUBLIB $$bn VhdlLibrary vhdl >> tmp.npl; \
	    echo DEFINE $$bn xncsim/$$bn "" >> cds.lib; \
	    if ((test $$ncsimfpu = 0) || (test $$j != "fpu")); then \
	      	echo -e \\tmkdir xncsim/$$bn "" >> compile.ncsim; \
	    fi; \
	    echo -e \\tmkdir gnu/$$bn "" >> compile.ghdl; \
	    if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
	       echo vlib modelsim/$$bn "" >> libs.do; \
            else \
               echo vmap grlibx $(GRLIB)/lib/fpu/modelsim/grlibx >> libs.do; \
               echo vmap fpu $(GRLIB)/lib/fpu/modelsim/fpu >> libs.do; \
	    fi; \
	    echo sh mkdir synopsys/$$bn "" >> compile.dc; \
	    echo define_design_lib $$bn -path synopsys/$$bn "" >> compile.dc; \
	    case $$bn in $(XSTLIBSKIP) )\
		;; *) \
	      echo "			keyname: $$xfiles" >> ise2.txt; \
	      echo "			keyval:  SUBLIB $$bn VhdlLibrary vhdl" >> ise2.txt; \
	      echo "			keytype: STRING" >> ise2.txt; \
	      xfiles=`expr $$xfiles + 1`;; \
	    esac; \
	      if test -r $$k/dirs.txt; then \
	        for l in `cat $$k/dirs.txt` ; do \
	    	  for i in vlogsyn vhdlsyn vlogsim vhdlsim ; do \
		    m=$$k/$$l/$$i; \
		    if test -r $$m.txt; then \
		      for q in `cat $$m.txt`; do \
		        if test -r $$k/$$l/$$q; then \
			case $$i in \
			vhdlsyn) \
                   echo "    "$$k"/"$$l"/"$$q""; \
		           echo Project_File_$$nfiles = $$k/$$l/$$q >> tmp.mpf; \
		           echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VHDL group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
			   nfiles=`expr $$nfiles + 1`; \
        		   if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
		               echo -e \\t$(VCOM) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.vsim; \
                           fi; \
			   if ((test $$ncsimfpu = 0) || (test $$j != "fpu")); then \
			        echo -e \\t$(NCVHDL) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.ncsim; \
		           fi; \
		           echo -e \\t$(GHDL) $(VHDLOPT) --workdir=gnu/$$bn --work=$$bn -P$$ppath $$k/$$l/$$q >> compile.ghdl; \
		           echo $(DCVHDL) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.dc; \
		           echo $(RTLCVHDL) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.rc; \
		           echo $(SYNPVHDL) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.synp; \
			   case $$bn in $(XSTLIBSKIP) )\
				uu=0;; \
			   *) \
			    case $$q in $(XSTSKIP) ) ;; *) \
		             echo LIBFILE $$k/$$l/$$q $$bn vhdl >> tmp.npl; \
			     echo "			keyname: $$xfiles" >> ise2.txt; \
			     echo "			keyval:  LIBFILE $$k/$$l/$$q $$bn vhdl" >> ise2.txt; \
			     echo "			keytype: STRING" >> ise2.txt; \
		             echo $(XSTVHDL) $(VHDLOPT) $$bn -ifn $$k/$$l/$$q >> compile.xst; \
			     xfiles=`expr $$xfiles + 1`;; \
			     esac; \
			   esac; \
			   case $$bn in $(PRECLIBSKIP) )\
				uu=0;; \
			   *) \
			    case $$q in $(PRECSKIP) ) ;; *) \
		               echo add_input_file -format VHDL -work $$bn  $$k/$$l/$$q >> $(TOP)_precision.tcl; \
		               echo set_global_assignment -name VHDL_FILE $$k/$$l/$$q >> $(TOP).qsf;; \
			    esac; \
			   esac;; \
			vlogsyn) \
		           echo Project_File_$$nfiles = $$k/$$l/$$q >> tmp.mpf; \
		           echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VERILOG group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
			   nfiles=`expr $$nfiles + 1`; \
		           echo -e \\t$(VLOG) $$bn $$k/$$l/$$q >> compile.vsim; \
		           echo -e \\t$(NCVLOG) $$bn $$k/$$l/$$q >> compile.ncsim; \
		           echo $(XSTVLOG) $$bn -ifn $$k/$$l/$$q >> compile.xst; \
		           echo $$k/$$l/$$q >> verilog.txt; \
		           echo $(DCVLOG) $$bn $$k/$$l/$$q >> compile.dc; \
		           echo $(RTLCVLOG) $$k/$$l/$$q >> compile.rc; \
		           echo LIBFILE $$k/$$l/$$q $$bn verilog >> tmp.npl; \
		           echo $(SYNPVLOG) $$k/$$l/$$q >> compile.synp; \
		           echo add_input_file -format VERILOG -work $$bn  $$k/$$l/$$q >> $(TOP)_precision.tcl; \
		           echo set_global_assignment -name VERILOG_FILE $$k/$$l/$$q >> $(TOP).qsf;; \
			vhdlsim) \
                   echo "    "$$k$$l$$q""; \
		           echo Project_File_$$nfiles = $$k/$$l/$$q >> tmp.mpf; \
		           echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VHDL group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
			   nfiles=`expr $$nfiles + 1`; \
		           echo -e \\t$(VCOM) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.vsim; \
		           echo -e \\t$(NCVHDL) $(VHDLOPT) $$bn $$k/$$l/$$q >> compile.ncsim; \
		           echo -e \\t$(GHDL) $(VHDLOPT) --workdir=gnu/$$bn --work=$$bn -P$$ppath $$k/$$l/$$q >> compile.ghdl;; \
			vlogsim) \
		           echo Project_File_$$nfiles = $$k/$$l/$$q >> tmp.mpf; \
		           echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VERILOG group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
			   nfiles=`expr $$nfiles + 1`; \
		           echo -e \\t$(VLOG) $$bn $$k/$$l/$$q >> compile.vsim; \
		           echo -e \\t$(NCVLOG) $$bn $$k/$$l/$$q >> compile.ncsim;; \
			esac ; \
			fi ; \
	    	      done ; \
		    fi ; \
	    	  done ; \
	        done ; \
	      fi ; \
	  else \
	    if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
               echo -n $$bn "" >> libs.txt; echo $$bn = modelsim/$$bn >> modelsim.ini; \
	       echo vlib modelsim/$$bn "" >> libs.do; \
            fi ; \
	  fi ; \
	done ; \
	for i in $(VHDLSYNFILES); do \
	  echo add_input_file -format VHDL -work work  $$i >> $(TOP)_precision.tcl; \
	  echo set_global_assignment -name VHDL_FILE $$i >> $(TOP).qsf; \
	done; \
	echo setup_design -design $(TOP) >> $(TOP)_precision.tcl; \
	echo setup_design -retiming >> $(TOP)_precision.tcl; \
	echo setup_design -vhdl >> $(TOP)_precision.tcl; \
	echo setup_design -transformations=false >> $(TOP)_precision.tcl; \
	echo setup_design -frequency=\"$(SYNFREQ)\" >> $(TOP)_precision.tcl; \
	echo save_impl >> $(TOP)_precision.tcl; \
	echo "set_global_assignment -name TOP_LEVEL_ENTITY" \"$(TOP)\" >> $(TOP).qsf; \
	echo "set_global_assignment -name TOP_LEVEL_ENTITY" \"$(TOP)\" >> $(TOP)_synplify.qsf; \
	cat tmp.npl >> $(TOP).npl; \
	echo DEPASSOC $(TOP) $(UCF) >> $(TOP).npl; \
	echo "			keyname: $$xfiles" >> ise2.txt; \
	echo "			keyval:  DEPASSOC $(TOP) $(UCF)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	xfiles=`expr $$xfiles + 1`; \
	echo "			keyname: NumEntries" >> ise2.txt; \
	echo "			keyval:  $$xfiles" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo SOURCE synplify/$(TOP).edf >> $(TOP)_synplify.npl; \
	echo DEPASSOC $(TOP) $(UCF) >> $(TOP)_synplify.npl; \
	echo "[Normal]" >> $(TOP).npl; echo "[Normal]" >> $(TOP)_synplify.npl; \
	echo "_SynthFsmEncode=xstvhd, " $(TECHNOLOGY)", VHDL.t_synthesize, 1102507235, None" >> $(TOP).npl; \
	echo "p_xstBusDelimiter=xstvhd, " $(TECHNOLOGY)", VHDL.t_synthesize, 1102507235, ()" >> $(TOP).npl; \
	echo "xilxMapAllowLogicOpt=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxMapCoverMode=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, Speed" >>  $(TOP).npl; \
	echo "xilxMapTimingDrivenPacking=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxNgdbld_AUL=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxNgdbldMacro=xstvhd, " $(TECHNOLOGY)", VHDL.t_ngdbuild, 1105377047, "$(GRLIB)/netlists/xilinx/$(TECHNOLOGY) >>  $(TOP).npl; \
	echo "xilxPAReffortLevel=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, Medium" >>  $(TOP).npl; \
	echo "xilxMapAllowLogicOpt=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, True" >>  $(TOP)_synplify.npl; \
	echo "xilxMapCoverMode=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, Speed" >>  $(TOP)_synplify.npl; \
	echo "xilxNgdbld_AUL=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, True" >>  $(TOP)_synplify.npl; \
	echo "xilxPAReffortLevel=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, Medium" >>  $(TOP)_synplify.npl; \
	echo "xilxNgdbldMacro=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1105378344, "$(GRLIB)/netlists/xilinx/$(TECHNOLOGY) >>  $(TOP)_synplify.npl; \
	cat $(TOP).npl | sed -e s/'\/'/'\\'/g > tmp.npl; \
	cp tmp.npl $(TOP)_win32.npl; \
	cat ise2.txt | sed -e s/'\/'/'\\'/g > tmp.txt; \
	cat ise.txt tmp.txt > ise_win32.txt; \
	cat ise2.txt >> ise.txt; \
	cat $(TOP)_synplify.npl | sed -e s/'\/'/'\\'/g > tmp.npl; \
	cp tmp.npl $(TOP)_synplify_win32.npl; \
	if test $(OS) != "Linux"; then \
	  if test $(OS) != "SunOs"; then \
	    cp $(TOP)_win32.npl $(TOP).npl; \
	    cp $(TOP)_synplify_win32.npl $(TOP)_synplify.npl; \
	    cp ise_win32.txt ise.txt; \
	  fi; \
	fi; \
	echo "		BEGIN_SECTION" > ise2.txt; \
	echo "			section_path: /ProjectNavigator/NORMAL/" >> ise2.txt; \
	echo "			keyname: _SynthFsmEncode" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1102507235, None" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: _SynthPipelining" >> ise2.txt; \
	echo "			keyval:  synprovhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119214358, TRUE" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: _SynthUseFsmExplorerData" >> ise2.txt; \
	echo "			keyval:  synprovhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119214358, TRUE" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstBusDelimiter" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1102507235, ()" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstCoresSearchDir" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119216548, $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstOptimizeInsPrimtives" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119216548, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapAllowLogicOpt" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapCoverMode" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, Speed" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapTimingDrivenPacking" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxNgdbldMacro" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1105377047, $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxNgdbld_AUL" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxPAReffortLevel" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, Medium" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo "		BEGIN_SECTION" >> ise2.txt; \
	echo "			section_path: /ProjectNavigator/STATUS-ALL/" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo "	END_CLIENT" >> ise2.txt; \
	echo "END_ISE_DATA" >> ise2.txt; \
	cat ise2.txt >> ise.txt; cat ise2.txt >> ise_win32.txt; rm ise2.txt; \
	echo "[STRATEGY-LIST]" >> $(TOP).npl; echo "[STRATEGY-LIST]" >> $(TOP)_synplify.npl; \
	echo "Normal=True" >> $(TOP).npl; echo "Normal=True" >> $(TOP)_synplify.npl; \
	echo "DEVSYNTHESISTOOL XST (VHDL/Verilog)" >>  $(TOP).npl; \
	for i in $(VHDLSYNFILES) $(VHDLSIMFILES); do \
	   echo Project_File_$$nfiles = $$i >> tmp.mpf; \
	   echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VHDL group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
	   nfiles=`expr $$nfiles + 1`; \
	done; rm tmp.npl; \
        echo "Project_Sim_Count = 1" >> tmp.mpf; \
	echo "Project_Sim_0 = Simulation 1" >> tmp.mpf; \
	echo "Project_Sim_P_0 = Generics {} timing default -std_output {} +notimingchecks 0 -L {} selected_du {} -hazards 0 -sdf {} +acc {} ok 1 folder {Top Level} -absentisempty 0 +pulse_r {} OtherArgs {} -multisource_delay {} +pulse_e {} -coverage 0 -sdfnoerror 0 +plusarg {} -vital2.2b 0 -t ps additional_dus" work.$(SIMTOP) "-nofileshare 0 -noglitch 0 -wlf {} +no_pulse_msg 0 -assertfile {} -sdfnowarn 0 -Lf {} -std_input {}" >> tmp.mpf; \
	grep -v unisim compile.synp > xx; mv xx compile.synp; \
	for i in $(QUARTUS_SKIP); do grep -v $$i $(TOP).qsf > xx; mv xx $(TOP).qsf; done;\
	for i in $(DCSKIP); do \
		grep -v $$i compile.dc > xx; mv xx compile.dc; \
		grep -v $$i compile.rc > xx; mv xx compile.rc; \
	done;\
	for i in $(GHDLSKIP); do grep -v $$i compile.ghdl > xx; mv xx compile.ghdl; done;\
	cat $(GRLIB)/bin/modelsim.ini >> modelsim.ini; \
	cp modelsim.ini $(SIMTOP).mpf; \
	echo "[Project]" >> $(SIMTOP).mpf; \
	echo Project_Version = 5 >> $(SIMTOP).mpf; \
	echo Project_DefaultLib = work >> $(SIMTOP).mpf; \
	echo Project_SortMethod = unused >> $(SIMTOP).mpf; \
        echo Project_Files_Count = $$nfiles >> $(SIMTOP).mpf; \
        cat tmp.mpf >> $(SIMTOP).mpf; rm tmp.mpf; \
	cat $(GRLIB)/bin/mt1.mpf >> $(SIMTOP).mpf; \
	echo -P$$ppath > ghdl.path; \
	if test -r "$(QSF)"; then cat $(QSF) >> $(TOP).qsf; fi; \
	echo ghdl: > tmpmake.ghdl;  cat compile.ghdl >> tmpmake.ghdl; \
	echo ncsim: > make.ncsim; cat compile.ncsim >> make.ncsim; \
	echo vsim: > make.vsim; cat compile.vsim >> make.vsim; \
	for i in $(VHDLSYNFILES) $(VHDLSIMFILES); do \
	  echo -e \\t$(VCOM) $(VHDLOPT) $$bn $$i >> make.vsim; \
	  echo -e \\t$(NCVHDL) $(VHDLOPT) $$bn $$i >> make.ncsim; \
	  echo -e \\t$(GHDL) $(VHDLOPT) --workdir=gnu/$$bn --work=$$bn -P$$ppath $$i >> tmpmake.ghdl; \
	done; \
	if test -r "$(SIMTOP)".vhd; then \
	  arch=`grep -i architecture $(SIMTOP).vhd | grep -i $(SIMTOP) | awk '{ print $$2}'`; \
	  echo -e \\tncelab $(SIMTOP):$$arch >> make.ncsim ; \
	fi; \
	echo $(SIMTOP).mpf; \
	if test -r "$(XILINX)/data/projnav/iseEdit.tcl"; then \
	  touch $(TOP).ise; rm $(TOP).ise; touch $(TOP).ise; \
	  xtclsh "$(XILINX)"/data/projnav/iseEdit.tcl import datafile=ise.txt >& ise.err; rm ise.err; \
	  echo "$(TOP).ise"; \
	fi; \

libs:
	@touch libs.txt; rm libs.txt;
	@cp $(GRLIB)/bin/cds.lib cds.lib; touch hdl.var;
	@echo -e \\tmkdir xncsim > compile.ncsim
	@echo sh mkdir synopsys > compile.dc
	@echo vlib modelsim > libs.do
	@echo -e \\tmkdir gnu > compile.ghdl
	@echo "" > compile.xst
	@echo "" > compile.synp
	@echo set_attribute input_pragma_keyword \"cadence synopsys get2chip g2c fast ambit pragma\" > compile.rc
	@echo "[Library]" > modelsim.ini;
	@echo "" > tmp.mpf;
	@echo "" > verilog.txt;
	@echo "" > work.v;
	@echo JDF G > $(TOP).npl
	@echo PROJECT $(TOP) >> $(TOP).npl
	@echo DESIGN $(TOP) >> $(TOP).npl
	@echo DEVFAM $(TECHNOLOGY) >> $(TOP).npl
	@echo DEVICE $(PART) >> $(TOP).npl
	@echo DEVSPEED $(SPEED) >> $(TOP).npl
	@echo DEVPKG $(PACKAGE) >> $(TOP).npl
	@cp $(TOP).npl $(TOP)_synplify.npl
	@echo DEVTOPLEVELMODULETYPE HDL >> $(TOP).npl
	@echo DEVTOPLEVELMODULETYPE EDIF >> $(TOP)_synplify.npl
	@cat  $(GRLIB)/bin/def.npl >> $(TOP).npl
	@cat  $(GRLIB)/bin/def.npl >> $(TOP)_synplify.npl
	@touch tmp.npl; rm tmp.npl
	@for i in $(VHDLSYNFILES); do \
	   echo SOURCE $$i >> tmp.npl; \
	done; \
	echo BEGIN_ISE_DATA > ise.txt; \
	echo "	version: 1.0" >> ise.txt; \
	echo "	ise_file: $(TOP).ise" >> ise.txt; \
	echo "	BEGIN_CLIENT" >> ise.txt; \
	echo "		client_name: ProjectNavigator" >> ise.txt; \
	echo "		BEGIN_SECTION" >> ise.txt; \
	echo "			section_path: /ProjectNavigator/" >> ise.txt; \
	echo "			keyname: 0" >> ise.txt; \
	echo "			keyval:  JDF H" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 1" >> ise.txt; \
	echo "			keyval:  // Created by Project Navigator ver 1.0" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 2" >> ise.txt; \
	echo "			keyval:  DEVFAM $(TECHNOLOGY)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 3" >> ise.txt; \
	echo "			keyval:  DEVICE $(PART)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 4" >> ise.txt; \
	echo "			keyval:  DEVPKG $(PACKAGE)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 5" >> ise.txt; \
	echo "			keyval:  DEVSPEED $(SPEED)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 6" >> ise.txt; \
	echo "			keyval:  DEVTOPLEVELMODULETYPE HDL" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	echo "			keyname: 7" >> ise.txt; \
	echo "			keyval:  DEVSYNTHESISTOOL XST (VHDL/Verilog)" >> ise.txt; \
	echo "			keytype: STRING" >> ise.txt; \
	cp $(GRLIB)/bin/quartus.qsf_head $(TOP).qsf; \
	cp $(GRLIB)/bin/quartus.qsf_head $(TOP)_synplify.qsf; \
	echo "set_global_assignment -name VQM_FILE" "synplify/$(TOP).edf" >> $(TOP)_synplify.qsf; \
	if test -r "$(QSF)"; then cat $(QSF) >> $(TOP)_synplify.qsf; fi; \
	cp $(GRLIB)/bin/quartus.qpf $(TOP).qpf; \
	cp $(TOP).qpf $(TOP)_synplify.qpf; \
	echo PROJECT_REVISION = $(TOP) >> $(TOP).qpf; \
	echo PROJECT_REVISION = $(TOP)_synplify >> $(TOP)_synplify.qpf; \
	ppath="gnu"; nfiles=0; xfiles=8; \
	echo "" > ise2.txt; \
	for i in $(VHDLSYNFILES); do \
	   echo "			keyname: $$xfiles" >> ise2.txt; \
	   echo "			keyval:  SOURCE $$i" >> ise2.txt; \
	   echo "			keytype: STRING" >> ise2.txt; \
	   xfiles=`expr $$xfiles + 1`; \
	done; \
	if ((test -r $(GRLIB)/lib/fpu/ncsim/grfpu.pk) && (!(test -r $(GRLIB)/lib/fpu/grfpu))) ; then \
	   ncsimfpu=1; else ncsimfpu=0; \
	fi;   \
	if ((test -r $(GRLIB)/lib/fpu/modelsim) && (!(test -r $(GRLIB)/lib/fpu/grfpu))); then \
	   vsimfpu=1; else vsimfpu=0; \
        fi; \
	echo "Generating libraries" ; \
	if test $$ncsimfpu = 1; then \
	   echo -e \\tncrelocate -install -overwrite -directory xncsim $(GRLIB)/lib/fpu/ncsim/grfpu.pk >> compile.ncsim; \
	  echo "DEFINE grlibx xncsim/grlibx" >> cds.lib ; \
	fi; \
	for j in `cat $(GRLIB)/lib/libs.txt  $(GRLIB)/lib/*/libs.txt` work ; do \
	  bn=`basename $$j` ; \
          ppath="$$ppath -Pgnu/$$bn"; \
	  k=$(GRLIB)/lib/$$j; \
	  if test -r $$k; then \
	    echo "  "$$bn ""; \
	    if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
                echo -n $$bn "" >> libs.txt; echo $$bn = modelsim/$$bn >> modelsim.ini; \
	    fi; \
	    echo SUBLIB $$bn VhdlLibrary vhdl >> tmp.npl; \
	    echo DEFINE $$bn xncsim/$$bn "" >> cds.lib; \
	    if ((test $$ncsimfpu = 0) || (test $$j != "fpu")); then \
	      	echo -e \\tmkdir xncsim/$$bn "" >> compile.ncsim; \
	    fi; \
	    echo -e \\tmkdir gnu/$$bn "" >> compile.ghdl; \
	    if ((test $$vsimfpu = 0) || (test $$j != "fpu")); then \
	       echo vlib modelsim/$$bn "" >> libs.do; \
            else \
               echo vmap grlibx $(GRLIB)/lib/fpu/modelsim/grlibx >> libs.do; \
               echo vmap fpu $(GRLIB)/lib/fpu/modelsim/fpu >> libs.do; \
	    fi; \
	    echo sh mkdir synopsys/$$bn "" >> compile.dc; \
	    echo define_design_lib $$bn -path synopsys/$$bn "" >> compile.dc; \
	    echo "			keyname: $$xfiles" >> ise2.txt; \
	    echo "			keyval:  SUBLIB $$bn VhdlLibrary vhdl" >> ise2.txt; \
	    echo "			keytype: STRING" >> ise2.txt; \
	    xfiles=`expr $$xfiles + 1`; \
	      if test -r $$k/dirs.txt; then \
		touch $$bn.vhd; rm $$bn.vhd; \
	        if test $$bn = "work"; then \
		   echo Project_File_$$nfiles = $$bn.v >> tmp.mpf; \
		   echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VERILOG group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
		   nfiles=`expr $$nfiles + 1`; \
		   echo -e \\t$(VLOG) $$bn $$bn.v >> compile.vsim; \
		   echo -e \\t$(NCVLOG) $$bn $$bn.v >> compile.ncsim; \
		   echo $(XSTVLOG) $$bn -ifn $$bn.v >> compile.xst; \
		   echo $(DCVLOG) $$bn $$bn.v >> compile.dc; \
		   echo $(RTLCVLOG) $$bn.v >> compile.rc; \
		   echo LIBFILE $$bn.v $$bn verilog >> tmp.npl; \
		   echo $(SYNPVLOG) $$bn.v >> compile.synp; \
		   echo set_global_assignment -name VERILOG_FILE $$bn.v >> $(TOP).qsf; \
                fi; \
		echo -e \\t$(GHDL) $(VHDLOPT) --workdir=gnu/$$bn --work=$$bn -P$$ppath $$bn.vhd >> compile.ghdl; \
		echo $(DCVHDL) $(VHDLOPT) $$bn $$bn.vhd >> compile.dc; \
	        if test $$bn != "altera"; then \
	          if test $$bn != "synplify"; then \
	            if test $$bn != "micron"; then \
		      echo Project_File_$$nfiles = $$bn.vhd >> tmp.mpf; \
		      echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VHDL group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
		      echo LIBFILE $$k.vhd $$bn vhdl >> tmp.npl; \
		      echo $(XSTVHDL) $(VHDLOPT) $$bn -ifn $$bn.vhd >> compile.xst; \
		      echo $(RTLCVHDL) $(VHDLOPT) $$bn $$bn.vhd >> compile.rc; \
		      echo $(SYNPVHDL) $(VHDLOPT) $$bn $$bn.vhd >> compile.synp; \
		      echo LIBFILE $$bn.vhd $$bn vhdl >> tmp.npl; \
		      echo "			keyname: $$xfiles" >> ise2.txt; \
		      echo "			keyval:  LIBFILE $$bn.vhd $$bn vhdl" >> ise2.txt; \
		      echo "			keytype: STRING" >> ise2.txt; \
		      xfiles=`expr $$xfiles + 1`; \
		    fi ; \
		  fi ; \
		fi ; \
		echo -e \\t$(VCOM) $(VHDLOPT) $$bn $$bn.vhd >> compile.vsim; \
		echo -e \\t$(NCVHDL) $(VHDLOPT) $$bn $$bn.vhd >> compile.ncsim; \
		echo set_global_assignment -name VHDL_FILE $$bn.vhd >> $(TOP).qsf; \
		nfiles=`expr $$nfiles + 1`; \
	        for l in `cat $$k/dirs.txt` ; do \
	    	  for i in vlogsyn vhdlsyn vlogsim vhdlsim ; do \
		    m=$$k/$$l/$$i; \
		    if test -r $$m.txt; then \
		      for q in `cat $$m.txt`; do \
		        if test -r $$k/$$l/$$q; then \
			case $$i in \
			vhdlsyn) \
		           cat $$k/$$l/$$q >> $$bn.vhd;; \
			vlogsyn) \
		           cat $$k/$$l/$$q >> $$bn.v;; \
			vhdlsim) \
		           cat $$k/$$l/$$q >> $$bn.vhd;; \
			vlogsim) \
		           cat $$k/$$l/$$q >> $$bn.v;; \
			esac ; \
			fi ; \
	    	      done ; \
		    fi ; \
	    	  done ; \
	        done ; \
	      fi ; \
	  fi ; \
	done ; \
	for i in $(VHDLSYNFILES); do \
	   echo set_global_assignment -name VHDL_FILE $$i >> $(TOP).qsf; \
	done; \
	echo "set_global_assignment -name TOP_LEVEL_ENTITY" \"$(TOP)\" >> $(TOP).qsf; \
	echo "set_global_assignment -name TOP_LEVEL_ENTITY" \"$(TOP)\" >> $(TOP)_synplify.qsf; \
	cat tmp.npl >> $(TOP).npl; \
	echo DEPASSOC $(TOP) $(UCF) >> $(TOP).npl; \
	echo "			keyname: $$xfiles" >> ise2.txt; \
	echo "			keyval:  DEPASSOC $(TOP) $(UCF)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	xfiles=`expr $$xfiles + 1`; \
	echo "			keyname: NumEntries" >> ise2.txt; \
	echo "			keyval:  $$xfiles" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo SOURCE synplify/$(TOP).edf >> $(TOP)_synplify.npl; \
	echo DEPASSOC $(TOP) $(UCF) >> $(TOP)_synplify.npl; \
	echo "[Normal]" >> $(TOP).npl; echo "[Normal]" >> $(TOP)_synplify.npl; \
	echo "_SynthFsmEncode=xstvhd, " $(TECHNOLOGY)", VHDL.t_synthesize, 1102507235, None" >> $(TOP).npl; \
	echo "p_xstBusDelimiter=xstvhd, " $(TECHNOLOGY)", VHDL.t_synthesize, 1102507235, ()" >> $(TOP).npl; \
	echo "xilxMapAllowLogicOpt=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxMapCoverMode=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, Speed" >>  $(TOP).npl; \
	echo "xilxMapTimingDrivenPacking=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxNgdbld_AUL=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, True" >>  $(TOP).npl; \
	echo "xilxNgdbldMacro=xstvhd, " $(TECHNOLOGY)", VHDL.t_ngdbuild, 1105377047, "$(GRLIB)/netlists/xilinx/$(TECHNOLOGY) >>  $(TOP).npl; \
	echo "xilxPAReffortLevel=xstvhd, " $(TECHNOLOGY)", VHDL.t_placeAndRouteDes, 1102861051, Medium" >>  $(TOP).npl; \
	echo "xilxMapAllowLogicOpt=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, True" >>  $(TOP)_synplify.npl; \
	echo "xilxMapCoverMode=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, Speed" >>  $(TOP)_synplify.npl; \
	echo "xilxNgdbld_AUL=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, True" >>  $(TOP)_synplify.npl; \
	echo "xilxPAReffortLevel=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1102861051, Medium" >>  $(TOP)_synplify.npl; \
	echo "xilxNgdbldMacro=edif, " $(TECHNOLOGY)", EDIF.t_placeAndRouteDes, 1105378344, "$(GRLIB)/netlists/xilinx/$(TECHNOLOGY) >>  $(TOP)_synplify.npl; \
	cat $(TOP).npl | sed -e s/'\/'/'\\'/g > tmp.npl; \
	cp tmp.npl $(TOP)_win32.npl; \
	cat $(TOP)_synplify.npl | sed -e s/'\/'/'\\'/g > tmp.npl; \
	cat ise2.txt | sed -e s/'\/'/'\\'/g > tmp.txt; \
	cat ise.txt tmp.txt > ise_win32.txt; \
	cat ise2.txt >> ise.txt; \
	cp tmp.npl $(TOP)_synplify_win32.npl; \
	if test $(OS) != "Linux"; then \
	  if test $(OS) != "SunOs"; then \
	    cp $(TOP)_win32.npl; cp tmp.npl $(TOP).npl; \
	    cp $(TOP)_synplify_win32.npl; cp tmp.npl $(TOP)_synplify.npl; \
	    cp ise_win32.txt ise.txt; \
	  fi; \
	fi; \
	echo "		BEGIN_SECTION" > ise2.txt; \
	echo "			section_path: /ProjectNavigator/NORMAL/" >> ise2.txt; \
	echo "			keyname: _SynthFsmEncode" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1102507235, None" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: _SynthPipelining" >> ise2.txt; \
	echo "			keyval:  synprovhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119214358, TRUE" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: _SynthUseFsmExplorerData" >> ise2.txt; \
	echo "			keyval:  synprovhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119214358, TRUE" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstBusDelimiter" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1102507235, ()" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstCoresSearchDir" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119216548, $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: p_xstOptimizeInsPrimtives" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_synthesize, 1119216548, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapAllowLogicOpt" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapCoverMode" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, Speed" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxMapTimingDrivenPacking" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxNgdbldMacro" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1105377047, $(GRLIB)/netlists/xilinx/$(TECHNOLOGY)" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxNgdbld_AUL" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, True" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "			keyname: xilxPAReffortLevel" >> ise2.txt; \
	echo "			keyval:  xstvhd, $(TECHNOLOGY), VHDL.t_placeAndRouteDes, 1102861051, Medium" >> ise2.txt; \
	echo "			keytype: STRING" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo "		BEGIN_SECTION" >> ise2.txt; \
	echo "			section_path: /ProjectNavigator/STATUS-ALL/" >> ise2.txt; \
	echo "		END_SECTION" >> ise2.txt; \
	echo "	END_CLIENT" >> ise2.txt; \
	echo "END_ISE_DATA" >> ise2.txt; \
	cat ise2.txt >> ise.txt; cat ise2.txt >> ise_win32.txt; rm ise2.txt; \
	echo "[STRATEGY-LIST]" >> $(TOP).npl; echo "[STRATEGY-LIST]" >> $(TOP)_synplify.npl; \
	echo "Normal=True" >> $(TOP).npl; echo "Normal=True" >> $(TOP)_synplify.npl; \
	echo "DEVSYNTHESISTOOL XST (VHDL/Verilog)" >>  $(TOP).npl; \
	for i in $(VHDLSYNFILES) $(VHDLSIMFILES); do \
	   echo Project_File_$$nfiles = $$i >> tmp.mpf; \
	   echo Project_File_P_$$nfiles = vhdl_novitalcheck 0 file_type VHDL group_id 0 vhdl_nodebug 0 vhdl_1164 1 vhdl_noload 0 vhdl_synth 0 folder {Top Level} last_compile 0 vhdl_disableopt 0 vhdl_vital 0 vhdl_warn1 0 vhdl_warn2 1 vhdl_explicit 0 vhdl_showsource 1 vhdl_warn3 1 vhdl_options {} vhdl_warn4 1 ood 0 vhdl_warn5 0 compile_to $$bn compile_order $$nfiles dont_compile 0 cover_stmt 1 vhdl_use93 93 >> tmp.mpf; \
	   nfiles=`expr $$nfiles + 1`; \
	done; rm tmp.npl; \
        echo "Project_Sim_Count = 1" >> tmp.mpf; \
	echo "Project_Sim_0 = Simulation 1" >> tmp.mpf; \
	echo "Project_Sim_P_0 = Generics {} timing default -std_output {} +notimingchecks 0 -L {} selected_du {} -hazards 0 -sdf {} +acc {} ok 1 folder {Top Level} -absentisempty 0 +pulse_r {} OtherArgs {} -multisource_delay {} +pulse_e {} -coverage 0 -sdfnoerror 0 +plusarg {} -vital2.2b 0 -t ps additional_dus" work.$(SIMTOP) "-nofileshare 0 -noglitch 0 -wlf {} +no_pulse_msg 0 -assertfile {} -sdfnowarn 0 -Lf {} -std_input {}" >> tmp.mpf; \
	grep -v unisim compile.synp > xx; mv xx compile.synp; \
	for i in $(XSTSKIP) ; do grep -v $$i compile.xst > xx; mv xx compile.xst; done;\
	for i in $(XSTSKIP) ; do grep -v $$i $(TOP).npl > xx; mv xx $(TOP).npl; done;\
	for i in actel_gen atc18_gen virage_gen umc_gen ; do grep -v $$i $(TOP).npl > xx; mv xx $(TOP).npl; done;\
	for i in $(QUARTUS_SKIP); do grep -v $$i $(TOP).qsf > xx; mv xx $(TOP).qsf; done;\
	for i in clkgen_xilinx clkgen_altera altera_gen xilinx_gen; do grep -v $$i compile.dc > xx; mv xx compile.dc; done;\
	for i in clkgen_xilinx clkgen_altera altera_gen xilinx_gen; do grep -v $$i compile.rc > xx; mv xx compile.rc; done;\
	for i in $(GHDLSKIP); do grep -v $$i compile.ghdl > xx; mv xx compile.ghdl; done;\
	cat $(GRLIB)/bin/modelsim.ini >> modelsim.ini; \
	cp modelsim.ini $(SIMTOP).mpf; \
	echo "[Project]" >> $(SIMTOP).mpf; \
	echo Project_Version = 5 >> $(SIMTOP).mpf; \
	echo Project_DefaultLib = work >> $(SIMTOP).mpf; \
	echo Project_SortMethod = unused >> $(SIMTOP).mpf; \
        echo Project_Files_Count = $$nfiles >> $(SIMTOP).mpf; \
        cat tmp.mpf >> $(SIMTOP).mpf; rm tmp.mpf; \
	cat $(GRLIB)/bin/mt1.mpf >> $(SIMTOP).mpf; \
	echo -P$$ppath > ghdl.path; \
	if test -r "$(QSF)"; then cat $(QSF) >> $(TOP).qsf; fi; \
	echo ghdl: > tmpmake.ghdl;  cat compile.ghdl >> tmpmake.ghdl; \
	echo ncsim: > make.ncsim; cat compile.ncsim >> make.ncsim; \
	echo vsim: > make.vsim; cat compile.vsim >> make.vsim; \
	for i in $(VHDLSYNFILES) $(VHDLSIMFILES); do \
	   echo -e \\t$(VCOM) $(VHDLOPT) $$bn $$i >> make.vsim; \
	   echo -e \\t$(NCVHDL) $(VHDLOPT) $$bn $$i >> make.ncsim; \
	   echo -e \\t$(GHDL) $(VHDLOPT) --workdir=gnu/$$bn --work=$$bn -P$$ppath $$i >> tmpmake.ghdl; \
	done; \
	if test -r "$(SIMTOP)".vhd; then \
	  arch=`grep -i architecture $(SIMTOP).vhd | grep -i $(SIMTOP) | awk '{ print $$2}'`; \
	  echo -e \\tncelab $(SIMTOP):$$arch >> make.ncsim ; \
	fi; \
	if test -r "$(XILINX)"/data/projnav/iseEdit.tcl; then \
	  touch $(TOP).ise; rm $(TOP).ise; touch $(TOP).ise; \
	  xtclsh "$(XILINX)"/data/projnav/iseEdit.tcl import datafile=ise.txt >& ise.err; rm ise.err; \
	  echo "$(TOP).ise"; \
	fi
	make $(TOP)_synplify.prj $(TOP).dc $(TOP).rc $(TOP).xst $(TOP).npl $(TOP).qsf $(TOP)_designer.tcl
	echo $(SIMTOP).mpf

######## Common cleaning   ####################

clean: $(CLEAN) vsim-clean ise-clean ncsim-clean ghdl-clean synplify-clean quartus-clean \
	actel-clean dc-clean rc-clean precision-clean
	-rm -rf verilog.txt tkparse.exe main.tk ahbrom outdata ahbrom.bin

scripts-clean:
	-rm -rf compile\.* libs.txt *.qsf *.qpf *\.ghdl ghdl.path \
	compile\.* libs.do make\.* *.xst cds.lib *.npl \
	config.h .config.old hdl.var $(TOP).dc  $(TOP).rc  \
	$(TOP)_synplify.prj $(SIMTOP).mpf tmpmake.ghdl \
	$(TOP)_designer.tcl $(TOP)_designer_act.tcl \
	$(TOP)_precision.tcl
#    modelsim.ini

distclean: $(CLEAN) clean scripts-clean

libclean:
	-@rm work.v; \
	for j in `cat libs.txt` work ; do \
	  rm $$j.vhd; \
	done;
	make distclean

none-clean:

######## xconfig targets   ####################

TKCONFIG= $(GRLIB)/bin/tkconfig

tkparse.o: $(TKCONFIG)/tkparse.c
	$(CC) -g -c $<

tkcond.o: $(TKCONFIG)/tkcond.c
	$(CC) -g -c $<

tkgen.o: $(TKCONFIG)/tkgen.c
	$(CC) -g -c $<


tkparse.exe: tkparse.o tkcond.o tkgen.o
	$(CC) -g tkparse.o tkcond.o tkgen.o -o tkparse.exe

lconfig.tk: config.in
	make main.tk
	cat $(TKCONFIG)/header.tk main.tk $(TKCONFIG)/tail.tk > lconfig.tk
	chmod a+x lconfig.tk

main.tk : config.in tkparse.exe
	./tkparse.exe config.in $(GRLIB) > main.tk

xconfig: lconfig.tk
	@if test -r "/usr/bin/wish84.exe"; then /usr/bin/wish84.exe -f lconfig.tk; \
	else wish -f lconfig.tk; fi; \
	if test $$? = "2" ; then                   \
	   cpp -P -DGRLIB_PATH=$(GRLIB) config.vhd.in > config.vhd; \
	   echo config.vhd created; \
	fi

xdep:
	cpp -P -DGRLIB_PATH=$(GRLIB) config.vhd.in > config.vhd

config:
	cp $(GRLIB)/boards/$(BOARD)/config .config
	cp $(GRLIB)/boards/$(BOARD)/config.h config.h
	make xdep

.PHONY: xst precision synplify scripts netlists bitfiles cut-actel

############ Maintenance - do not remove!   ##################

docs:
	find ./ -name '*.gif' | zip ../grdocs.zip -@
	find ./ -name '*.html' | zip ../grdocs.zip -@
	find ./ -name '*.pdf' | zip ../grdocs.zip -@
	find ./ -name '*.jpg' | zip ../grdocs.zip -@

netlists:
	tar cvzf ../grlib-netlists-$(VER).tar.gz netlists
	rm -rf netlists

bitfiles:
	tar cvzf ../grlib-bitfiles-$(VER).tar.gz `find -name '*bitfiles' -print | xargs`
	rm -rf `find -name '*bitfiles' -print | xargs`

clean-lib:
	-rm `find -name '*.backup.fm' -print | xargs`
	-rm `find -name '*.recover.fm' -print | xargs`
	-rm `find -name '*.backup.book' -print | xargs`
	-rm `find -name '*.backup.book' -print | xargs`
	echo "-- pragma translate_off"   > lib/grlib/stdlib/version.vhd
	echo "use std.textio.all;"   >> lib/grlib/stdlib/version.vhd
	echo "-- pragma translate_on"   >> lib/grlib/stdlib/version.vhd
	echo "package version is"   >> lib/grlib/stdlib/version.vhd
	echo "  constant grlib_version : integer := $(GVER);" >> lib/grlib/stdlib/version.vhd
	echo "-- pragma translate_off"   >> lib/grlib/stdlib/version.vhd
	echo -n "  constant grlib_date : string := \""  >> lib/grlib/stdlib/version.vhd
	date +%Y%m%d | tr "\n" "\"" >> lib/grlib/stdlib/version.vhd
	echo ";"                                                >> lib/grlib/stdlib/version.vhd
	echo "-- pragma translate_on"   >> lib/grlib/stdlib/version.vhd
	echo -n "  constant grlib_build : integer := " >> lib/grlib/stdlib/version.vhd
	svn info | awk '/Revision:/ {print $$2}'  | tr "\n" ";" >> lib/grlib/stdlib/version.vhd
	echo "" >> lib/grlib/stdlib/version.vhd
	echo "end;"                                             >> lib/grlib/stdlib/version.vhd

gpl:
	make -f bin/Makefile netlists
	rm -rf lib/fpu/grfpc lib/fpu/grfpu lib/fpu/meiko lib/fpu/mfpc \
	lib/fpu/grlfpc lib/fpu/grfpulite lib/fpu/meiko lib/fpu/mfpc  \
	lib/fpu/ncsim lib/fpu/sim lib/fpu/netl \
	designs/leon3 designs/edsu designs/leon3edsu TODO.txt \
	lib/gaisler/*ft lib/gaisler/*/*ft* lib/gaisler/misc/ft* \
	lib/gaisler/leon3/doc/*ft* lib/micron/sdram/ft* \
	lib/grlib/ftlib software/leon3/*ft* lib/esa/leon2 \
	lib/gaisler/leon3/grfpcw2.vhd lib/gaisler/leon3/grfpcx.vhd \
	lib/gaisler/leon3/grlfpcw2.vhd lib/gaisler/leon3/grlfpcx.vhd \
	lib/gaisler/leon3/mfpcw2.vhd lib/gaisler/leon3/mfpcx.vhd \
	lib/spw/core lib/techmap/clocks/clkgen_rh_lib18t.vhd \
	lib/topnet designs/esa_spw_tb designs/leon3ax designs/leon3-1553 \
	netlists/*/*/* designs/tb_msp  designs/rs_gf4_16_8 \
	designs/tmtc lib/tmtc designs/suidemo designs/spacewire \
	designs/*esa* designs/*ft* lib/actel designs/greth \
	designs/*flexody* designs/*prisma* designs/*/rhlib* \
	designs/ethspw designs/leon3ihp lib/gr1553 lib/gaisler/b1553 \
	lib/techmap/memory/mem_rh_lib18t_gen.vhd lib/tech/ramon \
	lib/techmap/pads/pad_rh_lib18t_gen.vhd \
	lib/techmap/*/*ft.vhd lib/ssc lib/gaisler/misc/ahb2ahb.vhd \
	lib/gaisler/misc/ahbbridge.vhd
	mv software/leon3/Makefile.gpl software/leon3/Makefile
	-rm `find -name '*.fm' -print | xargs`
	-rm `find -name '*.book' -print | xargs`
	make -f bin/Makefile bitfiles

.PHONY: xst synplify scripts netlists bitfiles

axsmall:
	cp $(GRLIB)/lib/actel/axcelerator/components/axcelerator_small.vhd \
	   $(GRLIB)/lib/actel/axcelerator/components/axcelerator.vhd
	cp $(GRLIB)/lib/actel/axcelerator/components/axcelerator_components_small.vhd \
	   $(GRLIB)/lib/actel/axcelerator/components/axcelerator_components.vhd

axfull:
	cp $(GRLIB)/lib/actel/axcelerator/components/axcelerator_full.vhd \
	   $(GRLIB)/lib/actel/axcelerator/components/axcelerator.vhd
	cp $(GRLIB)/lib/actel/axcelerator/components/axcelerator_components_full.vhd \
	   $(GRLIB)/lib/actel/axcelerator/components/axcelerator_components.vhd

cut-xilinx:
	mkdir x; mv boards/gr-* x; rm -rf boards/*; mv x/* boards; rm -rf boards/gr-cpci-ax
	mv designs/leon3-gr* x; rm -rf designs/*; mv x/* designs; rm -rf designs/leon3-gr-cpci-ax
	mv lib/tech/xilinx lib/tech/synplify x; rm -rf lib/tech/*; mv x/* lib/tech
	cd lib/techmap; rm -rf buffers/clkbuf_actel.vhd \
	clocks/clkgen_actel.vhd clocks/clkgen_altera.vhd memory/mem_altera_gen.vhd \
	memory/mem_ihp25_gen.vhd memory/mem_lattice_gen.vhd ddr/ddr_lattice.vhd \
	memory/mem_umc_gen.vhd memory/mem_virage_gen.vhd memory/mem_apa_gen.vhd \
	memory/mem_apa3_gen.vhd memory/mem_axcelerator_gen.vhd \
	pads/pad_atc18_gen.vhd pads/pad_ihp25_gen.vhd pads/pad_rhumc_gen.vhd \
	pads/pad_actel_gen.vhd  ddr/ddr_lattice.vhd 
	cd lib; rm -rf contrib gleichmann openchip 

