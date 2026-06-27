//========================================================================================
// Copyright (C) 2026 Open Security Research Inc. - All Rights Reserved
// 
// Version: v1.0.0-a
// 
// 4019 4_1_0_dev_Intellifusion 3ab97833df350f277d09663b6fa2bf114220b2a5
// 5990b6d6faa02bb3276acdc07eac001c11d17ec3
//========================================================================================
localparam  [31:0] HASH_CMD_NUM      = 1  ;
localparam  [31:0] SKE_CMD_NUM       = 1  ;
localparam  [31:0] PKE_CMD_NUM       = 1  ;
localparam  [31:0] PKE_SM2_CMD_NUM   = 1  ;
localparam  [31:0] PKE_ECC_CMD_NUM   = 1  ;
localparam  [31:0] RANDOMGEN_CMD_NUM = 1  ;
localparam  [31:0] KEY_GEN_CMD_NUM   = 1  ;
localparam  [31:0] KEY_EXPORT_CMD_NUM   = 1  ;
localparam  [31:0] KEY_IMPORT_CMD_NUM   = 1  ;

localparam  [31:0] WRITE_DATA_CMD_NUM   = 1  ;
localparam  [31:0] READ_DATA_CMD_NUM    = 1  ;
localparam  [31:0] GENMAC_CMD_NUM       = 1  ;
localparam  [31:0] GET_CHALLENGE_CMD_NUM= 1  ;
localparam  [31:0] DEBUG_AUTH_CMD_NUM   = 1  ;
localparam  [31:0] MODULE_STATUS_CMD_NUM = 1 ;
localparam  [31:0] SOC_VERIFY_CMD_NUM   = 1 ;
localparam  [31:0] SOC_UPDATE_CMD_NUM   = 1 ;
localparam  [31:0] SKE_CCM_CMD_NUM      = 1 ; 
localparam  [31:0] SKE_GCM_CMD_NUM      = 1 ; 
localparam  [31:0] PKE_RSA_CMD_NUM      = 1 ;
localparam  [31:0] SM9_WRAPPER_CMD_NUM  = 1 ;
localparam  [31:0] KEY_GENERATE_CMD_NUM = 1 ;
localparam  [31:0] KEY_GEN_PRIV_CMD_NUM = 1 ;
localparam  [31:0] SM9_UNWRAPPER_CMD_NUM= 1 ;
localparam  [31:0] PKE_SM9_CMD_NUM      = 1 ;
localparam  [31:0] KEY_GET_MASTER_CMD_NUM = 1 ;
localparam  [31:0] KEY_GET_PUB_CMD_NUM    = 1 ;
localparam  [31:0] KEY_GET_PRIV_CMD_NUM   = 1 ;
localparam  [31:0] SM9_EXCHG_CMD_NUM      = 1 ; 
localparam  [31:0] SM2_GEN_KEY_CMD_NUM    = 1 ;
localparam  [31:0] MAC_TEST_CMD_NUM       = 1 ;

reg [32:0]  check_cnt     ;
reg         hold          ;
reg [63:0]  clk_cnt       ;
reg [63:0]  hash_start    ;
reg [63:0]  ske_start     ;
reg [63:0]  ske_ccm_start ;
reg [63:0]  ske_gcm_start ;
reg [63:0]  pke_start     ;
reg [63:0]  pke_sm2_start    ;
reg [63:0]  sm2_verify_start ;
reg [63:0]  sm2_sign_start ;
reg [63:0]  module_status_start ;
reg [63:0]  soc_verify_start ;
reg [63:0]  soc_update_start ;
reg [63:0]  rsa_cipher_start ;
reg [63:0]  sm9_wrapper_start;
reg [63:0]  key_gen_priv_start  ;
reg [63:0]  key_generate_start  ;
reg [63:0]  sm9_unwrapper_start ;
reg [63:0]  sm9_cipher_start ;
reg [63:0]  sm9_verify_start ;
reg [63:0]  ecdsa_start      ;
reg [63:0]  pke_ecc_start    ;
reg [63:0]  randomgen_start  ;
reg [63:0]  key_gen_start    ;
reg [63:0]  key_export_start ;
reg [63:0]  key_import_start ;
reg [63:0]  key_import_sm9_start;
reg [63:0]  get_master_pubkey_start;
reg [63:0]  key_get_pub_start;
reg [63:0]  key_gen_usertmp_start ;
reg [63:0]  sm9_exchg_start  ;
reg [63:0]  sm2_gen_key_start ;
reg [63:0]  mac_test_start   ;

