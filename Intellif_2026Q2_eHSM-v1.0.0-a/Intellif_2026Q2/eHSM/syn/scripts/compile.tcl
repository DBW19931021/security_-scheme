#Initial dc env
source -e -v ../scripts/dc_setup.tcl

#Elaborate design
#svf record for fm
if { $PUSH >0 } {
    set_svf ./svf/rtl2elab_${TECH}_${CORNER}_LIMIT.svf
} else {
    set_svf ./svf/rtl2elab_${TECH}_${CORNER}.svf
}

set rtl_list [open ${RTL_LIST} r]
while {[gets $rtl_list line] != -1} {
    if { [regsub {^\s*\.\./} $line "analyze -format sverilog \{../../rtl/osr_define_cfg.v ../../rtl/osr_define_sim.v ../../rtl/osr_define_key.v ../../" x] == 1 } {
        puts "$x\}"
        eval "$x\}"
    }
}

elaborate ${top}

current_design ${top}


#rename_design ${top}* ${top}

set_svf -off
if { $PUSH >0 } {
    write -f ddc -hier -out ${ddc_path}${top}_elab_${TECH}_${CORNER}_LIMIT.ddc
} else {
    write -f ddc -hier -out ${ddc_path}${top}_elab_${TECH}_${CORNER}.ddc
}

set_register_merging ${top}* false

#Sdc apply and check
current_design ${top}*
source -e -v ../scripts/constraint.tcl
if { $PUSH >0 } {
    check_design > ${report_path}${top}_${TECH}_${CORNER}_LIMIT.check_design
    check_timing > ${report_path}${top}_${TECH}_${CORNER}_LIMIT.check_timing
} else {
    check_design > ${report_path}${top}_${TECH}_${CORNER}.check_design
    check_timing > ${report_path}${top}_${TECH}_${CORNER}.check_timing
}

set_fix_multiple_port_nets -feedthroughs -outputs -buffer_constants

#Compile_ultra
#Record svf for fm
if { $PUSH >0 } {
    set_svf ./svf/rtl2comp_${TECH}_${CORNER}_LIMIT.svf
} else {
    set_svf ./svf/rtl2comp_${TECH}_${CORNER}.svf
}
#set_clock_gating_objects -exclude [get_cells u_ram]
#compile_ultra -gate_clock -no_autoungroup -no_seq_output_inversion

compile_ultra -no_autoungroup -no_boundary_optimization

#Output & reports
define_name_rules verilog -case_insensitive
report_name_rules verilog
change_names -rule verilog -hier 

set_svf -off

if { $PUSH >0 } {
    report_timing -from [all_inputs] -to [get_pins -h */D]
    report_timing -from [get_pins -h */Q] -to [get_pins -h */D]
    report_timing -from [get_pins -h */Q] -to [all_outputs]
    report_timing -from [all_inputs] -to [all_outputs]

    write -f ddc -hier -out ${ddc_path}${top}_comp_${TECH}_${CORNER}_LIMIT.ddc
    write -f verilog -hier -out ${netlist_path}${top}_${TECH}_${CORNER}_LIMIT.v 
    write_sdc ${netlist_path}${top}_${TECH}_${CORNER}_LIMIT.sdc
    report_area > ${report_path}${top}_${TECH}_${CORNER}_LIMIT.rpt
    report_reference -hierarchy >> ${report_path}${top}_${TECH}_${CORNER}_LIMIT.rpt
    report_clock_gating -hier  >> ${report_path}${top}_${TECH}_${CORNER}_LIMIT.rpt
    report_constraints -all_violators > ${report_path}${top}_violators_${TECH}_${CORNER}_LIMIT.rpt
    report_constraints -all_violators -verbose >> ${report_path}${top}_violators_${TECH}_${CORNER}_LIMIT.rpt
    report_timing > ${report_path}${top}_critical_path_${TECH}_${CORNER}_LIMIT.rpt
} else {
    write -f ddc -hier -out ${ddc_path}${top}_comp_${TECH}_${CORNER}.ddc
    write -f verilog -hier -out ${netlist_path}${top}_${TECH}_${CORNER}.v 
    write_sdf ${netlist_path}${top}_${TECH}_${CORNER}.sdf
    write_sdc ${netlist_path}${top}_${TECH}_${CORNER}.sdc
    report_area > ${report_path}${top}_${TECH}_${CORNER}.rpt
    report_reference -hierarchy >> ${report_path}${top}_${TECH}_${CORNER}.rpt
    report_clock_gating -hier  >> ${report_path}${top}_${TECH}_${CORNER}.rpt
    report_constraints -all_violators > ${report_path}${top}_violators_${TECH}_${CORNER}.rpt
    report_constraints -all_violators -verbose >> ${report_path}${top}_violators_${TECH}_${CORNER}.rpt
    report_timing > ${report_path}${top}_critical_path_${TECH}_${CORNER}.rpt
}

if {$REGRESSION > 0 } {
    quit!
} else {
}
