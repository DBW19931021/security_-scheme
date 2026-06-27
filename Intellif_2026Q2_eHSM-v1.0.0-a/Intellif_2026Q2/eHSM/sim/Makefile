#-------------------------------------------------------------------------------
# declare parameter  
TOP=tb
TC=speed_test
OUT_DIR=wave
SEED = $$(date +'%d%H%M%S')

FPGA=0
waveswitch=on
wavetime=0
userdef=
bootselftest=OFF
TBM=0 # using trng model in TB
IMAGE_EHSM=0
bootinstall=OFF
bootalg=AES
#bootalg=SM4

OSR_SIM_POWER_TEST=0


SIM_MS=1000

# for FSDB AUTO SWITCH OPTION
ATW  =1	#auto switch wave size
FSDB_AUTO_SWITCH_SIZE=100
FSDB_AUTO_SWITCH_NUM =999

# set 1 for TB to load firmware to IRAM directly
FAST_SIM=1

#-------------------------------------------------------------------------------
# for generating define for rtl/tb/tc using parameter above
ifeq ($(FPGA),1)
SIMDEF  +=+define+OSR_FPGA
endif

ifeq ($(bootselftest),ON)
SIMDEF  +=+define+OSR_SIM_ENABLE_SELF_TEST
else ifeq ($(bootselftest),OFF)
SIMDEF  +=+define+OSR_SIM_BYPASS_SELF_TEST
else
SIMDEF  +=+define+OSR_SIM_BYPASS_SELF_TEST
endif

ifeq ($(bootinstall),ON)
SIMDEF  +=+define+OSR_SIM_INSTALL_ON
endif

ifeq ($(OSR_SIM_POWER_TEST),1)
SIMDEF  +=+define+OSR_SIM_POWER_TEST
endif

ifeq ($(bootalg),RSA)
SIMDEF  +=+define+OSR_SIM_BOOT_RSA
else ifeq ($(bootalg),SM2)
SIMDEF  +=+define+OSR_SIM_BOOT_SM2
else ifeq ($(bootalg),AES)
SIMDEF  +=+define+OSR_SIM_BOOT_AES
else ifeq ($(bootalg),SM4)
SIMDEF  +=+define+OSR_SIM_BOOT_SM4
else
SIMDEF  +=+define+OSR_SIM_BOOT_RSA
endif

ifeq ($(TBM),1)
SIMDEF  +=+define+OSR_TB_TRNG_MODEL_DONT_DEFINE_IN_SYNTHESIS
endif

ifeq ($(IMAGE_EHSM),1)
SIMDEF  +=+define+OSR_SIM_IMAGE_EHSM
endif



#SIMDEF   +=+define+UPPER_N205
SIMDEF   +=+define+OSR_SIM_PLL
SIMDEF   +=+define+OSR_TRNG_SIM
SIMDEF   +=+define+SIM_MS=$(SIM_MS)

ifeq ($(FAST_SIM),1)
SIMDEF   +=+define+OSR_SIM_FAST_SIM
endif

#-------------------------------------------------------------------------------
# for reading filelist

SRCFILES += -F ../../filelist/ehsm_rtl.f
SRCFILES += -F ../../fpga/src/fl/ehsm_fpga_sim.f

#-------------------------------------------------------------------------------
# for reading fpga modelm, tb and tc file
SRCFILES +=    ../tb/tb.sv 
SRCFILES += -v ../tb/cell/u055lscee12bbl_sdf30.v 
SRCFILES +=    ../tc/tc.sv 
SRCFILES +=    ../tb/fpga_uart_monitor.v



INCDIR += +incdir+../../romcode
INCDIR += +incdir+../../fpga/src
INCDIR += +incdir+../../fpga/src/misc
INCDIR += +incdir+../../fpga/src/cpu
INCDIR += +incdir+../../fpga/src/axi2sram
INCDIR += +incdir+../tb
INCDIR += +incdir+../tc
INCDIR += +incdir+../../rtl/cpu/Wing_M100A
INCDIR += +incdir+../../rtl/cpu/cm4/models/cell
INCDIR += +incdir+../../rtl/cpu/cm4/dapswjdp/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cortexm4_integration/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cortexm4/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_wic/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_tpiu/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_nvic/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_mpu/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_lic_defs/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_itm/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_fpu/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_fpb/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_dwt/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_dpu/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_dap_ahb_ap/verilog
INCDIR += +incdir+../../rtl/cpu/cm4/cm4_bus_matrix/verilog

