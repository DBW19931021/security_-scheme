//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
task ehsm_ecdsa_verify_test();
reg [31:0] key_import_return_handle;
//key_import
general_task(64'h6000_0000,"../tc/case/hex/import_ecdsa_key.hex","import_ecdsa_key",key_import_return_handle);
//ske_calc
general_task(64'h6000_0000,"../tc/case/hex/ecdsa_vry.hex","ecdsa_vry",key_import_return_handle);

endtask
