
#include "../../crypto_include/pke/rsa.h"
#include "../../crypto_include/pke/rsa_u8.h"
#include "../../crypto_include/crypto_common/utility.h"



/* function: out = a^e mod n(for high level use, operands are all U8 big-endian big number)
 * parameters:
 *     a -------------------------- input, uint8_t big integer a, base number, make sure a < n
 *     e -------------------------- input, uint8_t big integer e, exeponent, make sure e < n
 *     n -------------------------- input, uint8_t big integer n, modulus, make sure n is odd
 *     out ------------------------ output, out = a^e mod n
 *     eBitLen  ------------------- input, real bit length of uint8_t big integer e
 *     nBitLen  ------------------- input, real bit length of uint8_t big integer n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. this is for high level application or protocol to use RSA mod exponent directly. all
 *        operands of this API are U8 big-endian big number.
 *     2. modulus must be odd
 *     3. please make sure exp_bitLen <= mod_bitLen <= OPERAND_MAX_BIT_LEN
 *     4. a, n, and out have the same word length:((nBitLen+31)>>5); and e word length is (eBitLen+31)>>5
 */
uint32_t RSA_ModExp_U8(const uint8_t *a, const uint8_t *e, const uint8_t *n, 
        uint8_t *out, uint32_t eBitLen, uint32_t nBitLen)
{
#if 0
	uint32_t a_u32[RSA_MAX_WORD_LEN];
	uint32_t e_u32[RSA_MAX_WORD_LEN];
	uint32_t n_u32[RSA_MAX_WORD_LEN];
	uint32_t eByteLen = GET_BYTE_LEN(eBitLen);
	uint32_t eWordLen = GET_WORD_LEN(eBitLen);
	uint32_t nByteLen = GET_BYTE_LEN(nBitLen);
	uint32_t nWordLen = GET_WORD_LEN(nBitLen);
	uint32_t ret;

	if(0U != (eByteLen & 3U))
	{
		e_u32[eWordLen - 1U] = 0U;
	}
	else
	{}

	if(0U != (nByteLen & 3U))
	{
		a_u32[nWordLen - 1U] = 0U;
		n_u32[nWordLen - 1U] = 0U;
	}
	else
	{}

	reverse_byte_array(e, (uint8_t *)e_u32, eByteLen);
	reverse_byte_array(a, (uint8_t *)a_u32, nByteLen);
	reverse_byte_array(n, (uint8_t *)n_u32, nByteLen);

	ret = RSA_ModExp(a_u32, e_u32, n_u32, a_u32, eBitLen, nBitLen);
	if(RSA_SUCCESS == ret)
	{
		reverse_byte_array((uint8_t *)a_u32, out, nByteLen);
	}
	else
	{}

	return ret;
#else
    return pke_modexp_U8(n, e, a, out, nBitLen, eBitLen, 1U);
#endif
}


/* function: out = a ^ d mod n, here d represents RSA CRT private key (p,q,dp,dq,u),
 *           for high level use, operands are all U8 big-endian big number.
 * parameters:
 *     a -------------------------- input, uint8_t big integer a, base number, make sure a < n=pq
 *     p -------------------------- input, uint8_t big integer p, prime number, one part of private key (p,q,dp,dq,u)
 *     q -------------------------- input, uint8_t big integer q, prime number, one part of private key (p,q,dp,dq,u)
 *     dp ------------------------- input, uint8_t big integer dp = e^(-1) mod (p-1), one part of private key (p,q,dp,dq,u)
 *     dq ------------------------- input, uint8_t big integer dq = e^(-1) mod (q-1), one part of private key (p,q,dp,dq,u)
 *     u -------------------------- input, uint8_t big integer u = q^(-1) mod p, one part of private key (p,q,dp,dq,u)
 *     out ------------------------ output, out = a^d mod n
 *     nBitLen  ------------------- input, real bit length of uint8_t big integer n=pq
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. this is for high level application or protocol to use RSA mod exponent directly. all
 *        operands of this API are U8 big-endian big number.
 *     2. modulus must be odd
 *     3. please make sure exp_bitLen <= mod_bitLen <= OPERAND_MAX_BIT_LEN
 *     4. a, n and out have the same word length:((nBitLen+31)>>5); and p,p_h,q,q_h,dp,dq,u
 *        have the same word length:((nBitLen/2+31)>>5)
 */
