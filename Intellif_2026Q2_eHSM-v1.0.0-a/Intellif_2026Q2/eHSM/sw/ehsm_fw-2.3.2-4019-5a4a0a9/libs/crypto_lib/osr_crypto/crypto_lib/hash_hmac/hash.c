
#include "../../crypto_include/hash_hmac/hash.h"
#include "../../crypto_include/crypto_common/utility.h"


static const uint32_t *hash_get_IV(hash_alg_e alg);

static void hash_start_calculate(hash_ctx_st *ctx);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
static uint32_t hash_addr64_add_uint32(uint32_t *addr_h, uint32_t *addr_l, uint32_t offset);
#endif

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
static void hash_dma_update_recover_register(hash_dma_ctx_st *ctx);
#endif

//HASH IV definition
#ifndef HASH_CPU_BIG_ENDIAN

#ifdef SUPPORT_HASH_SM3
extern uint32_t const SM3_IV[8];
uint32_t const SM3_IV[8]         ={0x6F168073U,0xB9B21449U,0xD7422417U,0x00068ADAU,0xBC306FA9U,0xAA383116U,0x4DEE8DE3U,0x4E0EFBB0U,};
#endif

#ifdef SUPPORT_HASH_MD5
extern uint32_t const MD5_IV[4];
uint32_t const MD5_IV[4]         ={0x67452301U,0xefcdab89U,0x98badcfeU,0x10325476U,};
#endif

#ifdef SUPPORT_HASH_SHA256
extern uint32_t const SHA256_IV[8];
uint32_t const SHA256_IV[8]      ={0x67E6096AU,0x85AE67BBU,0x72F36E3CU,0x3AF54FA5U,0x7F520E51U,0x8C68059BU,0xABD9831FU,0x19CDE05BU,};
#endif

#ifdef SUPPORT_HASH_SHA384
extern uint32_t const SHA384_IV[16];
uint32_t const SHA384_IV[16]     ={0x5D9DBBCBU,0xD89E05C1U,0x2A299A62U,0x07D57C36U,0x5A015991U,0x17DD7030U,0xD8EC2F15U,0x39590EF7U,
                                   0x67263367U,0x310BC0FFU,0x874AB48EU,0x11155868U,0x0D2E0CDBU,0xA78FF964U,0x1D48B547U,0xA44FFABEU,};
#endif

#ifdef SUPPORT_HASH_SHA512
extern uint32_t const SHA512_IV[16];
uint32_t const SHA512_IV[16]     ={0x67E6096AU,0x08C9BCF3U,0x85AE67BBU,0x3BA7CA84U,0x72F36E3CU,0x2BF894FEU,0x3AF54FA5U,0xF1361D5FU,
                                   0x7F520E51U,0xD182E6ADU,0x8C68059BU,0x1F6C3E2BU,0xABD9831FU,0x6BBD41FBU,0x19CDE05BU,0x79217E13U,};
#endif

#ifdef SUPPORT_HASH_SHA1
extern uint32_t const SHA1_IV[5];
uint32_t const SHA1_IV[5]        ={0x01234567U,0x89ABCDEFU,0xFEDCBA98U,0x76543210U,0xF0E1D2C3U,};
#endif

#ifdef SUPPORT_HASH_SHA224
extern uint32_t const SHA224_IV[8];
uint32_t const SHA224_IV[8]      ={0xD89E05C1U,0x07D57C36U,0x17DD7030U,0x39590EF7U,0x310BC0FFU,0x11155868U,0xA78FF964U,0xA44FFABEU,};
#endif

#ifdef SUPPORT_HASH_SHA512_224
extern uint32_t const SHA512_224_IV[16];
uint32_t const SHA512_224_IV[16] ={0xC8373D8CU,0xA24D5419U,0x6699E173U,0xD6D4DC89U,0xAEB7FA1DU,0x829CFF32U,0x14D59D67U,0xCF9F2F58U,
                                   0x692B6D0FU,0xA84DD47BU,0x736FE377U,0x4289C404U,0xA8859D3FU,0xC8361D6AU,0xADE61211U,0xA192D691U,};
#endif

#ifdef SUPPORT_HASH_SHA512_256
extern uint32_t const SHA512_256_IV[16];
uint32_t const SHA512_256_IV[16] ={0x94213122U,0x2CF72BFCU,0xA35F559FU,0xC2644CC8U,0x6BB89323U,0x51B1536FU,0x19773896U,0xBDEA4059U,
                                   0xE23E2896U,0xE3FF8EA8U,0x251E5EBEU,0x92398653U,0xFC99012BU,0xAAB8852CU,0xDC2DB70EU,0xA22CC581U,};
#endif

//for SHA3, IV is zero of 1600 bits

#else

#ifdef SUPPORT_HASH_SM3
extern uint32_t const SM3_IV[8];
uint32_t const SM3_IV[8]         ={0x7380166fU,0x4914b2b9U,0x172442d7U,0xda8a0600U,0xa96f30bcU,0x163138aaU,0xe38dee4dU,0xb0fb0e4eU,};
#endif

#ifdef SUPPORT_HASH_MD5
extern uint32_t const MD5_IV[4];
uint32_t const MD5_IV[4]         ={0x01234567U,0x89ABCDEFU,0xFEDCBA98U,0x76543210U,};
#endif

#ifdef SUPPORT_HASH_SHA256
extern uint32_t const SHA256_IV[8];
uint32_t const SHA256_IV[8]      ={0x6a09e667U,0xbb67ae85U,0x3c6ef372U,0xa54ff53aU,0x510e527fU,0x9b05688cU,0x1f83d9abU,0x5be0cd19U,};
#endif

