set SYS_CLK_PIN i_clk
set SYS_RST_PIN i_rst_n
set jtag_TCK_period  100
#set SYS_CLK_DIV_PIN i_clk_div
#set SYS_CLK_DIV_N   2

if { $PUSH >0 } {
    set PERIOD 5 
}

#scenario
#set_operating_conditions WCCOM -library fsh0l_bls_generic_core_ss0p88v125c

#wire load model
#select amount top,enclosed,segmented
set_wire_load_mode segmented

#physical constraint
#input transition slew
#input pin load
#max_fanout & max_transition would not effect if the value is higher than the one in lib
set_input_transition 0.5 [all_inputs]
set_load 0.6 [all_outputs]
set_max_fanout 25 [current_design]
set_max_transition 2.8 [current_design]

#clock scheme
#period 15ns ~ 66.7MHz
#skew model as 200ps
#delay model as 100ps
create_clock -name clk -period $PERIOD [get_port $SYS_CLK_PIN] 
set_clock_uncertainty -setup 0.2 [get_clock clk]
set_clock_uncertainty -hold 0.2 [get_clock clk]
set_clock_transition 0.5 [get_clock clk]
set_clock_latency 0.1 [get_clock clk]

#create_generated_clock -name clk_div [get_port $SYS_CLK_DIV_PIN] -source [get_port $SYS_CLK_PIN] -divide_by $SYS_CLK_DIV_N
#set_clock_uncertainty -setup 0.2 [get_clock clk_div]
#set_clock_uncertainty -hold 0.2 [get_clock clk_div]
#set_clock_transition 0.5 [get_clock clk_div]
#set_clock_latency 0.1 [get_clock clk_div]




#set ideal & dont_touch network for clock and reset_n
#forbid use buffer insertion
set_dont_touch_network [all_clocks]
set_ideal_network -no_propagate $SYS_CLK_PIN
set_ideal_network -no_propagate $SYS_RST_PIN

set_ideal_network -no_propagate [get_pins u_cgu/OSR_DONTTOUCH_u_icg_*/OSR_DONTTOUCH_icg_cell/GCK]

#constrain in2reg & reg2out path
set_input_delay -max 1.0 -clock clk [remove_from_collection [all_inputs] [list $SYS_CLK_PIN $SYS_RST_PIN]]
set_input_delay -min 0.1 -clock clk [remove_from_collection [all_inputs] [list $SYS_CLK_PIN $SYS_RST_PIN]]
set_output_delay -max 1.0 -clock clk [all_outputs]
set_output_delay -min 0.1 -clock clk [all_outputs]

#uart
create_clock -name tck -period $jtag_TCK_period [get_port i_tck]

set_clock_groups  -asynchronous -group {clk} -group {tck}
set_dont_touch [list \
    [get_cells -hierarchical *OSR_DONTTOUCH*]
]

# find all OSR_DONTTOUCH_n01
# and disable timing on Z.
# In each slow ros1/ros2/ros3/ros4 ring, only 1 OSR_DONTTOUCH_n01
set n01_cell [get_cells -hier *OSR_DONTTOUCH_n01*]
foreach_in_collection item $n01_cell {
    set_disable_timing [get_pins -of_object $item -filter name=="Z"]
}

# find all OSR_DONTTOUCH_n1
# and disable timing on Z.
# In each slow ros1/ros2/ros3/ros4 ring, only 1 OSR_DONTTOUCH_n1
set n1_cell [get_cells -hier *OSR_DONTTOUCH_n1*]
foreach_in_collection item $n1_cell {
    set_disable_timing [get_pins -of_object $item -filter name=="Z"]
}

# find all OSR_DONTTOUCH_out
# and set max delay on Z.
# In each slow ros1/ros2/ros3/ros4 ring, have 17 OSR_DONTTOUCH_nout, but only get 16 for ro fast loop
set ro_nout_cell [filter_collection [get_cells -hierarchical *OSR_DONTTOUCH_nout*] {full_name =~ "*ro_*"}]

set_max_delay 0.1 -from [get_pins -of_object ${ro_nout_cell} -filter "name == Z"]


