#!/usr/bin/env bash
# on Windows, run this script in Git Bash

RSA_SIGN_KEY=rsa2048:00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010001A722EB6392E2FD8FCB98517D3BEEBCED5B3CD45D8BC66E55055DA26FD256F32D5A8F56CE6B8DD90F8F89875085D7147F6E600B9DD0B129955672695165FC4D917DF80441B5B34F4F7140761ECC052D5B49A0CE7D3B6AAF18F6A2AA659E2F2A92F67A849AB3F7E76AEDBA9B68A261DAA080E259A1B84EB50A16ABCCE5436190DFF1E0D9C57089157A2B16414621CDE3CC15428C70F5111E6577098178DE2EA4DA7FA41C9F8970123C788F934226EB6369CD8979DBEB9DBE7DA53EB3AFCFB891E3B78DC2696D0A7755BC6EA31ED891E0196194C5332F0A9EA766A624BC59259B8B5C52F7DA6DE6B6522AFEBD8956C63AEF983696703073D31D6521E1D93422850309BAAFA7293ACB75DE6D2C7934CEEDF2873820489DC55C50D8B21DAD92B9FB07EDD8A03DFDE6D0F6E5ED24925C682BCA4B3B8E118EBF71633F1803E7781F5AC89863A2A3CC96F68BF254C54B7FA6E18009E6317EFAC17815E4ED8D2BC32C8DB6E660356DE8020B579F2518BCDB8F04B2092EA3F7150A87E1617678C3F3CB2F2AB20DB17989B10A70BD5BB8B199FEABD633CE4F5E807BC5292A63F269B2101FC7ADF76450310F76500C336F6A97C1ED278C42DEDF129A65568D48C1896ED49988B92F547603AF6FF70A13010087C8D886F4FF1FFFBEA23D0B039B584CF98CD3A10B9EC8D819A5E410D87A7527616DD0AE9735446380550A335065B618DFBD6E41 
RSA3072_SIGN_KEY=rsa3072:00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010001C18CBE92DF76E3A3B53CAEED8725FBF0376BD689F372D8710CD0593329DE4E5F0647A1113A58BFB0BDB74ACFB51BB050D12908910BB75B5F987DC7E5F332FC4E4B077CF840CD1B642163B42C45517B3326F8FBE562A8DB1EA7AAAED1C7579CAA23383E108C43E1D86C143BBCDEEF071F10AB5B2B32C4BBE2A168046159E49A13803761BD2B49CE975AF790EE45F8F6D9DAED1644882AC851419C0410FA5305668647E85902F28C93981E91AE0D26998240F8C663487C1FBC320AD9A6A389C0C82353C9BA157DE375D249C7678A9076B91ACC026AC48C4074B37D2FA571C206441F46BCF24191A2E29EB92B5E20E90D9A5F4C7B87204E17D7F4525AE60A5D3CFF8AC29A46BB0D80ABBD46FD57D37460809D98E1EDDEE2FBA88CA4E27ADBA875FE74E6B14AF023A55E640D911E234FB38EADF85C5EF36F1EEBDDAE0BE264520F45AA4065393B5801E79B1CE7CFEFCAFDAFA49968CD5E514BA3CF53185B253F898681C009F558B203551815777AB033DC059F54CA52D47771D12DCCF7C7F7DD4A5124B71E2F9E545F3A5ED6F417FADB29BF97C3CD1EBE1FBB0F36EF0EFCB5A48619608795F63EAB3731BFCB5555B1755F2CF580632FA09957ABFAB1618A2D682E946B87301DA7AD40D4EDC08C1D19C92B25B8EEE259D12D56E3A9CAA6FF04618A467C07DF3E245D7C2A21D674BE8F4BA87E2C1B55600C716572C84B8309349438E61A1C675E0B282518711925550DD0FC82B714E2747DA01FFFCA025E8319F945BCFD1C06E75BB7C712F878540A4054622A6348CDD96796A42E7F0FAFD9FC8BD515CD97DAA29425F1358F4330944C09F0608C688A5295D66E5C0DC58306E2D448550C1F7F14BBC426DC525035749235ADEB4A30AA3D5C64D8000B453D772DEBAEAC27762440F145CF3E6FCC03F062E3BCE95163DD150452D03A95D696D05BB158369BB89D92B8444483AE3BD022D0B1262E707C3E73BA6818DB9EB4077CA3E275CB4FB92BF72A5E6121BBCE6D53DE21344E1CD6439D2432698506C30E405FF5F3BB11F89199DFDA7ED55D9A1C03F3C5AE4ECA418639A24C50FC7463BDB612A47841

ECC256_SIGN_KEY=ecc256:370885247EAB66499FD554ADF24B84817452CA1F5A67CD35428DAB1CF40202E0698F8D4A3C4FD068A7F509325FB12D3A147D7F3F0BE9A4AE795FF5AB9A535878415767176EBE1F38B399EC637CED52E8F501DBAD2A981F3BD9222A3CDE8DFEA2

SM2_SIGN_KEY=sm2:0428F01618BAC2FA8D69FC5FC4DD18207476215FA3E3B472E024687A73E25F9C3755293EF9E995C748A42BD2B87591502507D90457EB575BC577405F61758E12F906DB4C16F922728B991C10442E8638E478E2266EC96C1F13A7FCE0F8A80D911D