#ifdef SUPPORT_HASH_SHA384
extern uint32_t const SHA384_IV[16];
uint32_t const SHA384_IV[16]     ={0xcbbb9d5dU,0xc1059ed8U,0x629a292aU,0x367cd507U,0x9159015aU,0x3070dd17U,0x152fecd8U,0xf70e5939U,
                                   0x67332667U,0xffc00b31U,0x8eb44a87U,0x68581511U,0xdb0c2e0dU,0x64f98fa7U,0x47b5481dU,0xbefa4fa4U,};
#endif

#ifdef SUPPORT_HASH_SHA512
extern uint32_t const SHA512_IV[16];
uint32_t const SHA512_IV[16]     ={0x6a09e667U,0xf3bcc908U,0xbb67ae85U,0x84caa73bU,0x3c6ef372U,0xfe94f82bU,0xa54ff53aU,0x5f1d36f1U,
                                   0x510e527fU,0xade682d1U,0x9b05688cU,0x2b3e6c1fU,0x1f83d9abU,0xfb41bd6bU,0x5be0cd19U,0x137e2179U,};
#endif

#ifdef SUPPORT_HASH_SHA1
extern uint32_t const SHA1_IV[5];
uint32_t const SHA1_IV[5]        ={0x67452301U,0xefcdab89U,0x98badcfeU,0x10325476U,0xc3d2e1f0U,};
#endif

#ifdef SUPPORT_HASH_SHA224
extern uint32_t const SHA224_IV[8];
uint32_t const SHA224_IV[8]      ={0xc1059ed8U,0x367cd507U,0x3070dd17U,0xf70e5939U,0xffc00b31U,0x68581511U,0x64f98fa7U,0xbefa4fa4U,};
#endif

#ifdef SUPPORT_HASH_SHA512_224
extern uint32_t const SHA512_224_IV[16];
uint32_t const SHA512_224_IV[16] ={0x8C3D37C8U,0x19544DA2U,0x73E19966U,0x89DCD4D6U,0x1DFAB7AEU,0x32FF9C82U,0x679DD514U,0x582F9FCFU,
                                   0x0F6D2B69U,0x7BD44DA8U,0x77E36F73U,0x04C48942U,0x3F9D85A8U,0x6A1D36C8U,0x1112E6ADU,0x91D692A1U,};
#endif

#ifdef SUPPORT_HASH_SHA512_256
extern uint32_t const SHA512_256_IV[16];
uint32_t const SHA512_256_IV[16] ={0x22312194U,0xFC2BF72CU,0x9F555FA3U,0xC84C64C2U,0x2393B86BU,0x6F53B151U,0x96387719U,0x5940EABDU,
                                   0x96283EE2U,0xA88EFFE3U,0xBE5E1E25U,0x53863992U,0x2B0199FCU,0x2C85B8AAU,0x0EB72DDCU,0x81C52CA2U,};
#endif

//for SHA3, IV is zero of 1600 bits

#endif




/* function: check whether the hash algorithm is valid or not
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: HASH_SUCCESS(valid), other(invalid)
 * caution:
 */
uint32_t check_hash_alg(hash_alg_e alg)
{
    uint32_t ret;

    switch(alg)
    {
#ifdef SUPPORT_HASH_SM3
    case HASH_SM3:
#endif

#ifdef SUPPORT_HASH_MD5
    case HASH_MD5:
#endif

#ifdef SUPPORT_HASH_SHA256
    case HASH_SHA256:
#endif

#ifdef SUPPORT_HASH_SHA384
    case HASH_SHA384:
#endif

#ifdef SUPPORT_HASH_SHA512
    case HASH_SHA512:
#endif

#ifdef SUPPORT_HASH_SHA1
    case HASH_SHA1:
#endif

#ifdef SUPPORT_HASH_SHA224
    case HASH_SHA224:
#endif

#ifdef SUPPORT_HASH_SHA512_224
    case HASH_SHA512_224:
#endif

#ifdef SUPPORT_HASH_SHA512_256
    case HASH_SHA512_256:
#endif

#ifdef SUPPORT_HASH_SHA3_224
    case HASH_SHA3_224:
#endif

#ifdef SUPPORT_HASH_SHA3_256
    case HASH_SHA3_256:
#endif

#ifdef SUPPORT_HASH_SHA3_384
    case HASH_SHA3_384:
#endif

#ifdef SUPPORT_HASH_SHA3_512
    case HASH_SHA3_512:
#endif

        ret = HASH_SUCCESS;
        break;

    default:
        ret = HASH_INPUT_INVALID;
        break;
    }

    return ret;
}


/* function: get hash block word length
 * parameters:
 *     hash_alg_e ------------------- input, specific hash algorithm
 * return: hash block word length
 * caution:
 *     1. please make sure hash_alg_e is valid
 */