# get inv cell on slow ring. Define clock on Z
set slow_clock_cell    [filter_collection [get_cells -hierarchical  *clk_mux_div01*] {full_name =~ "*roclk*"}]
set slow_clock_cell_01 [filter_collection $slow_clock_cell {full_name =~ "*ros1*"}]
set slow_clock_cell_02 [filter_collection $slow_clock_cell {full_name =~ "*ros2*"}]
set slow_clock_cell_03 [filter_collection $slow_clock_cell {full_name =~ "*ros3*"}]
set slow_clock_cell_04 [filter_collection $slow_clock_cell {full_name =~ "*ros4*"}]
# get registers of clock divider
set slow_clock_01_divider_02_registers [filter_collection [get_cells -hier *clk_div02*] {full_name =~ "*ros1*"}]
set slow_clock_01_divider_04_registers [filter_collection [get_cells -hier *clk_div04*] {full_name =~ "*ros1*"}]
set slow_clock_01_divider_08_registers [filter_collection [get_cells -hier *clk_div08*] {full_name =~ "*ros1*"}]
set slow_clock_01_divider_16_registers [filter_collection [get_cells -hier *clk_div16*] {full_name =~ "*ros1*"}]
set slow_clock_01_divider_32_registers [filter_collection [get_cells -hier *clk_div32*] {full_name =~ "*ros1*"}]
set slow_clock_02_divider_02_registers [filter_collection [get_cells -hier *clk_div02*] {full_name =~ "*ros2*"}]
set slow_clock_02_divider_04_registers [filter_collection [get_cells -hier *clk_div04*] {full_name =~ "*ros2*"}]
set slow_clock_02_divider_08_registers [filter_collection [get_cells -hier *clk_div08*] {full_name =~ "*ros2*"}]
set slow_clock_02_divider_16_registers [filter_collection [get_cells -hier *clk_div16*] {full_name =~ "*ros2*"}]
set slow_clock_02_divider_32_registers [filter_collection [get_cells -hier *clk_div32*] {full_name =~ "*ros2*"}]
set slow_clock_03_divider_02_registers [filter_collection [get_cells -hier *clk_div02*] {full_name =~ "*ros3*"}]
set slow_clock_03_divider_04_registers [filter_collection [get_cells -hier *clk_div04*] {full_name =~ "*ros3*"}]
set slow_clock_03_divider_08_registers [filter_collection [get_cells -hier *clk_div08*] {full_name =~ "*ros3*"}]
set slow_clock_03_divider_16_registers [filter_collection [get_cells -hier *clk_div16*] {full_name =~ "*ros3*"}]
set slow_clock_03_divider_32_registers [filter_collection [get_cells -hier *clk_div32*] {full_name =~ "*ros3*"}]
set slow_clock_04_divider_02_registers [filter_collection [get_cells -hier *clk_div02*] {full_name =~ "*ros4*"}]
set slow_clock_04_divider_04_registers [filter_collection [get_cells -hier *clk_div04*] {full_name =~ "*ros4*"}]
set slow_clock_04_divider_08_registers [filter_collection [get_cells -hier *clk_div08*] {full_name =~ "*ros4*"}]
set slow_clock_04_divider_16_registers [filter_collection [get_cells -hier *clk_div16*] {full_name =~ "*ros4*"}]
set slow_clock_04_divider_32_registers [filter_collection [get_cells -hier *clk_div32*] {full_name =~ "*ros4*"}]

# get mux cell on slow ring. Define clock on Z
set slow_clock_mux_cell    [filter_collection [get_cells -hierarchical  *clk_div_mux_03*] {full_name =~ "*roclk*"}]
set slow_clock_mux_cell_01 [filter_collection $slow_clock_mux_cell {full_name =~ "*ros1*"}]
set slow_clock_mux_cell_02 [filter_collection $slow_clock_mux_cell {full_name =~ "*ros2*"}]
set slow_clock_mux_cell_03 [filter_collection $slow_clock_mux_cell {full_name =~ "*ros3*"}]
set slow_clock_mux_cell_04 [filter_collection $slow_clock_mux_cell {full_name =~ "*ros4*"}]


