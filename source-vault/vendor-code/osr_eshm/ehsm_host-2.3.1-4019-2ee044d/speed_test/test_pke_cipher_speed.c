#include "speed_test.h"
#include "api.h"
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

// RSA key definitions from test_pke_cipher_speed.py (hexadecimal byte arrays)
typedef struct {
    const char *key_type;
    const uint8_t *e_data; // Public exponent
    const uint8_t *n_data; // Modulus
    const uint8_t *d_data; // Private key data (d or CRT parameters)
    size_t e_size;         // Public exponent size
    size_t n_size;         // Modulus size
    size_t d_size;         // Private key size
} rsa_key_data_t;

// Convert hex string to bytes
static void hex_to_bytes(const char *hex_str, uint8_t *bytes)
{
    size_t len = strlen(hex_str);
    for (size_t i = 0; i < len; i += 2) {
        unsigned int temp;
        sscanf(&hex_str[i], "%2x", &temp);
        bytes[i / 2] = (uint8_t)temp;
    }
}

// RSA key data from Python test file
static const char *rsa_1024_e = "0000000000010001";
static const char *rsa_1024_n = "F379CAF263C3D649CED9C44D0D226C22FD3ADCFD5A17E77B6D8B889F76E0B9A8E8CABA84BD928E70BD1284"
                                "C7514C5BAE74C1330A6637F9555945A43219AD8E170296DA2ECBC170C2531A4FC39A794C78EF729F5B27CE"
                                "7E6324BE05CEF21FBCB962FFBF0F5CA72C4DA2908FE0D5C7E9FD36EB3B8F08931757B72B1F7405570307";
static const char *rsa_1024_d = "966B9357870D51918DEAFA1D045554EB94F94DD87455BA5C860423A7294193526867FF2DCD15034D617254"
                                "77F3A1322F28D0C7A0D0201AB9810AC673716BC910F7BA1FD08452A3B6591A308CFC6C5B39AE2FCEC35B73"
                                "9F9166D1F3CF2A65B540A554A8F7055E6E13AF3D6EBA6DFC57083188136D763190AF3E5E7ED8AC451429";

static const char *rsa_2048_e = "0000000000010001";
static const char *rsa_2048_n
    = "C523E26F9DC2C2A179CE895AA43A268FE331D461D5D9E11ABD1B4C7CA88A76A7603793549F226CE6A0618622619D7E9D8C08D9797EC6AB7E"
      "C8FA14517E764788F9E2945880A966D419DD2CCEA9B345667D40C2DFD108A3E36E47A46FFD4E58F78EF54E35FF98CCA3AA82A048BA7A31F8"
      "5B66FCDA91BF970908162F7CF30E6D6C5A7F6F217665FFA10939D8FA3C8F268E0AF49F1B2DB490C0D33463A1F9E32C7498814B5C83BF3E45"
      "0579171F2C7D2819D4D965FA34F0515AAFDD3815587FBFBA0279C9321F4BE96D217F08495E02EDC62C692F5F83572CC643E1BDE36F6F573B"
      "DF06588EC2A78DF954B491BA6EFE5E1304A7731F4990E8A0F059D1A95ABE97A7";
static const char *rsa_2048_d
    = "9F70A2563B75A50C0C03AABB104FA7987FDB71359F4B93EF983A57E60A39D23514705E4BF6585553412B1EDCF7ADF5155E8859BB15798082"
      "0DF1943544DE4C84CFF51C9D67919A84B8565542DFA509E016352666F61CD2CFC205574CEBC54DD70A5C036150D337CEB6BCE6FD6B3420EE"
      "089E86AA0F6EB0ACB149A026A23D6CE8E3755F256D4022DF2F2D507172F1A8DB74EFD85AB972A6ED92CA1855DB855F38619C1442967F8DBF"
      "7C6D9DD4B8021AA7702E972233A59D819452F5D76AC8F988C896C43B030E627EE2D7B77927F84C06B711BDE381D5937B25FD244D66BDA4AD"
      "58ACF63A7B3E44ABAE7F5C4645567B6AA95C5B0C0AD3B8452199C49EDF430F61";