#-------------------------------------------------------------------------------
# vcs option 
TEST_FSDB_FILES = $(addprefix $(OUT_DIR)/,$(addsuffix .fsdb,$(TC)))
VCS_OPT  = -ntb_opts rvm +v2k -sverilog +memcbk +vcs+dumparrays +lint=TFIPC-L +lint=PCWM  
VCS_OPT += -Mupdate +notimingcheck +nospecify +plusarg_save +acc -l vcs.log
VCS_OPT += +rad+2 -j4 
VCS_OPT += -override_timescale=1ns/1ps

#-------------------------------------------------------------------------------
# FSDB option 
FSDB_OPT += +fsdb_auto_switch_en=$(ATW)
FSDB_OPT += +fsdb_auto_switch_size=$(FSDB_AUTO_SWITCH_SIZE)
FSDB_OPT += +fsdb_auto_switch_num=$(FSDB_AUTO_SWITCH_NUM)
#FSDB_OPT += +fsdbfile=$(OUT_DIR)/$(TC)_$(SEED).fsdb
FSDB_OPT += +fsdbfile=$(OUT_DIR)/$(TC).fsdb

#-------------------------------------------------------------------------------
# verdi option 
ifeq ($(ATW),0)
VERDI_OPT += -ssf $(OUT_DIR)/$(TC).fsdb
else
VERDI_OPT += -ssf $(OUT_DIR)/$(TC).fsdb
endif

#-------------------------------------------------------------------------------
TB_OPT += 


# declare make command
com_sim: com sim

com: inf init 
	@mkdir -p ./run
	@cd ./run && vcs -full64 -debug_access+all  +vcs+lic+wait -o simv $(SIMDEF) $(VCS_OPT) $(SRCFILES) $(INCDIR) $(LIBDIR) $(IP_OPT) -top $(TOP) +define+${userdef} && [ $$PIPESTATUS -eq 0 ]

sim:
	@gnome-terminal --window --title=$(TC)_$(SEED) -e "./check  -t ${TC} -s ${SEED}"&

run_test: 
	@cd ./run && if test ! -e simv; then echo "Error: simv not exist! Run 'make com' first."; exit 1; fi
	@if [ ! -d ./run/$(OUT_DIR) ]; then mkdir ./run/$(OUT_DIR); fi
	@if [ ! -d ./run/log ]; then mkdir ./run/log; fi
	@if [ ! -d ./run/rpt ]; then mkdir ./run/rpt; fi
	@cd ./run && ./simv +vcs+initreg+0 +autofindloop -l log/$(TC)_$(SEED).log $(FSDB_OPT) $(TB_OPT) +tc_name=$(TC) +wave=${waveswitch} +wavetime=${wavetime} && [ $$PIPESTATUS -eq 0 ] 

verdi: inf
	@mkdir -p ./run
	@cd ./run && verdi $(SIMDEF) -sv -top $(TOP) $(SRCFILES) $(INCDIR) $(LIBDIR) $(IP_OPT) $(VERDI_OPT) &	

clean:
	@cd ./run && rm csrc *sim* *.* *verdi* -rf 
	@cd ./run && rm -rf log wave cov rpt

init:
	@mkdir -p ./run
	@touch ./run/init.data
	@rm ./run/wave/$(TC).fsdb* -rf

killgvim:
	@killall -9 -u `whoami` gvim

kill:
	@killall -9 -u `whoami` check 
	@killall -9 -u `whoami` simv 
	@killall -9 -u `whoami` make 


inf:
	@echo  
ifeq ($(FPGA),1)
	@echo define FPGA 
endif

ifeq ($(OSR_SIM_POWER_TEST),1)
	@echo define OSR_SIM_POWER_TEST
endif

ifeq ($(IMAGE_EHSM),1)
	@echo define IMAGE_EHSM
endif


	@echo TC=$(TC)


help:
	$(info make                               : compile    )
	$(info make sim TC=test1                  : simulation with TC=test1 )
	$(info make sim TC=test1 FPGA=1           : simulation with TC=test1 & define FPGA )
	$(info make verdi                         : open verdi )
	$(info waveswitch                         : on dump fsdb /off no dump fsdb )
	$(info wavetime                           : start dump wave time/ms )
	$(info bootselftest                       : ON ENABLE SELFTEST /OFF BYPASS SELFTEST )
	$(info bootinstall                        : ON SECURE_INSTALL /OFF SECURE_BOOT )
	$(info bootalg                            : SECURE_BOOT/SECURE_INSTALL ALG : AES128, RSA, SM2, SM4)