create_clock -name ro1_clk_in    -period 5 [get_pins -of_object ${slow_clock_cell_01} -filter "name == Z"]
create_generated_clock -name ro1_clk_div02 -divide_by 2 -source [get_pins -of_object ${slow_clock_cell_01} -filter "name == Z"] \
                       -master_clock ro1_clk_in [get_pins -of_object ${slow_clock_01_divider_02_registers} -filter "name == Q"]
create_generated_clock -name ro1_clk_div04 -divide_by 2 -source [get_pins -of_object ${slow_clock_01_divider_02_registers} -filter "name == Q"] \
                       -master_clock ro1_clk_div02 [get_pins -of_object ${slow_clock_01_divider_04_registers} -filter "name == Q"]
create_generated_clock -name ro1_clk_div08 -divide_by 2 -source [get_pins -of_object ${slow_clock_01_divider_04_registers} -filter "name == Q"] \
                       -master_clock ro1_clk_div04 [get_pins -of_object ${slow_clock_01_divider_08_registers} -filter "name == Q"]
create_generated_clock -name ro1_clk_div16 -divide_by 2 -source [get_pins -of_object ${slow_clock_01_divider_08_registers} -filter "name == Q"] \
                       -master_clock ro1_clk_div08 [get_pins -of_object ${slow_clock_01_divider_16_registers} -filter "name == Q"]
create_generated_clock -name ro1_clk_div32 -divide_by 2 -source [get_pins -of_object ${slow_clock_01_divider_16_registers} -filter "name == Q"] \
                       -master_clock ro1_clk_div16 [get_pins -of_object ${slow_clock_01_divider_32_registers} -filter "name == Q"]

create_clock -name ro2_clk_in    -period 5 [get_pins -of_object ${slow_clock_cell_02} -filter "name == Z"]
create_generated_clock -name ro2_clk_div02 -divide_by 2 -source [get_pins -of_object ${slow_clock_cell_02} -filter "name == Z"] \
                       -master_clock ro2_clk_in [get_pins -of_object ${slow_clock_02_divider_02_registers} -filter "name == Q"]
create_generated_clock -name ro2_clk_div04 -divide_by 2 -source [get_pins -of_object ${slow_clock_02_divider_02_registers} -filter "name == Q"] \
                       -master_clock ro2_clk_div02 [get_pins -of_object ${slow_clock_02_divider_04_registers} -filter "name == Q"]
create_generated_clock -name ro2_clk_div08 -divide_by 2 -source [get_pins -of_object ${slow_clock_02_divider_04_registers} -filter "name == Q"] \
                       -master_clock ro2_clk_div04 [get_pins -of_object ${slow_clock_02_divider_08_registers} -filter "name == Q"]
create_generated_clock -name ro2_clk_div16 -divide_by 2 -source [get_pins -of_object ${slow_clock_02_divider_08_registers} -filter "name == Q"] \
                       -master_clock ro2_clk_div08 [get_pins -of_object ${slow_clock_02_divider_16_registers} -filter "name == Q"]
create_generated_clock -name ro2_clk_div32 -divide_by 2 -source [get_pins -of_object ${slow_clock_02_divider_16_registers} -filter "name == Q"] \
                       -master_clock ro2_clk_div16 [get_pins -of_object ${slow_clock_02_divider_32_registers} -filter "name == Q"]

create_clock -name ro3_clk_in    -period 5 [get_pins -of_object ${slow_clock_cell_03} -filter "name == Z"]
create_generated_clock -name ro3_clk_div02 -divide_by 2 -source [get_pins -of_object ${slow_clock_cell_03} -filter "name == Z"] \
                       -master_clock ro3_clk_in [get_pins -of_object ${slow_clock_03_divider_02_registers} -filter "name == Q"]
create_generated_clock -name ro3_clk_div04 -divide_by 2 -source [get_pins -of_object ${slow_clock_03_divider_02_registers} -filter "name == Q"] \
                       -master_clock ro3_clk_div02 [get_pins -of_object ${slow_clock_03_divider_04_registers} -filter "name == Q"]