static const char *rsa_3072_e = "0000000000010001";
static const char *rsa_3072_n
    = "D0964C6866877C5792BE434810F7249B50C224197931005B983EB2862002523C3EA77A4BF50AB537113ED2C573581DDF61CE5DD5A5553AA9"
      "ABD3D4415A79CE215A5BC488EF876680DC899379965FE6FB33932DF16280C7BA30D60181B80C80C1424301F4A951E2FCAB079736D4828AFF"
      "22F7453D778611A8F669FE62CC93FBC6BB233905F2DBE602A37C95C90064AC497DFE1546888C51E92DE6D2DC8A06E6B4B97A9EDE277A22FC"
      "6EC01A4E9B40CE5E5A15CD33AACE17633B4AC124AEC0302DD2DD7078B71033A1DB13ED034EE525DD6E0BC03C804BDA2229CFA25379E25B5D"
      "D279A83616E1A9D45BC7A08194837850D6BAB6CD522AC9F94665F676240A51CC03A7AADDD24FC0686351EE6CAEE90DF1C7E154B722829760"
      "44E83C0EDB252991B977D32CB0F70DAFF73C228D6E548721DDE758EB07DFC4D8EC918AF51A8DB021D09F5E6C2A6689511A9F93DCBA998979"
      "0E4B38E117BAF1CD4F9722A3CA783D6B62B5C9764DCA0B60388DBC2EC5FCC3461170FD753FFD3B34009285774EB58A47";
static const char *rsa_3072_d
    = "01CC59EA488000856CC171290CE4648F6639D63FD5F8A32D26F9DB66269E7E810075472BA2782E29C45F4B47E27FAEFA3F089D6C7B196A01"
      "0E772C0C483066A388F8E38FDEA8C72C733A486832F0AEB819B0BA23AB9ABA9E134FB5019E49954E9538A8AAA84F096AEE3A8986327CD19F"
      "F37838439865327FC76810865F8723138A877A355FEB9F67046BCD8AC713171D2715EAAF09FC53B473EDDA81863B1B0011DD49C35689CA05"
      "6D1B00DF0E9D9769DED2F34AEFBD091C94E54716FEDEAF4BA1DAA21F85C7FBED56334D2A48B33F8DE9E8FC9697D0DFBD032ECC3F39309BA0"
      "56180458DDBB0D25D8BA738DEA492277EDA4B1DEFDE3352780C1FCC99FA8F9AC86A155B8FC1ED5C87C8914BB55833FDD40AB2AA35A35D5EA"
      "B6AC082DD89C54A40876429488E3B1D585F64615ECA1DF50C7BD80067882D690CC4CAA3DD4F2E0C302109EA0C528317730782698B7B3B6AB"
      "8B2A89816327256D9294E2B339C2DFA05332ECC9C5711F024AFBD20EA2E8F2B7A90DD1AD796063DAD3A369F6A3CB4FC9";

static const char *rsa_4096_e = "0000000000010001";
static const char *rsa_4096_n
    = "CE417623E8338711A96D957E12F607553AF82AD1A1D49978FCB03918DB865A84CFA946CAFF31C8EECE33390CD7C98756BFB4415C92DBD3CF"
      "C84F9A0E83C81EA9E2D13525E1F969644D72E604667A75AD5685415FF0183B4C446F5700C0F88351D26A9328168BF384A448BC76F180DB36"
      "6185DD8A2D135550FEAED10598D2FA787602F970390194645AD795C573CBE247852A37B67DB68D34BF06A9551F27A338817154250F29F883"
      "1C69C51F4DE58089C3858E16CE2100CFB5B5C773C7FABE30F07F37AA367CF69C9195A06D8A61A9CE85AFCBC924B1DB6609D9821B7184DC0B"
      "91438298A102E9F6A5A0C701649F6F25CC694E33867168C9C5DE70D19A5C08E3CEC53045963EAB9820C2D3B8F146A5817D21FA80473454D3"
      "4DAFF320846ADA693E742062FCE0E01A3FAA1884A956D10B8C72E65B97478D512209AA2B7C68B58CE204921C73E9BDC833AC3ADA65421B06"
      "C24A97DE2A1DFCFED26A5748193E71E6708E6F379B8712510E6A90B26B4A6EDD0FBB192647BC08410EC7B854B92A0BFBAD64E430CF4014FB"
      "F24E24B49FBF37F137A425DE6B7FEEBBC2DDA481E1CECDE90D1114D8E1E85E2F78DEF480B8B0B8A56DD435D6E2BB812F1D631BFF240C45AD"
      "23A4E63FFB8ECB11449174F738382113D07D71850A03BDC908DD593D2F3EFC6FD8B4764F26ED33D7529DA0108F28519141F19E431DF1AA6A"
      "E3A558B4C8E5D61F";
