#include "../../crypto_include/pke_config.h"


#ifdef SUPPORT_DH

#include "../../crypto_include/pke/dh.h"
#include "../../crypto_include/trng/trng.h"
#include "../../crypto_include/crypto_common/utility.h"



/* Function: DH parameters pointer init, set pointers of (p, q, g)
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     p_buf ---------------------- input, a U32 buffer holds p, the prime defining the GF(p)
 *     p_bits --------------------- input, bit length of p
 *     p_h_buf -------------------- input, a U32 buffer holds pre-calculated mont parameters H(R^2 mod p)
 *     q_buf ---------------------- input, a U32 buffer holds q, a prime factor of p-1, aka order of g.
 *     q_bits --------------------- input, bit length of q
 *     g_buf ---------------------- input, a U32 buffer holds g, a generator of the q-order subgroup of GF(p)*
 *     g_bits --------------------- input, bit length of g
 * Return:
 *     DH_SUCCESS(success); other(error)
 * Caution:
 *     1. p_h_buf holds H(R^2 mod p), to accelerate DH calculation, it occupies the same size 
 *        memory as well as p_buf. if you do not have this, please set p_h_buf to NULL.
 */
uint32_t dh_param_pointer_init(dh_para_st *dh_para, uint32_t *p_buf, uint32_t p_bits, uint32_t *p_h_buf, 
        uint32_t *q_buf, uint32_t q_bits, uint32_t *g_buf, uint32_t g_bits)
{
    uint32_t ret;

    if((NULL == dh_para) || (NULL == p_buf) || (NULL == q_buf) || (NULL == g_buf))
    {
        ret = DH_POINTER_NULL;
    }
    else if((0u == p_bits) || (0u == q_bits) || (0u == g_bits))
    {
        ret = DH_INVALID_INPUT;
    }
    else if((q_bits > p_bits) || (g_bits > p_bits))
    {
        ret = DH_INVALID_INPUT;
    }
    else if(p_bits > DH_MAX_BIT_LEN)
    {
        ret = DH_INVALID_INPUT;
    }
    else
    {
        dh_para->p      = p_buf;
        dh_para->p_bits = p_bits;
        dh_para->p_h    = p_h_buf;
        dh_para->q      = q_buf;
        dh_para->q_bits = q_bits;
        dh_para->g      = g_buf;
        dh_para->g_bits = g_bits;

        ret = DH_SUCCESS;
    }

    return ret;
}


/* Function: DH parameters value init, set values of (p, q, g)
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     p -------------------------- input, a prime defining the GF(p)
 *     p_h ------------------------ input, the pre-calculated mont parameter (R^2 mod p)
 *     q -------------------------- input, a prime factor of p-1, aka order of g.
 *     g -------------------------- input, a generator of the q-order subgroup of GF(p)*
 * Return:
 *     DH_SUCCESS(success); other(error)
 * Caution:
 *     1. please call dh_param_pointer_init() before calling this function.
 *     2. the input p occupies (dh_para->p_bits+7)/8 bytes
 *        the input p_h occupies (dh_para->p_bits+7)/8 bytes, if you have this
 *        the input q occupies (dh_para->q_bits+7)/8 bytes
 *        the input g occupies (dh_para->g_bits+7)/8 bytes
 *     3. if you do not have p_h, please set p_h to NULL
 */