create_generated_clock -name ro3_clk_div08 -divide_by 2 -source [get_pins -of_object ${slow_clock_03_divider_04_registers} -filter "name == Q"] \
                       -master_clock ro3_clk_div04 [get_pins -of_object ${slow_clock_03_divider_08_registers} -filter "name == Q"]
create_generated_clock -name ro3_clk_div16 -divide_by 2 -source [get_pins -of_object ${slow_clock_03_divider_08_registers} -filter "name == Q"] \
                       -master_clock ro3_clk_div08 [get_pins -of_object ${slow_clock_03_divider_16_registers} -filter "name == Q"]
create_generated_clock -name ro3_clk_div32 -divide_by 2 -source [get_pins -of_object ${slow_clock_03_divider_16_registers} -filter "name == Q"] \
                       -master_clock ro3_clk_div16 [get_pins -of_object ${slow_clock_03_divider_32_registers} -filter "name == Q"]

create_clock -name ro4_clk_in    -period 5 [get_pins -of_object ${slow_clock_cell_04} -filter "name == Z"]
create_generated_clock -name ro4_clk_div02 -divide_by 2 -source [get_pins -of_object ${slow_clock_cell_04} -filter "name == Z"] \
                       -master_clock ro4_clk_in [get_pins -of_object ${slow_clock_04_divider_02_registers} -filter "name == Q"]
create_generated_clock -name ro4_clk_div04 -divide_by 2 -source [get_pins -of_object ${slow_clock_04_divider_02_registers} -filter "name == Q"] \
                       -master_clock ro4_clk_div02 [get_pins -of_object ${slow_clock_04_divider_04_registers} -filter "name == Q"]
create_generated_clock -name ro4_clk_div08 -divide_by 2 -source [get_pins -of_object ${slow_clock_04_divider_04_registers} -filter "name == Q"] \
                       -master_clock ro4_clk_div04 [get_pins -of_object ${slow_clock_04_divider_08_registers} -filter "name == Q"]
create_generated_clock -name ro4_clk_div16 -divide_by 2 -source [get_pins -of_object ${slow_clock_04_divider_08_registers} -filter "name == Q"] \
                       -master_clock ro4_clk_div08 [get_pins -of_object ${slow_clock_04_divider_16_registers} -filter "name == Q"]
create_generated_clock -name ro4_clk_div32 -divide_by 2 -source [get_pins -of_object ${slow_clock_04_divider_16_registers} -filter "name == Q"] \
                       -master_clock ro4_clk_div16 [get_pins -of_object ${slow_clock_04_divider_32_registers} -filter "name == Q"]

create_generated_clock -name ro1_clk_div04_sel -comb -source [get_pins -of_object ${slow_clock_01_divider_04_registers} -filter "name == Q"] -master_clock ro1_clk_div04 [get_pins -of_object ${slow_clock_mux_cell_01} -filter "name == Z"]
create_generated_clock -name ro1_clk_div08_sel -comb -source [get_pins -of_object ${slow_clock_01_divider_08_registers} -filter "name == Q"] -master_clock ro1_clk_div08 [get_pins -of_object ${slow_clock_mux_cell_01} -filter "name == Z"] -add
create_generated_clock -name ro1_clk_div16_sel -comb -source [get_pins -of_object ${slow_clock_01_divider_16_registers} -filter "name == Q"] -master_clock ro1_clk_div16 [get_pins -of_object ${slow_clock_mux_cell_01} -filter "name == Z"] -add
create_generated_clock -name ro1_clk_div32_sel -comb -source [get_pins -of_object ${slow_clock_01_divider_32_registers} -filter "name == Q"] -master_clock ro1_clk_div32 [get_pins -of_object ${slow_clock_mux_cell_01} -filter "name == Z"] -add