static const char *rsa_4096_d
    = "31E792DF3342AC6EB090EC37D9FC9F5F96EA0EEA33FECDB781645E1C4E995E737E0F562AADBE6D00A2F1AFDF14A31554FF036D4129E37887"
      "70CF19D6633A5B78FD81631BAC667D82A05EA99AECF4BA5E5B6DD8988EEE3E02C1183373E23CFF0120295B3BBAE0D7E6031DFA43C9414549"
      "0E25A6A9D528355F689001D119DE0A6E751759253715D3C2C85C1B47F031A7A96866039EEB21ED5E92CDD8D0D8BDED773192AB2A6055E664"
      "4ACEA8BC5F4BA5632FE4B3B17EE46B5ED6778A2FD07631E88378348EC1AA50BD27588DBD935AF80C193AA4D32146CB700FF809CBFC168A6B"
      "5E157896D7766E0798A1A3250A24AD6EF6DB8FE955A1411A78FECE4B58CEE5F3515BC11CBB9288DBED3FD0DF28A79CCDE2EC85E17CA15902"
      "BC4FCD6CB50AF80CBFF94CFE2321CBDFA51C261C926177A349B8F9D642F2355556C0E9C807B6ABFF19D76F110C5E1F87A68650AFA9CD52B5"
      "A2B62866DF81047103496E3DD9E633D7AAD0734D1E3E3019A08D485B6BE51105B1898E28CAE73EB5B0E5CAF05D6B9CB68EEAD63DAC37498B"
      "E013FCF8A01B1CE3459AF58C92596AFDFFAD71CE27DF2A9D17A9C85BCDF43B279A63F5D9261D33B2E47D97CCAC7602509F8C16F1AB3887DF"
      "CD84A4AB6F2896BFF295222ACEDA3FC900F9981801EE75F07E6FA917A9D1A9731EA8E22944F9E056F1400399008B6F23A245751334F5F129"
      "9C9BA04382D8C7F1";