uint8_t hash_get_block_word_len(hash_alg_e alg)
{
    uint8_t block_words;

    switch(alg)
    {
#ifdef SUPPORT_HASH_SM3
    case HASH_SM3:
#endif

#ifdef SUPPORT_HASH_MD5
    case HASH_MD5:
#endif

#ifdef SUPPORT_HASH_SHA1
    case HASH_SHA1:
#endif

#ifdef SUPPORT_HASH_SHA256
    case HASH_SHA256:
#endif

#ifdef SUPPORT_HASH_SHA224
    case HASH_SHA224:
#endif

#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_MD5) || defined(SUPPORT_HASH_SHA1) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA224))
        block_words = 16;
        break;
#endif

#ifdef SUPPORT_HASH_SHA384
    case HASH_SHA384:
#endif

#ifdef SUPPORT_HASH_SHA512
    case HASH_SHA512:
#endif

#ifdef SUPPORT_HASH_SHA512_224
    case HASH_SHA512_224:
#endif

#ifdef SUPPORT_HASH_SHA512_256
    case HASH_SHA512_256:
#endif

#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) || defined(SUPPORT_HASH_SHA512_224) || defined(SUPPORT_HASH_SHA512_256))
        block_words = 32;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
    case HASH_SHA3_224:
        block_words = 36;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_256
    case HASH_SHA3_256:
        block_words = 34;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_384
    case HASH_SHA3_384:
        block_words = 26;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_512
    case HASH_SHA3_512:
        block_words = 18;
        break;
#endif

    default:
	block_words = 0;
        break;
    }

    return block_words;
}


/* function: get hash iterator word length
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: hash iterator word length
 * caution:
 *     1. please make sure alg is valid
 */
uint8_t hash_get_iterator_word_len(hash_alg_e alg)
{
    uint8_t iterator_words;

    switch(alg)
    {
#ifdef SUPPORT_HASH_MD5
    case HASH_MD5:
        iterator_words = 4;
        break;
#endif

#ifdef SUPPORT_HASH_SHA1
    case HASH_SHA1:
        iterator_words = 5;
        break;
#endif

#ifdef SUPPORT_HASH_SM3
    case HASH_SM3:
#endif

#ifdef SUPPORT_HASH_SHA256
    case HASH_SHA256:
#endif

#ifdef SUPPORT_HASH_SHA224
    case HASH_SHA224:
#endif

#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA224))
        iterator_words = 8;
        break;
#endif

#ifdef SUPPORT_HASH_SHA384
    case HASH_SHA384:
#endif

#ifdef SUPPORT_HASH_SHA512
    case HASH_SHA512:
#endif

#ifdef SUPPORT_HASH_SHA512_224
    case HASH_SHA512_224:
#endif

#ifdef SUPPORT_HASH_SHA512_256
    case HASH_SHA512_256:
#endif

#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) || defined(SUPPORT_HASH_SHA512_224) || defined(SUPPORT_HASH_SHA512_256))
        iterator_words = 16;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
        case HASH_SHA3_224:
#endif

#ifdef SUPPORT_HASH_SHA3_256
        case HASH_SHA3_256:
#endif

#ifdef SUPPORT_HASH_SHA3_384
        case HASH_SHA3_384:
#endif

#ifdef SUPPORT_HASH_SHA3_512
        case HASH_SHA3_512:
#endif

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
        iterator_words = 50;
        break;
#endif

    default:
	iterator_words = 0;
        break;
    }

    return iterator_words;
}


/* function: get hash digest word length
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: hash digest word length
 * caution:
 *     1. please make sure alg is valid
 */
uint8_t hash_get_digest_word_len(hash_alg_e alg)
{
    uint8_t digest_words;

    switch(alg)
    {
#ifdef SUPPORT_HASH_MD5
    case HASH_MD5:
        digest_words = 4;
        break;
#endif

#ifdef SUPPORT_HASH_SHA1
    case HASH_SHA1:
        digest_words = 5;
        break;
#endif

#ifdef SUPPORT_HASH_SHA224
    case HASH_SHA224:
#endif

#ifdef SUPPORT_HASH_SHA512_224
    case HASH_SHA512_224:
#endif

#if (defined(SUPPORT_HASH_SHA224) || defined(SUPPORT_HASH_SHA512_224))
        digest_words = 7;
        break;
#endif

#ifdef SUPPORT_HASH_SM3
    case HASH_SM3:
#endif

#ifdef SUPPORT_HASH_SHA256
    case HASH_SHA256:
#endif

#ifdef SUPPORT_HASH_SHA512_256
    case HASH_SHA512_256:
#endif

#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA512_256))
        digest_words = 8;
        break;
#endif

#ifdef SUPPORT_HASH_SHA384
    case HASH_SHA384:
        digest_words = 12;
        break;
#endif

#ifdef SUPPORT_HASH_SHA512
    case HASH_SHA512:
        digest_words = 16;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
    case HASH_SHA3_224:
        digest_words = 7;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_256
    case HASH_SHA3_256:
        digest_words = 8;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_384
    case HASH_SHA3_384:
        digest_words = 12;
        break;
#endif

#ifdef SUPPORT_HASH_SHA3_512
    case HASH_SHA3_512:
        digest_words = 16;
        break;
#endif

    default:
	    digest_words = 0;
        break;
    }

    return digest_words;
}


/* function: get hash IV pointer
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 * return: IV address
 * caution:
 */
static const uint32_t *hash_get_IV(hash_alg_e alg)
{
    const uint32_t *iv;

    switch(alg)
    {
#ifdef SUPPORT_HASH_SM3
    case HASH_SM3:
        iv = SM3_IV;
        break;
#endif

#ifdef SUPPORT_HASH_MD5
    case HASH_MD5:
        iv = MD5_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA256
    case HASH_SHA256:
        iv = SHA256_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA384
    case HASH_SHA384:
        iv = SHA384_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA1
    case HASH_SHA1:
        iv = SHA1_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA512
    case HASH_SHA512:
        iv = SHA512_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA224
    case HASH_SHA224:
        iv = SHA224_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA512_224
    case HASH_SHA512_224:
        iv = SHA512_224_IV;
        break;
#endif

#ifdef SUPPORT_HASH_SHA512_256
    case HASH_SHA512_256:
        iv = SHA512_256_IV;
        break;
#endif

    //here iv = NULL means SHA3 IV is zero of 1600 bits
    default:
        iv = NULL;
        break;
    }

    return iv;
}