AES_SIGN_KEY=aes128:32BB3737FB468C4B6AC9D84C1661737D
# AES_SIGN_KEY=aes256:32BB3737FB468C4B6AC9D84C1661737D32BB3737FB468C4B6AC9D84C1661737D

SM4_SIGN_KEY=sm4:32BB3737FB468C4B6AC9D84C1661737D

AES_ENC_KEY=aes128:9FEBA879C596E5022C22C1EA76260EF3
# AES_ENC_KEY=aes256:9FEBA879C596E5022C22C1EA76260EF39FEBA879C596E5022C22C1EA76260EF3
SM4_ENC_KEY=sm4:9FEBA879C596E5022C22C1EA76260EF3

IV=5ED24925293ACB755C50D8B263A2A3CC
VER_COUNTER=FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

# levels: debug, info, warning, error
LOG=info

if [[ -f ./ehsm_image_tool.py ]]; then
    TOOL=./ehsm_image_tool.py
else
    if [[ -z "$OS" ]]; then
        TOOL=./ehsm_image_tool
    else
        TOOL=./ehsm_image_tool.exe
    fi
fi

# ---------- generate secure-boot images --------------- #
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_aes_soc.bin -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_aes_soc.bin -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_aes_soc_ehsmkey.bin -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_aes_ehsm.bin -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_plain_soc.bin -s $RSA_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_plain_soc_ehsmkey.bin -s $RSA_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa2048_plain_ehsm.bin -s $RSA_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_aes_soc.bin -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_aes_soc.bin -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_aes_soc_ehsmkey.bin -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_aes_ehsm.bin -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_plain_soc.bin -s $RSA3072_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_plain_soc_ehsmkey.bin -s $RSA3072_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_rsa3072_plain_ehsm.bin -s $RSA3072_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_aes_soc.bin -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_aes_soc.bin -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_aes_soc_ehsmkey.bin -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_aes_ehsm.bin -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_plain_soc.bin -s $ECC256_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_plain_soc_ehsmkey.bin -s $ECC256_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_ecc256_plain_ehsm.bin -s $ECC256_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_aes_soc.bin -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_aes_soc_ehsmkey.bin -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_aes_ehsm.bin -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_plain_soc.bin -s $AES_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_plain_soc_ehsmkey.bin -s $AES_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_aes_plain_ehsm.bin -s $AES_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_sm4_soc.bin -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_sm4_soc_ehsmkey.bin -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_sm4_ehsm.bin -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_plain_soc.bin -s $SM2_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_plain_soc_ehsmkey.bin -s $SM2_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm2_plain_ehsm.bin -s $SM2_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_sm4_soc.bin -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_sm4_soc_ehsmkey.bin -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_sm4_ehsm.bin -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_plain_soc.bin -s $SM4_SIGN_KEY  -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_plain_soc_ehsmkey.bin -s $SM4_SIGN_KEY  -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_sm4_plain_ehsm.bin -s $SM4_SIGN_KEY -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG boot ehsm_fw.bin -A -o ehsm_fw_boot_image_naked.bin

# ---------- generate secure-upgrade images --------------- #
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa2048_aes_soc.bin -u $RSA_SIGN_KEY -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa2048_aes_soc_ehsmkey.bin -u $RSA_SIGN_KEY -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa2048_aes_ehsm.bin -u $RSA_SIGN_KEY -s $RSA_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa3072_aes_soc.bin -u $RSA3072_SIGN_KEY -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa3072_aes_soc_ehsmkey.bin -u $RSA3072_SIGN_KEY -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_rsa3072_aes_ehsm.bin -u $RSA3072_SIGN_KEY -s $RSA3072_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_ecc256_aes_soc.bin -u $ECC256_SIGN_KEY -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_ecc256_aes_soc_ehsmkey.bin -u $ECC256_SIGN_KEY -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_ecc256_aes_ehsm.bin -u $ECC256_SIGN_KEY -s $ECC256_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_aes_aes_soc.bin -u $AES_SIGN_KEY -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_aes_aes_soc_ehsmkey.bin -u $AES_SIGN_KEY -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_aes_aes_ehsm.bin -u $AES_SIGN_KEY -s $AES_SIGN_KEY -e $AES_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm2_sm4_soc.bin -u $SM2_SIGN_KEY -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm2_sm4_soc_ehsmkey.bin -u $SM2_SIGN_KEY -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm2_sm4_ehsm.bin -u $SM2_SIGN_KEY -s $SM2_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey

$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm4_sm4_soc.bin -u $SM4_SIGN_KEY -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-sockey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm4_sm4_soc_ehsmkey.bin -u $SM4_SIGN_KEY -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t soc-ehsmkey
$TOOL -l $LOG upgrade ehsm_fw.bin -A -o ehsm_fw_upgrade_image_sm4_sm4_ehsm.bin -u $SM4_SIGN_KEY -s $SM4_SIGN_KEY -e $SM4_ENC_KEY -i $IV -c $VER_COUNTER -t ehsm-ehsmkey
