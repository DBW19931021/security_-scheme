//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
task ehsm_hmac_sm3_test();
reg [31:0] key_import_return_handle;

general_task(64'h6000_0000,"../tc/case/hex/import_hmac_key.hex","import_hmac_key",key_import_return_handle);

general_task(64'h6000_0000,"../tc/case/hex/hmac_sm3.hex","hmac_sm3",key_import_return_handle);

endtask