uint32_t dh_param_value_init(const dh_para_st *dh_para, const uint8_t *p, const uint8_t *p_h, 
        const uint8_t *q, const uint8_t *g)
{
    uint32_t pByteLen = 0u, qByteLen = 0u, gByteLen = 0u;
    uint32_t ret;

    if((NULL == dh_para) || (NULL == p) || (NULL == q) || (NULL == g))
    {
        ret = DH_POINTER_NULL;
    }
    else
    {
        ret = PKE_SUCCESS;

        pByteLen = GET_BYTE_LEN(dh_para->p_bits);
        qByteLen = GET_BYTE_LEN(dh_para->q_bits);
        gByteLen = GET_BYTE_LEN(dh_para->g_bits);

        if(NULL != p_h)
        {
            if(NULL == dh_para->p_h)
            {
                ret = DH_POINTER_NULL;
            }
            else
            {
                reverse_byte_array(p_h, (uint8_t *)dh_para->p_h, pByteLen);
            }
        }
        else
        {}
    }

    if(PKE_SUCCESS == ret)
    {
        reverse_byte_array(p, (uint8_t *)dh_para->p, pByteLen);
        reverse_byte_array(q, (uint8_t *)dh_para->q, qByteLen);
        reverse_byte_array(g, (uint8_t *)dh_para->g, gByteLen);

        //p and q can not be even.
        if((0u == (dh_para->p[0] & 1u)) || (0u == (dh_para->q[0] & 1u)))
        {
            ret = DH_INVALID_INPUT;
        }
        else
        {}
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        //to support p == dh_para->p
        if(0u != (pByteLen & 3u))
        {
            memset_(&((uint8_t *)dh_para->p)[pByteLen], 0, 4u - (pByteLen & 3u));
        }
        else
        {}

        //to support q == dh_para->q
        if(0u != (qByteLen & 3u))
        {
            memset_(&((uint8_t *)dh_para->q)[qByteLen], 0, 4u - (qByteLen & 3u));
        }
        else
        {}

        //to support g == dh_para->g
        if(0u != (gByteLen & 3u))
        {
            memset_(&((uint8_t *)dh_para->g)[gByteLen], 0, 4u - (gByteLen & 3u));
        }
        else
        {}

        ret = DH_SUCCESS;
    }
    else
    {}

    return ret;
}


#if 0
uint32_t dh_parameter_check(dh_para_st *dh_para)
{
    //TODO

    return DH_SUCCESS;
}
#endif


/* Function: DH check public key, it must be in [2, p-2], and pubkey^q = 1 mod p
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     p_minus_1 ------------------ input, p-1
 *     pubkey --------------------- input, public key
 * Return:
 *     PKE_SUCCESS(success); other(error)
 * Caution:
 *     1. please call dh_param_value_init() before calling this function.
 *     2. the input p_minus_1 and pubkey both occupy (dh_para->p_bits+31)/32 words
 */
uint32_t dh_check_public_key(const dh_para_st *dh_para, const uint32_t *p_minus_1, 
        const uint32_t *pubkey)
{
#if 0
    uint32_t tmp[DH_MAX_WORD_LEN];
#else
    uint32_t *tmp;
#endif
    uint32_t step_bytes, pWordLen = 0u, qWordLen = 0u;
    uint32_t ret;

    if((NULL == dh_para) || (NULL == p_minus_1) || (NULL == pubkey))
    {
        ret = DH_POINTER_NULL;
    }
    else
    {
        pWordLen = GET_WORD_LEN(dh_para->p_bits);
        qWordLen = GET_WORD_LEN(dh_para->q_bits);

        //make sure pubkey is in [2, p-2]
        if(get_valid_bits(pubkey, pWordLen) <= 1u)
        {
            ret = DH_INVALID_INPUT;
        }
        else if(uint32_BigNumCmp(pubkey, pWordLen, p_minus_1, pWordLen) >= 0)
        {
            ret = DH_INVALID_INPUT;
        }
        else
        {
            ret = pke_set_modulus_and_pre_monts(dh_para->p, dh_para->p_h, dh_para->p_bits);
        }
    }

    if(PKE_SUCCESS == ret)
    {
        step_bytes = pke_get_operand_bytes();
        tmp = (uint32_t *)(rPKE_A(1u,step_bytes));
        ret = pke_modexp_internal(dh_para->q, pubkey, tmp, pWordLen, qWordLen);
        if(PKE_SUCCESS == ret)
        {
            if(1u != Bigint_Check_1(tmp, pWordLen))
            {
                ret = DH_INVALID_INPUT;
            }
            else
            {}
        }
        else
        {}
    }

    return ret;
}


/* Function: DH generate public key from private key.
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     prikey --------------------- input, private key
 *     pubkey --------------------- output, public key
 * Return:
 *     DH_SUCCESS(success); other(error)
 * Caution:
 *     1. please call dh_param_value_init() before calling this function.
 *     2. the input prikey occupies (dh_para->q_bits+7)/8 bytes
 *        the output pubkey occupies (dh_para->p_bits+7)/8 bytes
 */