reg [63:0]  write_data_start    ;
reg [63:0]  read_data_start     ;
reg [63:0]  genmac_start        ;
reg [63:0]  get_challenge_start ;
reg [63:0]  debug_auth_start    ;
reg [31:0]  ehsm_key_auth_code[$] ;//

real  hash_bw             ;
real  ske_bw              ;
real  ske_ccm_bw          ;
real  ske_gcm_bw          ;
real  pke_bw              ;
real  pke_sm2_bw          ;
real  sm2_verify_bw       ;
real  sm2_sign_bw         ;
real  module_status_bw    ;
real  soc_verify_bw       ;
real  soc_update_bw       ;
real  rsa_cipher_bw       ;
real  sm9_wrapper_bw   ;
real  key_generate_bw  ;
real  key_gen_priv_bw  ;
real  sm9_unwrapper_bw ;
real  sm9_cipher_bw    ;
real  sm9_verify_bw    ;
real  ecdsa_bw          ;
real  pke_ecc_bw          ;
real  randomgen_bw        ;
real  key_gen_bw          ;
real  key_export_bw       ;
real  key_import_bw       ;
real  key_import_sm9_bw   ;
real  get_master_pubkey_bw;
real  key_get_pub_bw      ;
real  key_gen_usertmp_bw  ;
real  sm9_exchg_bw        ;
real  sm2_gen_key_bw      ;
real  mac_test_bw         ;  

real  write_data_bw       ;
real  read_data_bw        ;
real  genmac_bw           ;
real  get_challenge_bw    ;
real  debug_auth_bw       ;

reg             rom_csb     ;
reg [16:0]      rom_a       ;
wire [38:0]     rom_do      ;
wire            rom_clk     ;

reg             ram_csb     ;
reg  [16:0]     ram_a       ;
reg             ram_web     ;
reg  [38:0]     ram_di      ;
wire [38:0]     ram_do      ;
wire            ram_clk     ;

reg             dram_csb    ;
reg             dram_web    ;
reg [16:0]      dram_a      ;
reg [38:0]      dram_di     ;

reg             kram_csb    ;
reg             kram_web    ;
reg [6:0]       kram_a      ;
reg [71:0]      kram_di     ;
wire [71:0]     kram_do     ;
wire            kram_clk    ;

reg             sram_csan   ;
reg             sram_csbn   ;
reg             sram_web    ;

reg [8:0]       sram_a      ;
reg [8:0]       sram_b      ;
reg [71:0]      sram_di     ;
wire [71:0]     sram_do     ;
wire            pke_sram_clk;

typedef enum logic [7:0] {
    CMD_RSP_KEY_OK,
    CMD_RSP_KEY_ERROR,
    CMD_RSP_OK,
    CMD_RSP_ERROR
} cmd_rsp_e;