/* function: input hash IV
 * parameters:
 *     alg ------------------- input, specific hash algorithm
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 */
void hash_set_IV(hash_alg_e alg, uint32_t hash_iterator_words)
{
    hash_set_iterator(hash_get_IV(alg), hash_iterator_words);
}


/* function: hash message total byte length a = a+b
 * parameters:
 *     a -------------------------- input&output, big number a, total byte length of hash message
 *     a_words -------------------- input, word length of buffer a
 *     b -------------------------- input, integer to be added to a
 * return: 0(success), other(error, hash total length overflow)
 * caution:
 */
uint32_t hash_total_byte_len_add_uint32(uint32_t *a, uint32_t a_words, uint32_t b)
{
    uint32_t i, ret;
    uint32_t tmp = b;

    for(i=0U; i<a_words; i++)
    {
        a[i] += tmp;
        if(a[i] < tmp)
        {
            tmp = 1U;
        }
        else
        {
            break;
        }
    }

    if(i == a_words)
    {
        ret = 1U;
    }
    else if(0U != (a[a_words-1U] & 0xE0000000U))  //bit length overflow
    {
        ret = 1U;
    }
    else
    {
        ret = 0U;
    }

    return ret;
}


#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
/* function: hash 64-bit address addition addr = addr+offset
 * parameters:
 *     addr_h --------------------- input&output, high address
 *     addr_l --------------------- input&output, low address
 *     offset --------------------- input, address offset
 * return: 0(success), other(error, hash 64-bit address overflow)
 * caution:
 */
static uint32_t hash_addr64_add_uint32(uint32_t *addr_h, uint32_t *addr_l, uint32_t offset)
{
    uint32_t ret = HASH_SUCCESS;
    uint32_t carry;

    do
    {
        (*addr_l) += offset;
        if((*addr_l) < offset)
        {
            carry = 1U;
        }
        else
        {
            break;
        }

        (*addr_h) += carry;

        if((*addr_h) < carry)
        {
            ret = HASH_LEN_OVERFLOW;
        }
        else
        {}
    } while (0);

    return ret;
}
#endif


/* function: transform hash message total byte length to bit length
 * parameters:
 *     a -------------------------- input&output, big number a
 *     a_words -------------------- input, word length of buffer a
 * return: none
 * caution:
 */
void hash_total_bytelen_2_bitlen(uint32_t *a, uint32_t a_words)
{
    uint32_t i;

    for(i = (a_words-1U); i>0U; i--)
    {
        a[i] <<= 3;
        a[i] |= a[i-1U]>>(32-3);
    }
    a[i] <<= 3;
}


/* function: start HASH iteration calc
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 * return: none
 * caution:
 *     1. if it is the first time to update, for hash, setting IV is needed, but for hmac not.
 */
static void hash_start_calculate(hash_ctx_st *ctx)
{
    if(0U != (ctx->first_update_flag))
    {
        if(HASH_MODE == ctx->hfe_mode)   //for hash, if it is the first time to calculate, set the IV
        {
            hash_set_IV(ctx->alg, ctx->iterator_word_len);
        }
        else
        {}

        ctx->first_update_flag = (uint8_t)0;   //clear the flag
    }
    else
    {}

    hash_start();
}


#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
/* function: recover hash hardware register information
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_st context pointer
 * return:
 * caution:
 *     1. please make sure the parameters are valid, and ctx is initialized
 */