create_generated_clock -name ro2_clk_div04_sel -comb -source [get_pins -of_object ${slow_clock_02_divider_04_registers} -filter "name == Q"] -master_clock ro2_clk_div04 [get_pins -of_object ${slow_clock_mux_cell_02} -filter "name == Z"]
create_generated_clock -name ro2_clk_div08_sel -comb -source [get_pins -of_object ${slow_clock_02_divider_08_registers} -filter "name == Q"] -master_clock ro2_clk_div08 [get_pins -of_object ${slow_clock_mux_cell_02} -filter "name == Z"] -add
create_generated_clock -name ro2_clk_div16_sel -comb -source [get_pins -of_object ${slow_clock_02_divider_16_registers} -filter "name == Q"] -master_clock ro2_clk_div16 [get_pins -of_object ${slow_clock_mux_cell_02} -filter "name == Z"] -add
create_generated_clock -name ro2_clk_div32_sel -comb -source [get_pins -of_object ${slow_clock_02_divider_32_registers} -filter "name == Q"] -master_clock ro2_clk_div32 [get_pins -of_object ${slow_clock_mux_cell_02} -filter "name == Z"] -add

create_generated_clock -name ro3_clk_div04_sel -comb -source [get_pins -of_object ${slow_clock_03_divider_04_registers} -filter "name == Q"] -master_clock ro3_clk_div04 [get_pins -of_object ${slow_clock_mux_cell_03} -filter "name == Z"]
create_generated_clock -name ro3_clk_div08_sel -comb -source [get_pins -of_object ${slow_clock_03_divider_08_registers} -filter "name == Q"] -master_clock ro3_clk_div08 [get_pins -of_object ${slow_clock_mux_cell_03} -filter "name == Z"] -add
create_generated_clock -name ro3_clk_div16_sel -comb -source [get_pins -of_object ${slow_clock_03_divider_16_registers} -filter "name == Q"] -master_clock ro3_clk_div16 [get_pins -of_object ${slow_clock_mux_cell_03} -filter "name == Z"] -add
create_generated_clock -name ro3_clk_div32_sel -comb -source [get_pins -of_object ${slow_clock_03_divider_32_registers} -filter "name == Q"] -master_clock ro3_clk_div32 [get_pins -of_object ${slow_clock_mux_cell_03} -filter "name == Z"] -add

create_generated_clock -name ro4_clk_div04_sel -comb -source [get_pins -of_object ${slow_clock_04_divider_04_registers} -filter "name == Q"] -master_clock ro4_clk_div04 [get_pins -of_object ${slow_clock_mux_cell_04} -filter "name == Z"]
create_generated_clock -name ro4_clk_div08_sel -comb -source [get_pins -of_object ${slow_clock_04_divider_08_registers} -filter "name == Q"] -master_clock ro4_clk_div08 [get_pins -of_object ${slow_clock_mux_cell_04} -filter "name == Z"] -add
create_generated_clock -name ro4_clk_div16_sel -comb -source [get_pins -of_object ${slow_clock_04_divider_16_registers} -filter "name == Q"] -master_clock ro4_clk_div16 [get_pins -of_object ${slow_clock_mux_cell_04} -filter "name == Z"] -add
create_generated_clock -name ro4_clk_div32_sel -comb -source [get_pins -of_object ${slow_clock_04_divider_32_registers} -filter "name == Q"] -master_clock ro4_clk_div32 [get_pins -of_object ${slow_clock_mux_cell_04} -filter "name == Z"] -add