typedef enum logic[7:0] {
  //init state
  INIT_SRAM0,
  INIT_SRAM1,
  INIT_SRAM2,
  INIT_SRAM3,
  INIT_AND_RST,
  SET_OTP,
  SET_IRAM_CODE,
  WAIT_SEIP_RDY,
  //hash state
  HASH_SET_DATA,
  HASH_SET_CMD,
  HASH_WAIT_DONE,
  HASH_GET_RSP,
  HASH_GET_RESULT,
  //ske state
  SKE_SET_KEY,
  SKE_SET_DATA,
  SKE_SET_IV,
  SKE_SET_CMD,
  SKE_WAIT_DONE,
  SKE_GET_RSP,
  SKE_GET_RESULT,
  //ske ccm state
  SKE_CCM_SET_KEY,
  SKE_CCM_SET_DATA,
  SKE_CCM_SET_IV,
  SKE_CCM_SET_CMD,
  SKE_CCM_WAIT_DONE,
  SKE_CCM_GET_RSP,
  SKE_CCM_GET_RESULT,
  //ske gcm state
  SKE_GCM_SET_KEY,
  SKE_GCM_SET_DATA,
  SKE_GCM_SET_IV,
  SKE_GCM_SET_CMD,
  SKE_GCM_WAIT_DONE,
  SKE_GCM_GET_RSP,
  SKE_GCM_GET_RESULT,
  //key import dbg
  SET_KEY_DATA,
  SET_KEY_CMD,
  WAIT_KEY_DONE,
  GET_KEY_RSP,
  //key export state
  SET_SKE_KEY1,
  SET_SKE_KEY2,
//KEY_IMPORT_SET_CMD,
//KEY_IMPORT_WAIT_DONE,
//KEY_IMPORT_GET_RSP,
//KEY_IMPORT_GET_RESULT,
  //pke state
  PKE_SET_DATA,
  PKE_SET_SIG,
  PKE_SET_E,
  PKE_SET_D,
  PKE_SET_N,
  PKE_SET_CMD,
  PKE_SET_PUB_KEY,
  PKE_SET_KEY,
  PKE_WAIT_DONE,
  PKE_GET_RSP,
  PKE_GET_RESULT,
  //pke sm2 state
  PKE_SM2_SET_DATA,
  PKE_SM2_SET_KEY,
  PKE_SM2_SET_CMD,
  PKE_SM2_WAIT_DONE,
  PKE_SM2_GET_RSP,
  PKE_SM2_GET_RESULT,
  //pke ecc state
  PKE_ECC_SET_DATA,
  PKE_ECC_SET_CMD,
  PKE_ECC_WAIT_DONE,
  PKE_ECC_GET_RSP,
  PKE_ECC_GET_RESULT,
  //pke ecc state
  PKE_RSA_SET_DATA,
  PKE_RSA_SET_CMD,
  PKE_RSA_WAIT_DONE,
  PKE_RSA_GET_RSP,
  PKE_RSA_GET_RESULT,
  //sm2 verify state
    SM2_VERIFY_SET_DATA,
    SM2_VERIFY_SET_KEY,
    SM2_VERIFY_SET_CMD,
    SM2_VERIFY_WAIT_DONE,
    SM2_VERIFY_GET_RSP,
  //sm2 sign state
    SM2_SIGN_SET_DATA,
    SM2_SIGN_SET_KEY,
    SM2_SIGN_SET_CMD,
    SM2_SIGN_WAIT_DONE,
    SM2_SIGN_GET_RSP,
  //module status test
  MODULE_STATUS_SET_CMD,
  MODULE_STATUS_WAIT_DONE,
  MODULE_STATUS_GET_RSP,
  MODULE_STATUS_GET_RESULT,
  //soc verify
  SOC_VERIFY_SET_DATA,
  SOC_VERIFY_SET_CMD,
  SOC_VERIFY_WAIT_DONE,
  SOC_VERIFY_GET_RSP,
  //soc update
  SOC_UPDATE_SET_DATA,
  SOC_UPDATE_SET_CMD,
  SOC_UPDATE_WAIT_DONE,
  SOC_UPDATE_GET_RSP,
  //pke sm9 wrapper state
  SM9_WRAPPER_GEN_MASTER_KEY,
  SM9_WRAPPER_GEN_PRIV_KEY,
  SM9_WRAPPER_SET_CMD,
  SM9_WRAPPER_WAIT_DONE,
  SM9_WRAPPER_GET_RSP,
  SM9_WRAPPER_GET_RESULT,
  //pke sm9 wrapper state
  SM9_UNWRAPPER_SET_DATA,
  SM9_UNWRAPPER_SET_CMD,
  SM9_UNWRAPPER_WAIT_DONE,
  SM9_UNWRAPPER_GET_RSP,
  SM9_UNWRAPPER_GET_RESULT,
  //pke ecc state
  PKE_SM9_SET_DATA,
  PKE_SM9_SET_CMD,
  PKE_SM9_WAIT_DONE,
  PKE_SM9_GET_RSP,
  PKE_SM9_GET_RESULT,
  //pke sm9 exchg state
  SM9_EXCHG_GEN_MASTER_KEY,
  SM9_EXCHG_GET_MASTER_PUBKEY,
  SM9_EXCHG_GEN_USERPRIV_KEY,
  SM9_EXCHG_GEN_USERTMP_KEYA,
  SM9_EXCHG_GEN_USERTMP_KEYB,
  SM9_EXCHG_GET_TEMP_PUBKEY,
  SM9_EXCHG_SET_CMD,
  SM9_EXCHG_WAIT_DONE,
  SM9_EXCHG_GET_RSP,
  SM9_EXCHG_GET_RESULT,
  //sm2 gen key state
  SM2_GEN_KEY_SET_DATA,
  SM2_GEN_KEY_SET_CMD,
  SM2_GEN_KEY_WAIT_DONE,
  SM2_GEN_KEY_GET_RSP,
  SM2_GEN_KEY_GET_RESULT,
  //mac test
   MAC_TEST_SET_DATA,
   MAC_TEST_SET_MAC,
   MAC_TEST_SET_KEY,
   MAC_TEST_SET_CMD,
   MAC_TEST_WAIT_DONE,
   MAC_TEST_GET_RSP,

  //ramdomize_test
  RANDOMGEN_GEN_SET_CMD,
  RANDOMGEN_GEN_WAIT_DONE,
  RANDOMGEN_GEN_GET_RSP,
  RANDOMGEN_GEN_GET_RESULT,
  //key gen state
  KEY_GEN_SET_CMD,
  KEY_GEN_WAIT_DONE,
  KEY_GEN_GET_RSP,
  KEY_GEN_GET_RESULT,
  //key export state
  KEY_EXPORT_SET_CMD,
  KEY_EXPORT_WAIT_DONE,
  KEY_EXPORT_GET_RSP,
  KEY_EXPORT_GET_RESULT,
  //key import state
  KEY_IMPORT_SET_CMD,
  KEY_IMPORT_WAIT_DONE,
  KEY_IMPORT_GET_RSP,
  KEY_IMPORT_GET_RESULT,
  SET_NVM_CODE,
  //boot state
  IMAGE_INSTALL_CMD_START,
  IMAGE_INSTALL_CMD_UPDATE,
  IMAGE_INSTALL_CMD_FINISH,
  IMAGE_INSTALL_WAIT_DONE,
  IMAGE_INSTALL_RSP,

  IMAGE_INSTALL_VERIFY_CMD_START,
  IMAGE_INSTALL_VERIFY_CMD_UPDATE,
  IMAGE_INSTALL_VERIFY_CMD_FINISH,
  IMAGE_INSTALL_VERIFY_WAIT_DONE,
  IMAGE_INSTALL_VERIFY_RSP

  ,
  //sensor alarm state
  SENSOR_DRV,
  SENSOR_LATCH,
  SENSOR_CLEAR,
  //write read data
  WRITE_DATA_SET_DATA,
  WRITE_DATA_SET_CMD,
  WRITE_DATA_WAIT_DONE,
  WRITE_DATA_GET_RSP,
  WRITE_DATA_GET_RESULT,
  READ_DATA_SET_CMD,
  READ_DATA_WAIT_DONE,
  READ_DATA_GET_RSP,
  READ_DATA_GET_RESULT,
  //genmac state
  GENMAC_SET_DATA,
  GENMAC_SET_KEY,
  GENMAC_SET_CMD,
  GENMAC_WAIT_DONE,
  GENMAC_GET_RSP,
  GENMAC_GET_RESULT,
  //DFU2
  SET_DFU2_CODE,
  RUN_DFU2_CODE,
  //DEBUG AUTH
  GET_CHALLENGE_SET_CMD,
  GET_CHALLENGE_WAIT_DONE,
  GET_CHALLENGE_GET_RSP,
  GET_CHALLENGE_GET_RESULT,
  DEBUG_AUTH_SET_PUPKEY,
  DEBUG_AUTH_SET_SIGN,
  DEBUG_AUTH_SET_CMD,
  DEBUG_AUTH_WAIT_DONE,
  DEBUG_AUTH_GET_RSP,

 //JTAG TEST
   JTAG_TEST_BEGIN,
   JTAG_TEST_DONE

} cmd_state_e;