uint32_t RSA_CRTModExp_U8(const uint8_t *a, const uint8_t *p, const uint8_t *q, 
        const uint8_t *dp, const uint8_t*dq, const uint8_t *u, uint8_t *out, 
        uint32_t nBitLen)
{
	uint32_t a_u32[RSA_MAX_WORD_LEN];
	uint32_t p_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t q_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t dp_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t dq_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t u_u32[RSA_MAX_WORD_LEN>>1];

	uint32_t pByteLen = GET_BYTE_LEN(nBitLen>>1);
	uint32_t pWordLen = GET_WORD_LEN(nBitLen>>1);
	uint32_t nByteLen = GET_BYTE_LEN(nBitLen);
	uint32_t nWordLen = GET_WORD_LEN(nBitLen);
	uint32_t ret;

	if(0U != (pByteLen & 3U))
	{
		p_u32[pWordLen - 1U]  = 0U;
		q_u32[pWordLen - 1U]  = 0U;
		dp_u32[pWordLen - 1U] = 0U;
		dq_u32[pWordLen - 1U] = 0U;
		u_u32[pWordLen - 1U]  = 0U;
	}
	else
	{}

	if(0U != (nByteLen & 3U))
	{
		a_u32[nWordLen - 1U] = 0U;
	}
	else
	{}

	reverse_byte_array(p, (uint8_t *)p_u32, pByteLen);
	reverse_byte_array(q, (uint8_t *)q_u32, pByteLen);
	reverse_byte_array(dp, (uint8_t *)dp_u32, pByteLen);
	reverse_byte_array(dq, (uint8_t *)dq_u32, pByteLen);
	reverse_byte_array(u, (uint8_t *)u_u32, pByteLen);
	reverse_byte_array(a, (uint8_t *)a_u32, nByteLen);

	ret = RSA_CRTModExp(a_u32, p_u32, q_u32, dp_u32, dq_u32, u_u32, a_u32, nBitLen);
	if(RSA_SUCCESS == ret)
	{
		reverse_byte_array((uint8_t *)a_u32, out, nByteLen);
	}
	else
	{}

	return ret;
}


/* function: generate RSA key (e,d,n), for high level use, operands are all U8 big-endian big number.
 * parameters:
 *     e -------------------------- output, uint8_t big integer, RSA public key e
 *     d -------------------------- output, uint8_t big integer, RSA private key d
 *     n -------------------------- output, uint8_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * return: RSA_SUCCESS(success), other(error)
 * caution:
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t RSA_GetKey_U8(uint8_t *e, uint8_t *d, uint8_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
	uint32_t e_u32[RSA_MAX_WORD_LEN];
	uint32_t d_u32[RSA_MAX_WORD_LEN];
	uint32_t n_u32[RSA_MAX_WORD_LEN];
	uint32_t eByteLen = GET_BYTE_LEN(eBitLen);
	uint32_t nByteLen = GET_BYTE_LEN(nBitLen);
	uint32_t ret;

	ret = RSA_GetKey(e_u32, d_u32, n_u32, eBitLen, nBitLen);
	if(RSA_SUCCESS == ret)
	{
		reverse_byte_array((uint8_t *)e_u32, e, eByteLen);
		reverse_byte_array((uint8_t *)d_u32, d, nByteLen);
		reverse_byte_array((uint8_t *)n_u32, n, nByteLen);
	}
	else
	{}

	return ret;
}


/* Function: generate RSA-CRT key (e,p,q,dp,dq,u,n), for high level use, operands are all U8 big-endian big number
 * Parameters:
 *     e -------------------------- output, uint8_t big integer, RSA public key e
 *     p -------------------------- output, uint8_t big integer, RSA private key p
 *     q -------------------------- output, uint8_t big integer, RSA private key q
 *     dp-------------------------- output, uint8_t big integer, RSA private key dp
 *     dq-------------------------- output, uint8_t big integer, RSA private key dq
 *     u -------------------------- output, uint8_t big integer, RSA private key u = q^(-1) mod p
 *     n -------------------------- output, uint8_t big integer, RSA public module n
 *     eBitLen  ------------------- input, real bit length of e
 *     nBitLen  ------------------- input, real bit length of n
 * Return: RSA_SUCCESS(success), other(error)
 * Caution:
 *     1. nBitLen can not be odd
 *     2. eBitLen must be greater than 1, and less than or equal to nBitLen
 *     3. if eBitLen is 2,5,17, here makes e as 3,17,65537 respectively, otherwise e is random
 */
uint32_t RSA_GetCRTKey_U8(uint8_t *e, uint8_t *p, uint8_t *q, uint8_t *dp, uint8_t *dq, uint8_t *u,
		uint8_t *n, uint32_t eBitLen, uint32_t nBitLen)
{
	uint32_t e_u32[RSA_MAX_WORD_LEN];
	uint32_t p_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t q_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t dp_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t dq_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t u_u32[RSA_MAX_WORD_LEN>>1];
	uint32_t n_u32[RSA_MAX_WORD_LEN];

	uint32_t eByteLen = GET_BYTE_LEN(eBitLen);
	uint32_t pByteLen = GET_BYTE_LEN(nBitLen>>1);
	uint32_t nByteLen = GET_BYTE_LEN(nBitLen);
	uint32_t ret;

	ret = RSA_GetCRTKey(e_u32, p_u32, q_u32, dp_u32, dq_u32, u_u32, n_u32, eBitLen, nBitLen);
	if(RSA_SUCCESS == ret)
	{
		reverse_byte_array((uint8_t *)e_u32, e, eByteLen);
		reverse_byte_array((uint8_t *)p_u32, p, pByteLen);
		reverse_byte_array((uint8_t *)q_u32, q, pByteLen);
		reverse_byte_array((uint8_t *)dp_u32, dp, pByteLen);
		reverse_byte_array((uint8_t *)dq_u32, dq, pByteLen);
		reverse_byte_array((uint8_t *)u_u32, u, pByteLen);
		reverse_byte_array((uint8_t *)n_u32, n, nByteLen);
	}
	else
	{}

	return ret;
}