set_clock_groups -asynchronous -group  { clk           } \
                               -group  { ro1_clk_in      \
                                         ro1_clk_div02   \
                                         ro1_clk_div04   \
                                         ro1_clk_div08   \
                                         ro1_clk_div16   \
                                         ro1_clk_div32   \
                                         ro1_clk_div04_sel  \
                                         ro1_clk_div08_sel  \
                                         ro1_clk_div16_sel  \
                                         ro1_clk_div32_sel} \
                               -group  { ro2_clk_in      \
                                         ro2_clk_div02   \
                                         ro2_clk_div04   \
                                         ro2_clk_div08   \
                                         ro2_clk_div16   \
                                         ro2_clk_div32   \
                                         ro2_clk_div04_sel  \
                                         ro2_clk_div08_sel  \
                                         ro2_clk_div16_sel  \
                                         ro2_clk_div32_sel} \
                               -group  { ro3_clk_in      \
                                         ro3_clk_div02   \
                                         ro3_clk_div04   \
                                         ro3_clk_div08   \
                                         ro3_clk_div16   \
                                         ro3_clk_div32   \
                                         ro3_clk_div04_sel  \
                                         ro3_clk_div08_sel  \
                                         ro3_clk_div16_sel  \
                                         ro3_clk_div32_sel} \
                               -group  { ro4_clk_in      \
                                         ro4_clk_div02   \
                                         ro4_clk_div04   \
                                         ro4_clk_div08   \
                                         ro4_clk_div16   \
                                         ro4_clk_div32   \
                                         ro4_clk_div04_sel  \
                                         ro4_clk_div08_sel  \
                                         ro4_clk_div16_sel  \
                                         ro4_clk_div32_sel} 

set_clock_groups -physically_exclusive -group {ro1_clk_div04_sel} -group {ro1_clk_div08_sel} -group {ro1_clk_div16_sel} -group {ro1_clk_div32_sel}
set_clock_groups -physically_exclusive -group {ro2_clk_div04_sel} -group {ro2_clk_div08_sel} -group {ro2_clk_div16_sel} -group {ro2_clk_div32_sel}
set_clock_groups -physically_exclusive -group {ro3_clk_div04_sel} -group {ro3_clk_div08_sel} -group {ro3_clk_div16_sel} -group {ro3_clk_div32_sel}
set_clock_groups -physically_exclusive -group {ro4_clk_div04_sel} -group {ro4_clk_div08_sel} -group {ro4_clk_div16_sel} -group {ro4_clk_div32_sel}


set_ideal_network -no_propagate [get_nets [get_pins -of_object ${slow_clock_cell_01} -filter "name == Z"]]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_01_divider_02_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_01_divider_04_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_01_divider_08_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_01_divider_16_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_01_divider_32_registers} -filter "name == Q"]

set_ideal_network -no_propagate [get_nets [get_pins -of_object ${slow_clock_cell_02} -filter "name == Z"]]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_02_divider_02_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_02_divider_04_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_02_divider_08_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_02_divider_16_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_02_divider_32_registers} -filter "name == Q"]

set_ideal_network -no_propagate [get_nets [get_pins -of_object ${slow_clock_cell_03} -filter "name == Z"]]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_03_divider_02_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_03_divider_04_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_03_divider_08_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_03_divider_16_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_03_divider_32_registers} -filter "name == Q"]

set_ideal_network -no_propagate [get_nets [get_pins -of_object ${slow_clock_cell_04} -filter "name == Z"]]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_04_divider_02_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_04_divider_04_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_04_divider_08_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_04_divider_16_registers} -filter "name == Q"]
set_ideal_network -no_propagate [get_pins -of_object ${slow_clock_04_divider_32_registers} -filter "name == Q"]


set_false_path -to [get_ports o_trng_ro_clk]
set_false_path -to [get_ports o_trng_ro_out]

set_max_delay [expr 0.5*$PERIOD] -from [get_pins u_cpu_wrapper/u_wing_m130a_top_wrapper/u_wing_m130a_top/u_m130_ds_top/i_dtm_top/i_tapc_synchronizer/tapcsync2dmi_ch_sel_o_reg/Q] -to [get_pins u_cpu_wrapper/u_wing_m130a_top_wrapper/u_wing_m130a_top/u_m130_ds_top/i_dtm_top/i_tapc/tdo_out_ff_reg/data_in]
set_max_delay [expr 0.5*$PERIOD] -from [get_pins u_cpu_wrapper/u_wing_m130a_top_wrapper/u_wing_m130a_top/u_m130_ds_top/i_dtm_top/i_dmi/tap_dr0_ff_reg/Q]                          -to [get_pins u_cpu_wrapper/u_wing_m130a_top_wrapper/u_wing_m130a_top/u_m130_ds_top/i_dtm_top/i_tapc/tdo_out_ff_reg/data_in]
