/**
 * \file asn1.h
 *
 * \brief Generic ASN.1 parsing
 */
/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 */
#ifndef MBEDTLS_ASN1_H
#define MBEDTLS_ASN1_H

typedef unsigned int size_t;

/**
 * \addtogroup asn1_module
 * \{
 */

/** Generic error */
#define MBEDTLS_ERR_ERROR_GENERIC_ERROR       -0x0001
/** This is a bug in the library */
#define MBEDTLS_ERR_ERROR_CORRUPTION_DETECTED -0x006E


/** Memory allocation failed. */
#define MBEDTLS_ERR_PK_ALLOC_FAILED        -0x3F80
/** Type mismatch, eg attempt to encrypt with an ECDSA key */
#define MBEDTLS_ERR_PK_TYPE_MISMATCH       -0x3F00
/** Bad input parameters to function. */
#define MBEDTLS_ERR_PK_BAD_INPUT_DATA      -0x3E80
/** Read/write of file failed. */
#define MBEDTLS_ERR_PK_FILE_IO_ERROR       -0x3E00
/** Unsupported key version */
#define MBEDTLS_ERR_PK_KEY_INVALID_VERSION -0x3D80
/** Invalid key tag or value. */
#define MBEDTLS_ERR_PK_KEY_INVALID_FORMAT  -0x3D00
/** Key algorithm is unsupported (only RSA and EC are supported). */
#define MBEDTLS_ERR_PK_UNKNOWN_PK_ALG      -0x3C80
/** Private key password can't be empty. */
#define MBEDTLS_ERR_PK_PASSWORD_REQUIRED   -0x3C00
/** Given private key password does not allow for correct decryption. */
#define MBEDTLS_ERR_PK_PASSWORD_MISMATCH   -0x3B80
/** The pubkey tag or value is invalid (only RSA and EC are supported). */
#define MBEDTLS_ERR_PK_INVALID_PUBKEY      -0x3B00
/** The algorithm tag or value is invalid. */
#define MBEDTLS_ERR_PK_INVALID_ALG         -0x3A80
/** Elliptic curve is unsupported (only NIST curves are supported). */
#define MBEDTLS_ERR_PK_UNKNOWN_NAMED_CURVE -0x3A00
/** Unavailable feature, e.g. RSA disabled for RSA key. */
#define MBEDTLS_ERR_PK_FEATURE_UNAVAILABLE -0x3980
/** The buffer contains a valid signature followed by more data. */
#define MBEDTLS_ERR_PK_SIG_LEN_MISMATCH    -0x3900
/** The output buffer is too small. */
#define MBEDTLS_ERR_PK_BUFFER_TOO_SMALL    -0x3880

/**
 * \name ASN1 Error codes
 * These error codes are combined with other error codes for
 * higher error granularity.
 * e.g. X.509 and PKCS #7 error codes
 * ASN1 is a standard to specify data structures.
 * \{
 */
/** Out of data when parsing an ASN1 data structure. */
#define MBEDTLS_ERR_ASN1_OUT_OF_DATA                      -0x0060
/** ASN1 tag was of an unexpected value. */
#define MBEDTLS_ERR_ASN1_UNEXPECTED_TAG                   -0x0062
/** Error when trying to determine the length or invalid length. */
#define MBEDTLS_ERR_ASN1_INVALID_LENGTH                   -0x0064
/** Actual length differs from expected length. */
#define MBEDTLS_ERR_ASN1_LENGTH_MISMATCH                  -0x0066
/** Data is invalid. */
#define MBEDTLS_ERR_ASN1_INVALID_DATA                     -0x0068
/** Memory allocation failed */
#define MBEDTLS_ERR_ASN1_ALLOC_FAILED                     -0x006A
/** Buffer too small when writing ASN.1 data structure. */
#define MBEDTLS_ERR_ASN1_BUF_TOO_SMALL                    -0x006C

/** \} name ASN1 Error codes */

/**
 * \name DER constants
 * These constants comply with the DER encoded ASN.1 type tags.
 * DER encoding uses hexadecimal representation.
 * An example DER sequence is:\n
 * - 0x02 -- tag indicating INTEGER
 * - 0x01 -- length in octets
 * - 0x05 -- value
 * Such sequences are typically read into \c ::mbedtls_x509_buf.
 * \{
 */
