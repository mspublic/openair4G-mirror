

## Technology Settings
setup_design -manufacturer Xilinx -family VIRTEX-II -part 2V2000fg676 -speed 6


## Input File Settings
set_input_dir /homes/khalfall/work/IDROMEL/grlib-eval/designs/leon3-eurecom-platform-2
add_input_file -format {Xilinx UCF File} -work work ../../boards/eurecom-platform-2/leon3mp.ucf


## Output File Settings


## Design Settings
setup_design -addio
setup_design -vhdl
setup_design -verilog=false
setup_design -edif
setup_design -vendor_constraint_file
setup_design -transformations
setup_design -retiming=false
setup_design -advanced_fsm_optimization=false
setup_design -compile_for_area=true
setup_design -use_safe_fsm=false
setup_design -encoding=\"auto\"
setup_design -resource_sharing
setup_design -array_bounds_check=false
setup_design -transform_tristates=\"auto\"
setup_design -frequency=\"52\"
setup_design -radhardmethod=\"\"
setup_design -input_delay=\"9\"
setup_design -output_delay=\"9\"
setup_design -partition_size=\"30000\"
setup_design -global_clock_limit=\"\"
setup_design -search_path=\"\"
setup_design -altera_mangle_prefix=\"_MGC\"
setup_design -frontend_2004=false
setup_design -verilog_2001
setup_design -generics=\"\"
setup_design -defines=\"\"
setup_design -automap_work=false
setup_design -error_design_contention=false
setup_design -ignore_ram_rw_collision=false
setup_design -translate_ucf_constraints
MGS_Optimize::set_path_compression_options -run_during_physical_synthesis=false

## Analysis Settings
setup_analysis -clock_frequency
setup_analysis -summary
setup_analysis -num_summary_paths=10
setup_analysis -critical_paths
setup_analysis -num_critical_paths=1
setup_analysis -timing_violations
setup_analysis -net_fanout
setup_analysis -clock_domain_crossing=false
setup_analysis -missing_constraints