typedef enum logic[31:0] {
    // MISC commands
    GET_DEVICEINFO = 'h02,
    RANDOM_GENERATE = 'h03,
    KEY_GENERATE = 'h04,
    KEY_FAILUREANALYSIS,
    KEY_SELFDESTROY,
    KEY_EXPORT,
    KEY_IMPORT,
    KEY_ERASE,
    PUBKEYHASHVERIFY,
    KEY_TRANSPORT,
    SET_RANDOM_CLK,
    SEIP_JUMP = 'h0D,
    SEIP_DISABLE = 'h0F,
    KEY_CRC   = 'hB1,

    // MAC commands
    GENERATE_MAC = 'h11,
    VERIFY_MAC,

    // SKE commands
    SKE_ENCRYPT = 'h21,
    SKE_DECRYPT,
    AEADENCRYPT_GCM,
    AEADDECRYPT_GCM,
    AEADENCRYPT_CCM,
    AEADDECRYPT_CCM,

    // SM2 commands
    SM2_GETZ = 'h41,
    SM2_GETE,
    SM2_GENERATEKEY,
    SM2_GENERATESIGNATUREWITHE,
    SM2_VERIFYSIGNATUREWITHE,
    SM2_ENCRYPT,
    SM2_DECRYPT,
    SM2_EXCHANGEKEYWITHZ,
    SM2_GETPUBKEYFROMPRIKEY,

    // ECCP commands
    ECCP_POINTDOUBLING = 'h71,
    ECCP_POINTADDITION,
    ECCP_POINTMULTIPLICATION,
    ECCP_POINTVERIFYING,
    ECCP_GENERATEKEY,
    ECDH_EXCHANGEKEY,
    ECDSA_GENERATESIGNATURE,
    ECDSA_VERIFYSIGNATURE,

    // RSA commands
    RSA_GENERATEPRIME = 'h81,
    RSA_GENERATEKEY,
    RSA_GENERATECRTKEY,
    RSA_ENCRYPT,
    RSA_DECRYPT,
    RSA_CRT_DECRYPT,
    RSA_GENERATESIGNATURE,
    RSA_CRT_GENERATESIGNATURE,
    RSA_VERIFYSIGNATURE,

    // HASH commands
    HASH = 'hA1
    // WRITE READ DATA commands
    ,WR_DATA = 'hC4
    //DEBUG AUTH
    ,GET_CHALLENGE = 32'h00FCFF03
    ,DEBUG_AUTH = 32'h00FBFF04

    //EHSM NEW CMD ID
    ,KEY_IMPORT_EHSM_DEBUG = 32'hFFFF0000
    ,KEY_IMPORT_EHSM = 32'hF7FE0801
    ,HASH_EHSM ='hFCFE0301
    ,SKE_EHSM ='hFEFE0101
    ,SM2_EHSM ='hFBFE0401
    ,RSA_EHSM ='hFAFD0502
    ,RANDOMGEN_TEST_EHSM ='hF5FE0A01
    //,GET_CHALLENGE_EHSM ='h00FCFF03,
    //,DEBUG_AUTH_EHSM ='h00FBFF04

} cmd_id_e;

typedef struct {
    cmd_id_e   cmd_id;
    cmd_rsp_e  cmd_rsp;
    string     cmd_name;    
    reg        flag_s;
    reg [31:0] result_s[$];
    reg [31:0] rm_result_s[$];
}s_result;

s_result s_result_x;
s_result s_result_q[$];

cmd_state_e  cmd_state;

reg [38:0]      ram_rdata[15:0];
reg [38:0]      ram_wdata[15:0];
reg [71:0]      pke_ram_rdata[15:0];
reg [71:0]      pke_ram_wdata[15:0];

reg [71:0]      kmu_ram_rdata[1:0];
reg [71:0]      kmu_ram_wdata[1:0];