#define MBEDTLS_ASN1_BOOLEAN                 0x01
#define MBEDTLS_ASN1_INTEGER                 0x02
#define MBEDTLS_ASN1_BIT_STRING              0x03
#define MBEDTLS_ASN1_OCTET_STRING            0x04
#define MBEDTLS_ASN1_NULL                    0x05
#define MBEDTLS_ASN1_OID                     0x06
#define MBEDTLS_ASN1_ENUMERATED              0x0A
#define MBEDTLS_ASN1_UTF8_STRING             0x0C
#define MBEDTLS_ASN1_SEQUENCE                0x10
#define MBEDTLS_ASN1_SET                     0x11
#define MBEDTLS_ASN1_PRINTABLE_STRING        0x13
#define MBEDTLS_ASN1_T61_STRING              0x14
#define MBEDTLS_ASN1_IA5_STRING              0x16
#define MBEDTLS_ASN1_UTC_TIME                0x17
#define MBEDTLS_ASN1_GENERALIZED_TIME        0x18
#define MBEDTLS_ASN1_UNIVERSAL_STRING        0x1C
#define MBEDTLS_ASN1_BMP_STRING              0x1E
#define MBEDTLS_ASN1_PRIMITIVE               0x00
#define MBEDTLS_ASN1_CONSTRUCTED             0x20
#define MBEDTLS_ASN1_CONTEXT_SPECIFIC        0x80

/* Slightly smaller way to check if tag is a string tag
 * compared to canonical implementation. */
#define MBEDTLS_ASN1_IS_STRING_TAG(tag)                                \
    ((unsigned int) (tag) < 32u && (                                   \
         ((1u << (tag)) & ((1u << MBEDTLS_ASN1_BMP_STRING)       |     \
                           (1u << MBEDTLS_ASN1_UTF8_STRING)      |     \
                           (1u << MBEDTLS_ASN1_T61_STRING)       |     \
                           (1u << MBEDTLS_ASN1_IA5_STRING)       |     \
                           (1u << MBEDTLS_ASN1_UNIVERSAL_STRING) |     \
                           (1u << MBEDTLS_ASN1_PRINTABLE_STRING))) != 0))

/*
 * Bit masks for each of the components of an ASN.1 tag as specified in
 * ITU X.690 (08/2015), section 8.1 "General rules for encoding",
 * paragraph 8.1.2.2:
 *
 * Bit  8     7   6   5          1
 *     +-------+-----+------------+
 *     | Class | P/C | Tag number |
 *     +-------+-----+------------+
 */
#define MBEDTLS_ASN1_TAG_CLASS_MASK          0xC0
#define MBEDTLS_ASN1_TAG_PC_MASK             0x20
#define MBEDTLS_ASN1_TAG_VALUE_MASK          0x1F

/** \} name DER constants */

/** Returns the size of the binary string, without the trailing \\0 */
#define MBEDTLS_OID_SIZE(x) (sizeof(x) - 1)

/**
 * Compares an mbedtls_asn1_buf structure to a reference OID.
 *
 * Only works for 'defined' oid_str values (MBEDTLS_OID_HMAC_SHA1), you cannot use a
 * 'unsigned char *oid' here!
 */
#define MBEDTLS_OID_CMP(oid_str, oid_buf)                                   \
    ((MBEDTLS_OID_SIZE(oid_str) != (oid_buf)->len) ||                \
     memcmp((oid_str), (oid_buf)->p, (oid_buf)->len) != 0)

