//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
task ehsm_pke_rsa_test();
reg [31:0] key_import_return_handle;
//key_import
general_task(64'h6000_0000,"../tc/case/hex/import_rsa_key.hex","import_rsa_key",key_import_return_handle);

general_task(64'h6000_0000,"../tc/case/hex/rsa_cipher.hex","rsa_cipher",key_import_return_handle);
endtask