static uint32_t test_rsa_cipher_single(speed_test_ctx_t *ctx, const char *key_type, const char *e_hex,
    const char *n_hex, const char *d_hex, size_t key_size)
{
    if (!ctx || !key_type || !e_hex || !n_hex || !d_hex)
        return 1;

    ehsm_port_printf("Testing RSA cipher: %s, key size: %u bits\n", key_type, (unsigned int)key_size);

    // Convert hex strings to binary data
    size_t e_len = strlen(e_hex) / 2;
    size_t n_len = strlen(n_hex) / 2;
    size_t d_len = strlen(d_hex) / 2;

    uint8_t *e_data = malloc(e_len);
    uint8_t *n_data = malloc(n_len);
    uint8_t *d_data = malloc(d_len);

    if (!e_data || !n_data || !d_data) {
        free(e_data);
        free(n_data);
        free(d_data);
        return 1;
    }

    hex_to_bytes(e_hex, e_data);
    hex_to_bytes(n_hex, n_data);
    hex_to_bytes(d_hex, d_data);

    // Get key type ID
    uint32_t key_type_id = get_algo_id_by_name(key_types, key_type);
    if (key_type_id == UNKNOWN_ALG_ID) {
        ehsm_port_printf("Unknown RSA key type: %s\n", key_type);
        free(e_data);
        free(n_data);
        free(d_data);
        return 1;
    }

    // Prepare key data structure for import (use same format as working test_pke_speed.c)
    uint8_t *key_buffer = (uint8_t *)ehsm_port_raddr_to_addr(ctx->data1_addr);

    // Pack header: magic(4) + key_type(1) + attributes(1) + reserved(2) + e_len(2) + sk_len(2) + reserved(4)
    uint32_t magic = 0x100F;
    uint16_t e_len_u16 = (uint16_t)e_len;
    uint16_t sk_len_u16 = (uint16_t)d_len;

    size_t offset = 0;
    memcpy(key_buffer + offset, &magic, 4);
    offset += 4;
    key_buffer[offset++] = (uint8_t)key_type_id;
    key_buffer[offset++] = 0x03 | EHSM_KEY_PRIV_IMPORT_PLAIN; // attributes
    offset += 2;                                              // reserved field - skip 2 bytes
    memcpy(key_buffer + offset, &e_len_u16, 2);
    offset += 2; // e_len
    memcpy(key_buffer + offset, &sk_len_u16, 2);
    offset += 2; // sk_len

    // Copy key components: e + n + d
    memcpy(key_buffer + offset, e_data, e_len);
    offset += e_len;
    memcpy(key_buffer + offset, n_data, n_len);
    offset += n_len;
    memcpy(key_buffer + offset, d_data, d_len);
    offset += d_len;

    size_t total_key_size = offset;

    ehsm_port_printf("Importing RSA key %s: e_len=%zu, n_len=%zu, d_len=%zu, total_size=%zu\n", key_type, e_len, n_len,
        d_len, total_key_size);

    // Import RSA key
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_import_key(
        ctx->ehsm_ctx, 0xFFFFFFFF, 0xFFFFFFFF, (ehsm_key_format_st *)key_buffer, total_key_size, 0, 0, &key_handle);

    if (ret != EHSM_OK) {
        ehsm_port_printf("RSA key import failed for %s: 0x%08x\n", key_type, ret);
        free(e_data);
        free(n_data);
        free(d_data);
        return 1;
    }

    double enc_total_time = 0.0;
    double dec_total_time = 0.0;
    uint32_t success_count = 0;
    uint32_t plaintext_size = 32; // Test with 32-byte plaintext

    for (uint32_t loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        // Test encryption
        double start = ehsm_port_get_time_ms();
        uint32_t ciphertext_size = 1024; // Buffer size
        ret = ehsm_rsa_cipher(ctx->ehsm_ctx, key_handle, true, ehsm_port_raddr_to_addr(ctx->data1_addr), plaintext_size,
            ehsm_port_raddr_to_addr(ctx->data2_addr), &ciphertext_size);
        double end = ehsm_port_get_time_ms();

        if (ret == EHSM_OK) {
            double loop_time = end - start;
            enc_total_time += (loop_time > 0) ? loop_time : 0.1;

            // Test decryption
            start = ehsm_port_get_time_ms();
            uint32_t decrypted_size = 1024;
            ret = ehsm_rsa_cipher(ctx->ehsm_ctx, key_handle, false, ehsm_port_raddr_to_addr(ctx->data2_addr),
                ciphertext_size, ehsm_port_raddr_to_addr(ctx->data1_addr), &decrypted_size);
            end = ehsm_port_get_time_ms();

            if (ret == EHSM_OK) {
                loop_time = end - start;
                dec_total_time += (loop_time > 0) ? loop_time : 0.1;
                success_count++;
            }
        }
    }

    free(e_data);
    free(n_data);
    free(d_data);

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for %s\n", key_type);
        return 1;
    }

    // Calculate and log encryption results
    double avg_enc_time = enc_total_time / success_count;
    double enc_speed_5m = calculate_speed_tps(avg_enc_time);
    double enc_speed_1g = enc_speed_5m * 200.0;

    speed_test_result_t enc_result = { 0 };
    snprintf(enc_result.algo_name, sizeof(enc_result.algo_name), "%s", key_type);
    snprintf(enc_result.mode_name, sizeof(enc_result.mode_name), "encrypt");
    enc_result.data_size = plaintext_size;
    enc_result.is_encrypt = true;
    enc_result.loop_count = success_count;
    enc_result.time_ms = avg_enc_time;
    enc_result.speed_5m_mbps = enc_speed_5m;
    enc_result.speed_1g_mbps = enc_speed_1g;

    speed_test_log_result(ctx, &enc_result);

    // Calculate and log decryption results
    double avg_dec_time = dec_total_time / success_count;
    double dec_speed_5m = calculate_speed_tps(avg_dec_time);
    double dec_speed_1g = dec_speed_5m * 200.0;

    speed_test_result_t dec_result = { 0 };
    snprintf(dec_result.algo_name, sizeof(dec_result.algo_name), "%s", key_type);
    snprintf(dec_result.mode_name, sizeof(dec_result.mode_name), "decrypt");
    dec_result.data_size = plaintext_size;
    dec_result.is_encrypt = false;
    dec_result.loop_count = success_count;
    dec_result.time_ms = avg_dec_time;
    dec_result.speed_5m_mbps = dec_speed_5m;
    dec_result.speed_1g_mbps = dec_speed_1g;

    return speed_test_log_result(ctx, &dec_result);
}