uint32_t dh_generate_pubkey_from_prikey(const dh_para_st *dh_para, const uint8_t *prikey, 
        uint8_t *pubkey)
{
    uint32_t tmp[DH_MAX_WORD_LEN];
    uint32_t x[DH_MAX_WORD_LEN];
#if 0
    uint32_t y[DH_MAX_WORD_LEN];
#else
    uint32_t *y;
#endif
    uint32_t *g;
    uint32_t pWordLen = 0u, qWordLen = 0u, gWordLen = 0u;
    uint32_t pByteLen = 0u, qByteLen = 0u;
    uint32_t tmpLen = 0u, ret;

    if((NULL == dh_para) || (NULL == prikey) || (NULL == pubkey))
    {
        ret = DH_POINTER_NULL;
    }
    else
    {
        pWordLen = GET_WORD_LEN(dh_para->p_bits);
        qWordLen = GET_WORD_LEN(dh_para->q_bits);
        gWordLen = GET_WORD_LEN(dh_para->g_bits);
        pByteLen = GET_BYTE_LEN(dh_para->p_bits);
        qByteLen = GET_BYTE_LEN(dh_para->q_bits);

        //get tmp = q-1
        uint32_copy(tmp, dh_para->q, qWordLen);
#ifdef SUPPORT_STATIC_ANALYSIS
        if(tmp[0u] > 0u)   //just for static analysis.
        {
#endif
            tmp[0u] -= 1u;
#ifdef SUPPORT_STATIC_ANALYSIS
        }
        else
        {}
#endif

#ifdef SUPPORT_STATIC_ANALYSIS
        if(qWordLen > 0u)   //just for static analysis.
        {
#endif
            x[qWordLen - 1u] = 0u;
            reverse_byte_array(prikey, (uint8_t *)x, qByteLen);
#ifdef SUPPORT_STATIC_ANALYSIS
        }
        else
        {}
#endif

        // x should be in [2, q-2]
        tmpLen = get_valid_bits(x, qWordLen);
        if(0u == tmpLen)
        {
            ret = DH_ZERO_ALL;
        }
        else if(1u == tmpLen)
        {
            ret = DH_VALUE_ONE;
        }
        else if(uint32_BigNumCmp(x, qWordLen, tmp, qWordLen) >= 0)
        {
            ret = DH_INTEGER_TOO_BIG;
        }
        else
        {
            ret = pke_set_modulus_and_pre_monts(dh_para->p, dh_para->p_h, dh_para->p_bits);
        }
    }

    if(PKE_SUCCESS == ret)
    {
        tmpLen = pke_get_operand_bytes();
        y = (uint32_t *)(rPKE_A(1u,tmpLen));

        if(gWordLen < pWordLen)
        {
            g = (uint32_t *)(rPKE_B(1u,tmpLen));
            uint32_copy(g, dh_para->g, gWordLen);
            uint32_clear(&g[gWordLen], pWordLen - gWordLen);
        }
        else
        {
            g = dh_para->g;
        }

        ret = pke_modexp_internal(x, g, y, pWordLen, qWordLen);
    }
    else
    {}

    if(PKE_SUCCESS == ret)
    {
        reverse_byte_array((uint8_t *)y, (uint8_t *)pubkey, pByteLen);
        ret = DH_SUCCESS;
    }
    else
    {}

    return ret;
}


/* Function: DH generate key pair.
 * Parameters:
 *     dh_para -------------------- input, dh_para_st struct pointer
 *     prikey --------------------- output, private key
 *     pubkey --------------------- output, public key
 * Return:
 *     DH_SUCCESS(success); other(error)
 * Caution:
 *     1. please call dh_param_value_init() before calling this function.
 *     2. the output prikey occupies (dh_para->q_bits+7)/8 bytes
 *        the output pubkey occupies (dh_para->p_bits+7)/8 bytes
 */
uint32_t dh_generate_key(const dh_para_st *dh_para, uint8_t *prikey, uint8_t *pubkey)
{
    uint32_t qByteLen;
    uint32_t tmpBitLen, ret;

    if((NULL == dh_para) || (NULL == prikey) || (NULL == pubkey))
    {
        ret = DH_POINTER_NULL;
    }
    else
    {
        qByteLen = GET_BYTE_LEN(dh_para->q_bits);

        do {
            ret = get_rand((uint8_t *)prikey, qByteLen);
            if(TRNG_SUCCESS != ret)
            {
                break;
            }
            else
            {}

            //make sure prikey has the same bit length as q
            tmpBitLen = (dh_para->q_bits)&7u;
            if(0u != tmpBitLen)
            {
                prikey[0u] &= (1u<<(tmpBitLen))-1u;
            }
            else
            {}

            ret = dh_generate_pubkey_from_prikey(dh_para, prikey, pubkey);

            // prikey should be in [2, q-2]
        } while ((DH_ZERO_ALL == ret) || (DH_VALUE_ONE == ret) || (DH_INTEGER_TOO_BIG == ret));
    }

    return ret;
}

#endif

