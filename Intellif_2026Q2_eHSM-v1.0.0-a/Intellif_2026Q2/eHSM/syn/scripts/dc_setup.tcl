#lib path set
set search_path [concat  [list \
	../. \
    .././dbs/ \
    ../../rtl \
    ../../rtl/define \
    ../../rtl/ahb_top/ahb_trng \
    ../../rtl/ahb_top/ahb_hash/src/hfe \
    ../../rtl/ahb_top/ahb_ske/src/ske_hp \
    ../../rtl/ahb_top/ahb_pke/src \
] $search_path]

set synthetic_library [concat [list \
	standard.sldb \
	dw_foundation.sldb \
]]

set link_library [list "*" \
    u055lscee12bbl_108c125_wc.db \
]
set target_library [list \
    u055lscee12bbl_108c125_wc.db \
]

#Site Specific Variables
set command_log_file "/command.log"
set view_command_log_file "/view_command.log"
set bus_naming_style {%s[%d]}

#app_var
set verilogout_no_tri "true"
set hdlin_enable_vpp "true"
set write_name_nets_same_as_ports "true"
set auto_wire_load_selection "false"
set hdlin_presto_net_name_prefix "n"
set timing_enable_multiple_clocks_per_reg "true"
set enable_page_mode "false"
set mv_default_level_shifter_voltage_range_infinity "true"
set mv_insert_level_shifter_verbose "true" 
set mv_insert_level_shifters_on_ideal_nets "true"
set auto_wire_load_selection "true"
set hdlin_enable_rtldrc_info "true"
set hdlin_keep_signal_name "user"
set enable_keep_signal "true"

#path define
set scr_path     	./scripts/
set netlist_path 	./netlist/
set report_path  	./report/
set sdf_path     	./sdf/
set ddc_path     	./ddc/

#project setting
set top $TOPCELL
define_design_lib WORK -path ./work