static uint32_t test_sm2_cipher_single(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("Testing SM2 cipher\n");

    // Generate SM2 key
    uint32_t key_handle = 0x00100001;
    uint32_t ret = ehsm_km_gen_key(
        ctx->ehsm_ctx, EHSM_KEY_TYPE_SM2, 0x100F | EHSM_KEY_PRIV_IMPORT_PLAIN, 0, 0, 0, 0, &key_handle);

    if (ret != EHSM_OK) {
        ehsm_port_printf("SM2 key generation failed: 0x%08x\n", ret);
        return 1;
    }

    double enc_total_time = 0.0;
    double dec_total_time = 0.0;
    uint32_t success_count = 0;
    uint32_t plaintext_size = 32; // Test with 32-byte plaintext

    for (uint32_t loop = 0; loop < DEFAULT_TEST_LOOPS; loop++) {
        // Test encryption
        double start = ehsm_port_get_time_ms();
        uint32_t ciphertext_size = 1024; // Buffer size
        ret = ehsm_sm2_cipher(ctx->ehsm_ctx, key_handle, true, ehsm_port_raddr_to_addr(ctx->data1_addr), plaintext_size,
            ehsm_port_raddr_to_addr(ctx->data2_addr), &ciphertext_size);
        double end = ehsm_port_get_time_ms();

        if (ret == EHSM_OK) {
            double loop_time = end - start;
            enc_total_time += (loop_time > 0) ? loop_time : 0.1;

            // Test decryption
            start = ehsm_port_get_time_ms();
            uint32_t decrypted_size = 1024;
            ret = ehsm_sm2_cipher(ctx->ehsm_ctx, key_handle, false, ehsm_port_raddr_to_addr(ctx->data2_addr),
                ciphertext_size, ehsm_port_raddr_to_addr(ctx->data1_addr), &decrypted_size);
            end = ehsm_port_get_time_ms();

            if (ret == EHSM_OK) {
                loop_time = end - start;
                dec_total_time += (loop_time > 0) ? loop_time : 0.1;
                success_count++;
            }
        }
    }

    if (success_count == 0) {
        ehsm_port_printf("All test loops failed for SM2\n");
        return 1;
    }

    // Calculate and log encryption results
    double avg_enc_time = enc_total_time / success_count;
    double enc_speed_5m = calculate_speed_tps(avg_enc_time);
    double enc_speed_1g = enc_speed_5m * 200.0;

    speed_test_result_t enc_result = { 0 };
    snprintf(enc_result.algo_name, sizeof(enc_result.algo_name), "SM2");
    snprintf(enc_result.mode_name, sizeof(enc_result.mode_name), "encrypt");
    enc_result.data_size = plaintext_size;
    enc_result.is_encrypt = true;
    enc_result.loop_count = success_count;
    enc_result.time_ms = avg_enc_time;
    enc_result.speed_5m_mbps = enc_speed_5m;
    enc_result.speed_1g_mbps = enc_speed_1g;

    speed_test_log_result(ctx, &enc_result);

    // Calculate and log decryption results
    double avg_dec_time = dec_total_time / success_count;
    double dec_speed_5m = calculate_speed_tps(avg_dec_time);
    double dec_speed_1g = dec_speed_5m * 200.0;

    speed_test_result_t dec_result = { 0 };
    snprintf(dec_result.algo_name, sizeof(dec_result.algo_name), "SM2");
    snprintf(dec_result.mode_name, sizeof(dec_result.mode_name), "decrypt");
    dec_result.data_size = plaintext_size;
    dec_result.is_encrypt = false;
    dec_result.loop_count = success_count;
    dec_result.time_ms = avg_dec_time;
    dec_result.speed_5m_mbps = dec_speed_5m;
    dec_result.speed_1g_mbps = dec_speed_1g;

    return speed_test_log_result(ctx, &dec_result);
}

uint32_t test_pke_cipher_speed(speed_test_ctx_t *ctx)
{
    if (!ctx)
        return 1;

    ehsm_port_printf("\n=== Starting PKE Cipher Speed Tests ===\n");

    int total_tests = 0;
    int failed_tests = 0;

    // Test RSA cipher algorithms
    struct {
        const char *key_type;
        const char *e_hex;
        const char *n_hex;
        const char *d_hex;
        size_t key_size;
    } rsa_tests[] = { { "RSA_1024", rsa_1024_e, rsa_1024_n, rsa_1024_d, 1024 },
        { "RSA_2048", rsa_2048_e, rsa_2048_n, rsa_2048_d, 2048 },
        { "RSA_3072", rsa_3072_e, rsa_3072_n, rsa_3072_d, 3072 },
        { "RSA_4096", rsa_4096_e, rsa_4096_n, rsa_4096_d, 4096 } };

    for (size_t i = 0; i < sizeof(rsa_tests) / sizeof(rsa_tests[0]); i++) {
        total_tests++;
        uint32_t ret = test_rsa_cipher_single(ctx, rsa_tests[i].key_type, rsa_tests[i].e_hex, rsa_tests[i].n_hex,
            rsa_tests[i].d_hex, rsa_tests[i].key_size);
        if (ret != 0) {
            failed_tests++;
        }
    }

    // Test SM2 cipher
    total_tests++;
    uint32_t ret = test_sm2_cipher_single(ctx);
    if (ret != 0) {
        failed_tests++;
    }

    ehsm_port_printf("PKE Cipher Speed Tests completed: %d total, %d failed\n", total_tests, failed_tests);
    return failed_tests == 0 ? 0 : 1;
}