#define MBEDTLS_OID_CMP_RAW(oid_str, oid_buf, oid_buf_len)              \
    ((MBEDTLS_OID_SIZE(oid_str) != (oid_buf_len)) ||             \
     memcmp((oid_str), (oid_buf), (oid_buf_len)) != 0)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \name Functions to parse ASN.1 data structures
 * \{
 */

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef struct mbedtls_asn1_buf {
    int tag;                /**< ASN1 type, e.g. MBEDTLS_ASN1_UTF8_STRING. */
    size_t len;             /**< ASN1 length, in octets. */
    unsigned char *p;       /**< ASN1 data, e.g. in ASCII. */
} mbedtls_asn1_buf;

/**
 * Container for ASN1 bit strings.
 */
typedef struct mbedtls_asn1_bitstring {
    size_t len;                 /**< ASN1 length, in octets. */
    unsigned char unused_bits;  /**< Number of unused bits at the end of the string */
    unsigned char *p;           /**< Raw ASN1 data for the bit string */
}
mbedtls_asn1_bitstring;

/**
 * Container for a sequence of ASN.1 items
 */
typedef struct mbedtls_asn1_sequence {
    mbedtls_asn1_buf buf;                   /**< Buffer containing the given ASN.1 item. */

    /** The next entry in the sequence.
     *
     * The details of memory management for sequences are not documented and
     * may change in future versions. Set this field to \p NULL when
     * initializing a structure, and do not modify it except via Mbed TLS
     * library functions.
     */
    struct mbedtls_asn1_sequence *next;
}
mbedtls_asn1_sequence;

/**
 * Container for a sequence or list of 'named' ASN.1 data items
 */
typedef struct mbedtls_asn1_named_data {
    mbedtls_asn1_buf oid;                   /**< The object identifier. */
    mbedtls_asn1_buf val;                   /**< The named value. */

    /** The next entry in the sequence.
     *
     * The details of memory management for named data sequences are not
     * documented and may change in future versions. Set this field to \p NULL
     * when initializing a structure, and do not modify it except via Mbed TLS
     * library functions.
     */
    struct mbedtls_asn1_named_data *next;

    /** Merge next item into the current one?
     *
     * This field exists for the sake of Mbed TLS's X.509 certificate parsing
     * code and may change in future versions of the library.
     */
    unsigned char next_merged;
}
mbedtls_asn1_named_data;

/**
 * \brief       Get the length of an ASN.1 element.
 *              Updates the pointer to immediately behind the length.
 *
 * \param p     On entry, \c *p points to the first byte of the length,
 *              i.e. immediately after the tag.
 *              On successful completion, \c *p points to the first byte
 *              after the length, i.e. the first byte of the content.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param len   On successful completion, \c *len contains the length
 *              read from the ASN.1 input.
 *
 * \return      0 if successful.
 * \return      #MBEDTLS_ERR_ASN1_OUT_OF_DATA if the ASN.1 element
 *              would end beyond \p end.
 * \return      #MBEDTLS_ERR_ASN1_INVALID_LENGTH if the length is unparsable.
 */
int mbedtls_asn1_get_len(unsigned char **p,
                         const unsigned char *end,
                         size_t *len);

/**
 * \brief       Get the tag and length of the element.
 *              Check for the requested tag.
 *              Updates the pointer to immediately behind the tag and length.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p points to the first byte
 *              after the length, i.e. the first byte of the content.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param len   On successful completion, \c *len contains the length
 *              read from the ASN.1 input.
 * \param tag   The expected tag.
 *
 * \return      0 if successful.
 * \return      #MBEDTLS_ERR_ASN1_UNEXPECTED_TAG if the data does not start
 *              with the requested tag.
 * \return      #MBEDTLS_ERR_ASN1_OUT_OF_DATA if the ASN.1 element
 *              would end beyond \p end.
 * \return      #MBEDTLS_ERR_ASN1_INVALID_LENGTH if the length is unparsable.
 */
int mbedtls_asn1_get_tag(unsigned char **p,
                         const unsigned char *end,
                         size_t *len, int tag);


/**
 * \brief       Retrieve a boolean ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p points to the first byte
 *              beyond the ASN.1 element.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param val   On success, the parsed value (\c 0 or \c 1).
 *
 * \return      0 if successful.
 * \return      An ASN.1 error code if the input does not start with
 *              a valid ASN.1 BOOLEAN.
 */
int mbedtls_asn1_get_bool(unsigned char **p,
                          const unsigned char *end,
                          int *val);

/**
 * \brief       Retrieve a bitstring ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p is equal to \p end.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param bs    On success, ::mbedtls_asn1_bitstring information about
 *              the parsed value.
 *
 * \return      0 if successful.
 * \return      #MBEDTLS_ERR_ASN1_LENGTH_MISMATCH if the input contains
 *              extra data after a valid BIT STRING.
 * \return      An ASN.1 error code if the input does not start with
 *              a valid ASN.1 BIT STRING.
 */
int mbedtls_asn1_get_bitstring(unsigned char **p, const unsigned char *end,
                               mbedtls_asn1_bitstring *bs);

/**
 * \brief       Retrieve a bitstring ASN.1 tag without unused bits and its
 *              value.
 *              Updates the pointer to the beginning of the bit/octet string.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p points to the first byte
 *              of the content of the BIT STRING.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param len   On success, \c *len is the length of the content in bytes.
 *
 * \return      0 if successful.
 * \return      #MBEDTLS_ERR_ASN1_INVALID_DATA if the input starts with
 *              a valid BIT STRING with a nonzero number of unused bits.
 * \return      An ASN.1 error code if the input does not start with
 *              a valid ASN.1 BIT STRING.
 */
int mbedtls_asn1_get_bitstring_null(unsigned char **p,
                                    const unsigned char *end,
                                    size_t *len);


/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p points to the first byte
 *              beyond the AlgorithmIdentifier element.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param alg   The buffer to receive the OID.
 * \param params The buffer to receive the parameters.
 *              This is zeroized if there are no parameters.
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
int mbedtls_asn1_get_alg(unsigned char **p,
                         const unsigned char *end,
                         mbedtls_asn1_buf *alg, mbedtls_asn1_buf *params);

/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence with NULL or no
 *              params.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     On entry, \c *p points to the start of the ASN.1 element.
 *              On successful completion, \c *p points to the first byte
 *              beyond the AlgorithmIdentifier element.
 *              On error, the value of \c *p is undefined.
 * \param end   End of data.
 * \param alg   The buffer to receive the OID.
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
int mbedtls_asn1_get_alg_null(unsigned char **p,
                              const unsigned char *end,
                              mbedtls_asn1_buf *alg);


/** \} name Functions to parse ASN.1 data structures */
/** \} addtogroup asn1_module */



#ifdef __cplusplus
}
#endif

#endif /* asn1.h */