static void hash_dma_update_recover_register(hash_dma_ctx_st *ctx)
{
    if(HASH_MODE == ctx->hfe_mode)
    {
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_dma_interruption();
        hash_set_last_block(0);
        hash_set_alg(ctx->alg);
        hash_update_config();
        hash_clear_risr();
        hash_set_dma_output_len(0);
        hash_set_dma_mode();
    }
    else
    {}

    //set the input iterator data
    if(((uint8_t)0) != (ctx->first_update_flag))
    {
        if(HASH_MODE == ctx->hfe_mode)
        {
            hash_set_IV(ctx->alg, ctx->iterator_word_len);
        }
        else
        {}

        ctx->first_update_flag = (uint8_t)0;   //clear the flag
    }
    else
    {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
}
#endif


/* function: init HASH
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     alg ------------------- input, specific hash algorithm
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 */
uint32_t hash_init(hash_ctx_st *ctx, hash_alg_e alg)
{
    uint32_t ret = HASH_SUCCESS;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else if(HASH_SUCCESS != check_hash_alg(alg))
    {
        ret = HASH_INPUT_INVALID;
    }
    else
    {
        memset_(ctx, 0, sizeof(hash_ctx_st));

#ifndef CONFIG_HASH_SUPPORT_MUL_THREAD
        hash_set_cpu_mode();
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_cpu_interruption();
        hash_set_last_block(0);//set not the last block
        hash_set_alg(alg);
        hash_update_config();
#endif

        //set context config
        ctx->alg          = alg;
        ctx->hfe_mode          = HASH_MODE;
        ctx->block_byte_len    = hash_get_block_word_len(alg)<<2;
        ctx->iterator_word_len = hash_get_iterator_word_len(alg);
        ctx->digest_byte_len   = hash_get_digest_word_len(alg)<<2;
        ctx->status.busy       = 0U;
        ctx->first_update_flag = (uint8_t)1;
        ctx->finish_flag       = (uint8_t)0;
    }

    return ret;
}


/* function: init HASH with iv and updated message length
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     alg ------------------------ input, specific hash algorithm
 *     iv ------------------------- input, iv or iterator after updating some blocks
 *     byte_length_h -------------- input, high 32 bit of updated message byte length
 *     byte_length_l -------------- input, low 32 bit of updated message byte length,
 *                                         this must be a multiple of block byte length
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. updated message byte length must be a multiple of block byte length
 */
uint32_t hash_init_with_iv_and_updated_length(hash_ctx_st *ctx, hash_alg_e alg, const uint32_t *iv, 
        uint32_t byte_length_h, uint32_t byte_length_l)
{
    uint32_t ret;
    uint8_t block_byte_len = hash_get_block_word_len(alg)<<2;

    if(0U != block_byte_len)
    {
        if(0U != (byte_length_l % CAST2UINT32(block_byte_len)))
        {
            ret = HASH_INPUT_INVALID;
        }
        else
        {
            ret = hash_init(ctx, alg);
            if(HASH_SUCCESS == ret)
            {
                ctx->first_update_flag = (uint8_t)0;
                ctx->total[0] = byte_length_l;
                ctx->total[1] = byte_length_h;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
                memcpy_((uint8_t *)ctx->iterator, iv, CAST2UINT32(ctx->iterator_word_len)<<2);
#else
                hash_set_iterator(iv, ctx->iterator_word_len);
#endif
            }
            else
            {}
        }
    }
    else
    {
        ret = HASH_INPUT_INVALID;
    }

    return ret;
}


/* function: hash iterate calc with some blocks
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     msg ------------------------ input, message of some blocks
 *     block_count ---------------- input, count of blocks
 * return: none
 * caution:
 *     1. please make sure the three parameters is valid
 */
uint32_t hash_calc_blocks(hash_ctx_st *ctx, const uint8_t *msg, uint32_t block_count)
{
    uint32_t ret;
    const uint8_t *in = msg;
    uint32_t round_num = block_count;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    //set the input iterator data
    if(((uint8_t)1) != ctx->first_update_flag)
    {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
    else
    {}
#endif

    //set the bit length of the input blocks
    hash_set_msg_len(ctx->block_byte_len * block_count);

    hash_start_calculate(ctx);

    while(0U != round_num)
    {
        //input the block message
        hash_input_msg_u8(in, ctx->block_byte_len);
        in = &(in[ctx->block_byte_len]);
        round_num--;
    }

    ret = hash_wait_till_done();

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    if(HASH_SUCCESS == ret)
    {
        //if message update not done, get the new iterator hash value
        if(((uint8_t)1) != ctx->finish_flag)
        {
            hash_get_iterator((uint8_t *)(ctx->iterator), ctx->iterator_word_len);
        }
        else
        {}
    }
    else
    {}
#endif

    return ret;
}


/* function: hash iterate calc with padding
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     msg ------------------------ input, message that contains the last block(maybe not full)
 *     bytes ---------------------- input, byte length of msg
 * return: none
 * caution:
 *     1. msg contains the last byte of the total message while the total message length is not a
 *        multiple of hash block length, otherwise byte length of msg is zero.
 *     2. at present this function does not support the case that byte length of msg is a multiple
 *        of hash block length. actually msg_bytes here must be less than the hash block byte length,
 *        namely, this function is just for the remainder message, and will do padding, finally get
 *        digest.
 *     3. before calling this function, some blocks(could be 0 block) must be calculated.
 */
uint32_t hash_calc_rand_len_msg(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    //set the input iterator data
    if(((uint8_t)1) != ctx->first_update_flag)
    {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
    else
    {}
#endif

    hash_set_last_block(1);

    hash_start_calculate(ctx);

    hash_input_msg_u8(msg, msg_bytes);

    return hash_wait_till_done();
}


/* function: hash message total byte length a = a+b
 * parameters:
 *     ctx ------------------------------- input, hash_ctx_st context pointer
 *     in_bytes -------------------------- input, integer to be added to ctx->ctx
 * return: 0(success), other(error, hash total length overflow)
 * caution:
 */
uint32_t hash_total_byte_len_update(hash_ctx_st *ctx, uint32_t in_bytes)
{
    uint32_t ret = HASH_SUCCESS;
    uint8_t left;

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
    if(0U != (hash_check_whether_sha3_alg(ctx->alg)))
    {
        left = (uint8_t)(ctx->total[0] % (ctx->block_byte_len));
        ctx->total[0] = (left + in_bytes) % (ctx->block_byte_len);
    }
    else
#endif
    {
        //update total byte length
        if(0U != (hash_total_byte_len_add_uint32(ctx->total, CAST2UINT32(ctx->block_byte_len)/32U, in_bytes)))
        {
            ret = HASH_LEN_OVERFLOW;
        }
        else
        {}
    }

    return ret;
}


/* function: hash update message
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hash_update(hash_ctx_st *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t ret = HASH_SUCCESS;
    uint32_t count;
    uint8_t left, fill;
    const uint8_t *in = msg;
    uint32_t in_bytes = msg_bytes;
    uint8_t is_finished = (uint8_t)0;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {}

    if((NULL != in) && (0U != in_bytes) && (HASH_SUCCESS == ret))
    {
        ctx->status.busy = 1U;                             //start to update processing

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        hash_set_cpu_mode();
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_cpu_interruption();
        hash_set_last_block(0);//set not the last block
        hash_set_alg(ctx->alg);
        hash_update_config();
#endif

        left = (uint8_t)(ctx->total[0] % (ctx->block_byte_len));    //byte length of valid message left in block buffer
        fill = (ctx->block_byte_len) - left;             //byte length that block buffer need to fill a block

        ret = hash_total_byte_len_update(ctx, in_bytes);

        if((0U != left) && (HASH_SUCCESS == ret))
        {
            if(in_bytes >= fill)
            {
                memcpy_(&(ctx->hash_buffer[left]), in, fill);
                ret = hash_calc_blocks(ctx, ctx->hash_buffer, 1);
                if(HASH_SUCCESS == ret)
                {
                    in_bytes -= fill;
                    in = &(in[fill]);
                }
                else
                {}
            }
            else
            {
                memcpy_(&(ctx->hash_buffer[left]), in, in_bytes);
                is_finished = (uint8_t)1;
            }
        }
        else
        {}

        if(((uint8_t)0 == is_finished) && (HASH_SUCCESS == ret))
        {
            //process some blocks
            count = in_bytes/(ctx->block_byte_len);
            if(0U != count)
            {
                ret = hash_calc_blocks(ctx, in, count);
            }
            else
            {}

            if(HASH_SUCCESS == ret)
            {
                //process the remainder
                in_bytes = in_bytes % (ctx->block_byte_len);
                if(0U != in_bytes)
                {
                    in = &(in[(ctx->block_byte_len)*count]);
                    memcpy_(ctx->hash_buffer, in, in_bytes);
                }
                else
                {}
            }
            else
            {}
        }
        else
        {}

        ctx->status.busy = 0U;   //update end, status becomes idle
    }

    return ret;
}


/* function: message update done, get the digest
 * parameters:
 *     ctx ------------------------ input, hash_ctx_st context pointer
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the digest buffer is sufficient
 */
uint32_t hash_final(hash_ctx_st *ctx, uint8_t *digest)
{
    uint32_t ret;
    uint8_t tmp;

    if((NULL == ctx) || (NULL == digest))
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        hash_set_cpu_mode();
        hash_set_hash_mode();
        hash_set_endian_uint32();
        hash_disable_cpu_interruption();
        //hash_set_last_block(0) //set not the last block
        hash_set_alg(ctx->alg);
        hash_update_config();
#endif
        ctx->finish_flag = ((uint8_t)1);    //the last block calc

        //get the byte length of the remainder msg(less than one block)
        tmp = (uint8_t)(ctx->total[0] % (ctx->block_byte_len));

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
        if(0U != (hash_check_whether_sha3_alg(ctx->alg)))
        {
            //set remainder msg bit length
            hash_set_msg_len(tmp);
        }
        else
#endif
        {
            //set total msg bit length
            hash_total_bytelen_2_bitlen(ctx->total, CAST2UINT32(ctx->block_byte_len)/32U);
            hash_set_msg_total_bit_len(ctx->total, ctx->block_byte_len);
        }

        //input the remainder msg(less than one block)
        ret = hash_calc_rand_len_msg(ctx, ctx->hash_buffer, tmp);
        if(HASH_SUCCESS == ret)
        {
            //get the hash result
            hash_get_iterator(digest, CAST2UINT32(ctx->digest_byte_len)>>2);
        }
        else
        {}
    }

    //clear the context
    if(NULL != ctx)
    {
        memset_(ctx, 0, sizeof(hash_ctx_st));
    }
    else
    {}

    return ret;
}


/* function: input whole message and get its digest
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t hash(hash_alg_e alg, const uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    hash_ctx_st ctx[1];
    uint32_t ret;

    ret = hash_init(ctx, alg);
    if(HASH_SUCCESS == ret)
    {
        ret = hash_update(ctx, msg, msg_bytes);
        if(HASH_SUCCESS == ret)
        {
            ret = hash_final(ctx, digest);
        }
        else
        {}
    }
    else
    {}

    return ret;
}


#ifdef SUPPORT_HASH_NODE
/* function: input whole message and get its digest(node style)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 */
uint32_t hash_node_steps(hash_alg_e alg, const hash_node_st *node, uint32_t node_num, uint8_t *digest)
{
    hash_ctx_st ctx[1];
    uint32_t i, ret;

    ret = hash_init(ctx, alg);
    if(HASH_SUCCESS == ret)
    {
        for(i=0U; i<node_num; i++)
        {
            ret = hash_update(ctx, node[i].msg_addr, node[i].msg_bytes);
            if(HASH_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }

        if(HASH_SUCCESS == ret)
        {
            ret = hash_final(ctx, digest);
        }
        else
        {}
    }
    else
    {}

    return ret;
}
#endif


#ifdef HASH_DMA_FUNCTION
/* function: init dma hash with iv and updated message length
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_st context pointer
 *     alg ------------------------ input, specific hash algorithm
 *     iv ------------------------- input, iv or iterator after updating some blocks
 *     byte_length_h -------------- input, high 32 bit of updated message byte length
 *     byte_length_l -------------- input, low 32 bit of updated message byte length,
 *                                         this must be a multiple of block byte length
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure alg is valid
 *     2. updated message byte length must be a multiple of block byte length
 */
uint32_t hash_dma_init_with_iv_and_updated_length(hash_dma_ctx_st *ctx, hash_alg_e alg, const uint32_t *iv, 
        uint32_t byte_length_h, uint32_t byte_length_l, HASH_CALLBACK callback)
{
    uint32_t ret = HASH_SUCCESS;
    uint8_t block_word_len;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else if(HASH_SUCCESS != check_hash_alg(alg))
    {
        ret = HASH_INPUT_INVALID;
    }
    else
    {
        block_word_len = hash_get_block_word_len(alg);
        if(0U != block_word_len)
        {
            if(0U != (byte_length_l % (CAST2UINT32(block_word_len)<<2)))
            {
                ret = HASH_INPUT_INVALID;
            }
            else
            {}
        }
        else
        {
            ret = HASH_INPUT_INVALID;
        }

        if(HASH_SUCCESS == ret)
        {
            //clear the context
            memset_(ctx, 0, sizeof(hash_dma_ctx_st));

            //init context
            ctx->alg          = alg;
            ctx->block_word_len    = block_word_len;
            ctx->callback          = callback;
            ctx->total[0]          = byte_length_l;
            ctx->total[1]          = byte_length_h;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            ctx->hfe_mode          = HASH_MODE;
            ctx->iterator_word_len = hash_get_iterator_word_len(alg);
            if(NULL != iv)
            {
                ctx->first_update_flag = (uint8_t)0;
                memcpy_((ctx->iterator), iv, CAST2UINT32(ctx->iterator_word_len)<<2);
            }
            else
            {
                ctx->first_update_flag = (uint8_t)1;
            }
#else
            hash_set_hash_mode();
            hash_set_endian_uint32();
            hash_disable_dma_interruption();
            hash_set_last_block(0);
            hash_set_alg(alg);
            hash_update_config();
            hash_clear_risr();
            hash_set_dma_output_len(0);
            hash_set_dma_mode();

            //set IV
            if(NULL != iv)
            {
                hash_set_iterator(iv, hash_get_iterator_word_len(alg));
            }
            else
            {
                hash_set_IV(alg, hash_get_iterator_word_len(alg));
            }
#endif
        }
    }

    return ret;
}


/* function: init dma hash
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_st context pointer
 *     alg ------------------------ input, specific hash algorithm
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hash_dma_init(hash_dma_ctx_st *ctx, hash_alg_e alg, HASH_CALLBACK callback)
{
    return hash_dma_init_with_iv_and_updated_length(ctx, alg, NULL, 0, 0, callback);
}


/* function: dma hash update some message blocks
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_st context pointer
 *     msg ------------------------ input, message blocks
 *     msg_bytes ------------------ input, byte length of the input message, must be a multiple of hash block byte length
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_update_blocks(hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes)
#else
uint32_t hash_dma_update_blocks(hash_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes)
#endif
{
    uint32_t ret = HASH_SUCCESS;

    if(NULL == ctx)
    {
        ret = HASH_BUFFER_NULL;
    }
    else if(0U != (msg_bytes % (CAST2UINT32(ctx->block_word_len)<<2)))
    {
        ret = HASH_INPUT_INVALID;
    }
    else
    {
        //handle other
    }

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((0U != msg_bytes) && ((0U != msg_h) || (0U != msg_l)) && (HASH_SUCCESS == ret))
#else
    if((0U != msg_bytes) && (NULL != msg) && (HASH_SUCCESS == ret))
#endif
    {
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
        if(0U == hash_check_whether_sha3_alg(ctx->alg))
        {
#endif
            //update total byte length
            if(0U != (hash_total_byte_len_add_uint32(ctx->total, CAST2UINT32(ctx->block_word_len)/8U, msg_bytes)))
            {
                ret = HASH_LEN_OVERFLOW;
            }
            else
            {}
#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
        }
        else
        {}
#endif

        if(HASH_SUCCESS == ret)
        {
#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            hash_dma_update_recover_register(ctx);
#endif

            hash_set_msg_len(msg_bytes);

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_operate(msg_h, msg_l, 0, 0, msg_bytes, ctx->callback);
#else
            ret = hash_dma_operate(msg, NULL, msg_bytes, ctx->callback);
#endif

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
            if(HASH_SUCCESS == ret)
            {
                //get the new iterator hash value
                hash_get_iterator((uint8_t *)(ctx->iterator), (uint32_t)(ctx->iterator_word_len));
            }
            else
            {}
#endif
        }
        else
        {}
    }

    return ret;
}


/* function: dma hash final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_st context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 *     2. if remainder_msg is NULL, or remainder_bytes is zero, in this case input valid,
 *        means the message is NULL.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_final(hash_dma_ctx_st *ctx, uint32_t msg_h, uint32_t msg_l, 
        uint32_t msg_bytes, uint32_t digest_h, uint32_t digest_l)
#else
uint32_t hash_dma_final(hash_dma_ctx_st *ctx, const uint32_t *msg, uint32_t msg_bytes, uint32_t *digest)
#endif
{
    uint32_t remainder_bytes, blocks_bytes;
    uint32_t actual_msg_bytes = msg_bytes;
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    uint32_t current_msg_h = msg_h;
    uint32_t current_msg_l = msg_l;
#else
    const uint32_t *current_msg = msg;
#endif
    uint32_t ret = HASH_SUCCESS;

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((NULL == ctx) || ((0U == digest_h) && (0U == digest_l)))
#else
    if((NULL == ctx) || (NULL == digest))
#endif
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
        if((0U == msg_h) && (0U == msg_l))
        {
#else
        if((NULL == msg))
        {
#endif
            actual_msg_bytes = 0U;
        }
        else
        {}

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
        hash_dma_update_recover_register(ctx);
#endif
        //update some whole blocks
        remainder_bytes = actual_msg_bytes % (CAST2UINT32(ctx->block_word_len)<<2);
        blocks_bytes = actual_msg_bytes - remainder_bytes;
        if(0U != blocks_bytes)
        {
            hash_set_msg_len(blocks_bytes);
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_operate(msg_h, msg_l, 0U, 0U, blocks_bytes, ctx->callback);
            if(HASH_SUCCESS == ret)
            {
                ret = hash_addr64_add_uint32(&current_msg_h, &current_msg_l, blocks_bytes);
            }
            else
            {}
#else
            ret = hash_dma_operate(msg, NULL, blocks_bytes, ctx->callback);
            current_msg = &(msg[blocks_bytes>>2]);
#endif
        }
        else
        {}

        if(HASH_SUCCESS == ret)
        {
            //update the remainder message(maybe empty)
            hash_set_last_block(1);
            hash_set_dma_output_len((uint32_t)hash_get_digest_word_len(ctx->alg)<<2);

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) || defined(SUPPORT_HASH_SHA3_384) || defined(SUPPORT_HASH_SHA3_512))
            if(0U != (hash_check_whether_sha3_alg(ctx->alg)))
            {
                hash_set_msg_len(remainder_bytes);
            }
            else
#endif
            {
                //update total byte length
                if(0U != (hash_total_byte_len_add_uint32(ctx->total, CAST2UINT32(ctx->block_word_len)/8U, actual_msg_bytes)))
                {
                    ret = HASH_LEN_OVERFLOW;
                }
                else
                {
                    //set total length of message
                    hash_total_bytelen_2_bitlen(ctx->total, CAST2UINT32(ctx->block_word_len)>>3);
                    hash_set_msg_total_bit_len(ctx->total, CAST2UINT32(ctx->block_word_len)<<2);
                }
            }

            if(HASH_SUCCESS == ret)
            {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
                ret = hash_dma_operate(current_msg_h, current_msg_l, digest_h, digest_l, remainder_bytes, ctx->callback);
#else
                ret = hash_dma_operate(current_msg, digest, remainder_bytes, ctx->callback);
#endif
            }
            else
            {}
        }
        else
        {}

        //clear the context
        if(HASH_SUCCESS == ret)
        {
            memset_(ctx, 0, sizeof(hash_dma_ctx_st));
        }
        else
        {}
    }

    return ret;
}


/* function: dma hash digest calculate
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, hash digest
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma(hash_alg_e alg, uint32_t msg_h, uint32_t msg_l, uint32_t msg_bytes, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback)
#else
uint32_t hash_dma(hash_alg_e alg, uint32_t *msg, uint32_t msg_bytes, uint32_t *digest, HASH_CALLBACK callback)
#endif
{
    uint32_t ret;
    hash_dma_ctx_st ctx[1];

#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
    if((0U == digest_h) && (0U == digest_l))
#else
    if(NULL == digest)
#endif
    {
        ret = HASH_BUFFER_NULL;
    }
    else
    {
        ret = hash_dma_init(ctx, alg, callback);
        if(HASH_SUCCESS == ret)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_final(ctx, msg_h, msg_l, msg_bytes, digest_h, digest_l);
#else
            ret = hash_dma_final(ctx, msg, msg_bytes, digest);
#endif
        }
        else
        {}
    }

    return ret;
}


#ifdef SUPPORT_HASH_DMA_NODE
/* function: input whole message and get its digest(dma node style)
 * parameters:
 *     alg ------------------------ input, specific hash algorithm
 *     node ----------------------- input, message node pointer
 *     node_num ------------------- input, number of hash nodes, i.e. number of message segments.
 *     digest --------------------- output, hash digest
 *     callback ------------------- input, callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 *     2. if the whole message consists of some segments, every segment is a node, a node includes
 *        address and byte length.
 *     3. for every node or segment except the last, its message length must be a multiple of block length.
 */
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
uint32_t hash_dma_node_steps(hash_alg_e alg, const hash_dma_node_st *node, uint32_t node_num, uint32_t digest_h, 
        uint32_t digest_l, HASH_CALLBACK callback)
#else
uint32_t hash_dma_node_steps(hash_alg_e alg, const hash_dma_node_st *node, uint32_t node_num, uint32_t *digest, 
        HASH_CALLBACK callback)
#endif
{
    hash_dma_ctx_st ctx[1];
    uint32_t i, ret;

    ret = hash_dma_init(ctx, alg, callback);
    if(HASH_SUCCESS == ret)
    {
        for(i=0; i<(node_num-1U); i++)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_update_blocks(ctx, node[i].msg_addr_h, node[i].msg_addr_l, node[i].msg_bytes);
#else
            ret = hash_dma_update_blocks(ctx, node[i].msg_addr, node[i].msg_bytes);
#endif
            if(HASH_SUCCESS != ret)
            {
                break;
            }
            else
            {}
        }

        if(HASH_SUCCESS == ret)
        {
#ifdef CONFIG_HASH_SUPPORT_DMA_HIGH_LOW_ADDRESS
            ret = hash_dma_final(ctx, node[i].msg_addr_h, node[i].msg_addr_l, node[i].msg_bytes, digest_h, digest_l);
#else
            ret = hash_dma_final(ctx, node[i].msg_addr, node[i].msg_bytes, digest);
#endif
        }
    }
    else
    {}
    
    return ret;
}
#endif

#endif
