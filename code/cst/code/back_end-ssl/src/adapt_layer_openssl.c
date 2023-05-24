/*===========================================================================*/
/**
    @file    adapt_layer_openssl.c

    @brief   Implements Code Signing Tool's Adaptation Layer API for the
             Freescale reference Code Signing Tool.  This file may be
             replaced in implementations using a Hardware Security Module
             or a client/server based infrastructure.

@verbatim
=============================================================================

              Freescale Semiconductor
        (c) Freescale Semiconductor, Inc. 2011-2015. All rights reserved.
        Copyright 2018, 2020, 2022 NXP

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=============================================================================
@endverbatim */

/*===========================================================================
                                INCLUDE FILES
=============================================================================*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include "ssl_wrapper.h"
#include <string.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/cms.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <openssl/rsa.h>
#include "adapt_layer.h"
#include "ssl_backend.h"
#include "openssl_helper.h"
#include "pkey.h"
#include "csf.h"
#include <sys/stat.h>
#if (defined _WIN32 || defined __CYGWIN__) && defined USE_APPLINK
#include <openssl/applink.c>
#endif

#define ENABLE_VERIFY 0
#define AUTOX_SIGN 1

#if AUTOX_SIGN
#include "autox_sign_with_hsm.h"

#define SIGN_SERVER_SIGNED_CSF_OUT_NAME "signed_file_csf.bin"
#define SIGN_SERVER_SIGNED_IMAGE_OUT_NAME "signed_file_image.bin"
#define SIGN_SERVER_SIGNED_CSF_IN_NAME "to_sign_file_csf.bin"
#define SIGN_SERVER_SIGNED_IMAGE_IN_NAME "to_sign_file_image.bin"
#define SIGN_SERVER_SSL_CERT "sign_server.crt"
#define SIGN_SERVER_SSL_KEY "sign_server.key"
#define SIGN_SERVER_ROOT_CA "root_ca.crt"
#define SIGN_SERVER_API_URL "https://dev.xsec-gateway.autox.tech/v1/signServer/cms/sign?type=rt117x"
#define SIGN_SERVER_CA_URL "https://dev.ca.autox.tech/ejbca/publicweb/webdist/certdist?cmd=cachain&caid=-238079556&format=pem"

int32_t autox_gen_sig_data_cms(const char* in_file,
                               uint8_t* sig_buf,
                               size_t *sig_buf_bytes,
                               char *sig_out_file);

static char autox_signed_file_name[1024] = {0};

#define LOG_DEBUG printf("[AUTOX_SIGN] "); printf

#endif /* AUTOX_SIGN */
static int32_t autox_write_binary_all(const char *filename, uint8_t *buffer, size_t o_len);
/*===========================================================================
                                 LOCAL MACROS
=============================================================================*/
#define MAX_CMS_DATA                4096   /**< Max bytes in CMS_ContentInfo */
#define MAX_LINE_CHARS              1024   /**< Max. chars in output line    */

/*===========================================================================
                  LOCAL TYPEDEFS (STRUCTURES, UNIONS, ENUMS)
=============================================================================*/

/*===========================================================================
                          LOCAL FUNCTION PROTOTYPES
=============================================================================*/
#if ENABLE_VERIFY

int32_t
verify_sig_data_cms(const char *in_file,
                    const char *cert_ca,
                    const char *cert_signer,
                    const char *sig_file,
                    hash_alg_t hash_alg);

#define DUMP_WIDTH 16
static void bio_dump(const char *s, int len)
{
    char buf[160+1] = {0};
    char tmp[20] = {0};
    unsigned char ch;
    int32_t i, j, rows;

#ifdef TRUNCATE
    int32_t trunc = 0;
    for(; (len > 0) && ((s[len-1] == ' ') || (s[len-1] == '\0')); len--)
        trunc++;
#endif

    rows = (len / DUMP_WIDTH);
    if ((rows * DUMP_WIDTH) < len)
        rows ++;
    for (i = 0; i < rows; i ++) {
        /* start with empty string */
        buf[0] = '\0';
        sprintf(tmp, "%04x - ", i * DUMP_WIDTH);
        strcpy(buf, tmp);
        for (j = 0; j < DUMP_WIDTH; j ++) {
            if (((i * DUMP_WIDTH) + j) >= len) {
                strcat(buf,"   ");
            } else {
                ch = ((unsigned char)*(s + i * DUMP_WIDTH + j)) & 0xff;
                sprintf(tmp, "%02x%c" , ch, j == 7 ? '-':' ');
                strcat(buf, tmp);
            }
        }
        strcat(buf, "  ");
        for(j = 0;j < DUMP_WIDTH;j ++) {
            if (((i * DUMP_WIDTH) + j) >= len)
                break;
            ch = ((unsigned char)*(s + i * DUMP_WIDTH + j)) & 0xff;
            sprintf(tmp, "%c", ((ch >= ' ')&&(ch <= '~')) ? ch : '.');
            strcat(buf, tmp);
        }
        strcat(buf, "\n");
        printf("%s", buf);
    }
#ifdef TRUNCATE
    if (trunc > 0) {
        sprintf(buf,"%04x - <SPACES/NULS>\n",len+trunc);
        printf("%s", buf);
    }
#endif
}

void utils_print_bio_array(uint8_t *buffer, size_t len, char* msg)
{
    printf("\n");
    printf("%s: the len is %zu\n", msg, len);
    bio_dump((const char *)buffer, len);
    printf("\n");
}

#endif /* ENABLE_VERIFY */

/** Converts hash_alg to an equivalent NID value for OpenSSL
 *
 * @param[in] hash_alg Hash digest algorithm from #hash_alg_t
 *
 * @pre hash_alg is a valid value from #hash_alg_t
 *
 * @returns Openssl NID value corresponding to a valid value for @a hash_alg,
 *          NID_undef otherwise.
 */
static int32_t
get_NID(hash_alg_t hash_alg);

/** Generate raw PKCS#1 Signature Data
 *
 * Generates a raw PKCS#1 v1.5 signature for the given data file, signer
 * certificate, and hash algorithm. The signature data is returned in
 * a buffer provided by caller.
 *
 * @param[in] in_file string containing path to file with data to sign
 *
 * @param[in] key_file string containing path to signing key
 *
 * @param[in] hash_alg hash algorithm from #hash_alg_t
 *
 * @param[out] sig_buf signature data buffer
 *
 * @param[in,out] sig_buf_bytes On input, contains size of @a sig_buf in bytes,
 *                              On output, contains size of signature in bytes.
 *
 * @pre @a in_file, @a cert_file, @a key_file, @a sig_buf and @a sig_buf_bytes
 *         must not be NULL.
 *
 * @post On success @a sig_buf is updated to hold the resulting signature and
 *       @a sig_buf_bytes is updates to hold the length of the signature in
 *       bytes
 *
 * @retval #CAL_SUCCESS API completed its task successfully
 *
 * @retval #CAL_CRYPTO_API_ERROR An Openssl related error has occured
 */
static int32_t
gen_sig_data_raw(const char *in_file,
                 const char *key_file,
                 hash_alg_t hash_alg,
                 uint8_t *sig_buf,
                 int32_t *sig_buf_bytes);

/** Generate CMS Signature Data
 *
 * Generates a CMS signature for the given data file, signer certificate, and
 * hash algorithm. The signature data is returned in a buffer provided by
 * caller.  Note that sign_data cannot be used here since that function
 * requires an input buffer as an argument.  For large files it becomes
 * unreasonable to allocate a contigous block of memory.
 *
 * @param[in] in_file string containing path to file with data to sign
 *
 * @param[in] cert_file string constaining path to signer certificate
 *
 * @param[in] hash_alg hash algorithm from #hash_alg_t
 *
 * @param[out] sig_buf signature data buffer
 *
 * @param[in,out] sig_buf_bytes On input, contains size of @a sig_buf in bytes,
 *                              On output, contains size of signature in bytes.
 *
 * @pre @a in_file, @a cert_file, @a key_file, @a sig_buf and @a sig_buf_bytes
 *         must not be NULL.
 *
 * @post On success @a sig_buf is updated to hold the resulting signature and
 *       @a sig_buf_bytes is updates to hold the length of the signature in
 *       bytes
 *
 * @retval #CAL_SUCCESS API completed its task successfully
 *
 * @retval #CAL_INVALID_ARGUMENT One of the input arguments is invalid
 *
 * @retval #CAL_CRYPTO_API_ERROR An Openssl related error has occured
 */
#if !AUTOX_SIGN
static int32_t
gen_sig_data_cms(const char *in_file,
                 const char *cert_file,
                 const char *key_file,
                 hash_alg_t hash_alg,
                 uint8_t *sig_buf,
                 size_t *sig_buf_bytes);
#endif /* !AUTOX_SIGN */

/** Copies CMS Content Info with encrypted or signature data to buffer
 *
 * @param[in] cms CMS Content Info
 *
 * @param[in] bio_in input bio
 *
 * @param[out] data_buffer address to data buffer
 *
 * @param[in] data_buffer_size max size, [out] return size
 *
 * @param[in] flags CMS Flags
 *
 * @returns CAL_SUCCESS upon success
 *
 * @returns CAL_CRYPTO_API_ERROR when openssl BIO API fail
 */
int32_t cms_to_buf(CMS_ContentInfo *cms, BIO * bio_in, uint8_t * data_buffer,
                            size_t * data_buffer_size, int32_t flags);

/** generate_dek_key
 *
 * Uses openssl API to generate a random 128 bit AES key
 *
 * @param[out] key buffer to store the key data
 *
 * @param[in] len length of the key to generate
 *
 * @post if successful the random bytes are placed into output buffer
 *
 * @pre  #openssl_initialize has been called previously
 *
 * @returns if successful function returns location CAL_SUCCESS.
 */
int32_t generate_dek_key(uint8_t * key, int32_t len);

/**  write_plaintext_dek_key
 *
 * Writes the provide DEK to the give path. It will be encrypted
 * under the certificate file if provided.
 *
 * @param[in] key input key data
 *
 * @param[in] key_bytes length of the input key
 *
 * @param[in] cert_file  certificate to encrypt the DEK
 *
 * @param[in] enc_file  destination file
 *
 * @post if successful the dek is written to the file
 *
 * @returns if successful function returns location CAL_SUCCESS.
 */
int32_t write_plaintext_dek_key(uint8_t * key, size_t key_bytes,
                        const char * cert_file, const char * enc_file);

/** encrypt_dek_key
 *
 * Uses openssl API to encrypt the key. Saves the encrypted structure to a file
 *
 * @param[in] key input key data
 *
 * @param[in] key_bytes length of the input key
 *
 * @param[in] cert filename of the RSA certificate, dek will be encrypted with
 *
 * @param[in] file encrypted data saved in the file
 *
 * @post if successful the file is created with the encrypted data
 *
 * @pre  #openssl_initialize has been called previously
 *
 * @returns if successful function returns location CAL_SUCCESS.
 */
int32_t encrypt_dek_key(uint8_t * key, size_t key_bytes,
                const char * cert_file, const char * enc_file);

/** Display error message
 *
 * Displays error message to STDERR
 *
 * @param[in] err Error string to display
 *
 * @pre  @a err is not NULL
 *
 * @post None
 */
static void
display_error(const char *err);

/*===========================================================================
                               GLOBAL VARIABLES
=============================================================================*/

/*===========================================================================
                               LOCAL FUNCTIONS
=============================================================================*/
/*--------------------------
  get_NID
---------------------------*/
int32_t
get_NID(hash_alg_t hash_alg)
{
    return OBJ_txt2nid(get_digest_name(hash_alg));
}

/*--------------------------
  gen_sig_data_raw
---------------------------*/
int32_t
gen_sig_data_raw(const char *in_file,
                 const char *key_file,
                 hash_alg_t hash_alg,
                 uint8_t *sig_buf,
                 int32_t *sig_buf_bytes)
{
    EVP_PKEY *key = NULL; /**< Ptr to read key data */
    RSA *rsa = NULL; /**< Ptr to rsa of key data */
    uint8_t *rsa_in = NULL; /**< Mem ptr for hash data of in_file */
    uint8_t *rsa_out = NULL; /**< Mem ptr for encrypted data */
    int32_t rsa_inbytes; /**< Holds the length of rsa_in buf */
    unsigned int rsa_outbytes = 0; /**< Holds the length of rsa_out buf */
    int32_t key_bytes; /**< Size of key data */
    int32_t hash_nid; /**< hash id needed for RSA_sign() */
    /** Array to hold error string */
    char err_str[MAX_ERR_STR_BYTES];
    /**< Holds the return error value */
    int32_t err_value = CAL_CRYPTO_API_ERROR;

    do
    {
        /* Read key */
        key = read_private_key(key_file,
                           (pem_password_cb *)get_passcode_to_key_file,
                           key_file);
        if (!key) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open key file %s", key_file);
            display_error(err_str);
            break;
        }

        rsa = EVP_PKEY_get1_RSA(key);
        EVP_PKEY_free(key);

        if (!rsa) {
            display_error("Unable to extract RSA key for RAW PKCS#1 signature");
            break;
        }

        rsa_inbytes = HASH_BYTES_MAX;
        rsa_in = OPENSSL_malloc(HASH_BYTES_MAX);
        key_bytes = RSA_size(rsa);
        rsa_out = OPENSSL_malloc(key_bytes);

        /* Generate hash data of data from in_file */
        err_value = calculate_hash(in_file, hash_alg, rsa_in, &rsa_inbytes);
        if (err_value != CAL_SUCCESS) {
            break;
        }

        /* Compute signature.  Note: RSA_sign() adds the appropriate DER
         * encoded prefix internally.
         */
        hash_nid = get_NID(hash_alg);
        if (!RSA_sign(hash_nid, rsa_in,
                      rsa_inbytes, rsa_out,
                      (unsigned int *)&rsa_outbytes, rsa)) {
            err_value = CAL_CRYPTO_API_ERROR;
            display_error("Unable to generate signature");
            break;
        }
        else {
            err_value = CAL_SUCCESS;
        }

        /* Copy signature to sig_buf and update sig_buf_bytes */
        *sig_buf_bytes = rsa_outbytes;
        memcpy(sig_buf, rsa_out, rsa_outbytes);
    } while(0);

    if (err_value != CAL_SUCCESS) {
        ERR_print_errors_fp(stderr);
    }

    if (rsa) RSA_free(rsa);
    if (rsa_in) OPENSSL_free(rsa_in);
    if (rsa_out) OPENSSL_free(rsa_out);
    return err_value;
}

/*--------------------------
  gen_sig_data_pss
---------------------------*/
int32_t
gen_sig_data_pss(const char *in_file,
                 const char *key_file,
                 hash_alg_t hash_alg,
                 uint8_t *sig_buf,
                 int32_t *sig_buf_bytes)
{
    const EVP_MD *md = EVP_get_digestbyname(get_digest_name(hash_alg));
    EVP_PKEY *key = NULL; /**< Ptr to read key data */
    RSA *rsa = NULL; /**< Ptr to rsa of key data */
    uint8_t *hash_msg = NULL; /**< Mem ptr for hash data of in_file */
    int32_t hash_msg_size; /**< Holds the length of hash_msg buf */
    uint8_t *enc_sig = NULL; /**< Mem ptr for encrypted signature */
    size_t enc_sig_size; /**< Holds the length of encrypted signature buf */
    uint8_t *em = NULL; /**< Mem ptr for encoded message */
    size_t em_size; /**< Holds the length of encoded message */
    char err_str[MAX_ERR_STR_BYTES]; /**< Array to hold error string */
    int32_t err_value = CAL_CRYPTO_API_ERROR; /**< Holds the return error value */

    do
    {
        /* Read key */
        key = read_private_key(key_file,
                           (pem_password_cb *)get_passcode_to_key_file,
                           key_file);
        if (!key) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open key file %s", key_file);
            display_error(err_str);
            break;
        }

        /* Get RSA key */
        rsa = EVP_PKEY_get1_RSA(key);
        EVP_PKEY_free(key);

        hash_msg_size = HASH_BYTES_MAX;
        hash_msg = OPENSSL_malloc(HASH_BYTES_MAX);
        if (hash_msg == NULL) {
          err_value = CAL_CRYPTO_API_ERROR;
          break;
        }

        /* Generate hash data of data from in_file */
        err_value = calculate_hash(in_file, hash_alg, hash_msg, &hash_msg_size);
        if (err_value != CAL_SUCCESS) {
            break;
        }

        em_size = RSA_size(rsa);
        em = OPENSSL_malloc(em_size);
        if (em == NULL) {
          err_value = CAL_CRYPTO_API_ERROR;
          break;
        }

        /* Create encoded RSA PSS message */
        if (RSA_padding_add_PKCS1_PSS(rsa, em, hash_msg, md, -1) != 1) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot create EM %s", key_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        enc_sig_size = em_size;
        enc_sig = OPENSSL_malloc(enc_sig_size);

        /* Create RSA PSS signature */
        if (RSA_private_encrypt(enc_sig_size, em, enc_sig, rsa, RSA_NO_PADDING) == -1) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot create signature %s", key_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        if (enc_sig_size > *sig_buf_bytes) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Generated signature too large for allocated buffer %s", key_file);
            display_error(err_str);
            err_value = CAL_INVALID_SIG_DATA_SIZE;
            break;
        }
        *sig_buf_bytes = enc_sig_size;
        memcpy(sig_buf, enc_sig, enc_sig_size);

    } while(0);

    if (rsa) RSA_free(rsa);
    if (hash_msg) OPENSSL_free(hash_msg);
    if (em) OPENSSL_free(em);
    if (enc_sig) OPENSSL_free(enc_sig);
    return err_value;
}

/*--------------------------
  cms_to_buf
---------------------------*/
int32_t cms_to_buf(CMS_ContentInfo *cms, BIO * bio_in, uint8_t * data_buffer,
                            size_t * data_buffer_size, int32_t flags)
{
    int32_t err_value = CAL_SUCCESS;
    BIO * bio_out = NULL;
    BUF_MEM buffer_memory;            /**< Used with BIO functions */
    (void) bio_in;

    buffer_memory.length = 0;
    buffer_memory.data = (char*)data_buffer;
    buffer_memory.max = *data_buffer_size;

    do {
        if (!(bio_out = BIO_new(BIO_s_mem()))) {
            display_error("Unable to allocate CMS signature result memory");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        BIO_set_mem_buf(bio_out, &buffer_memory, BIO_NOCLOSE);

        /* Convert cms to der format */
        if (!i2d_CMS_bio_stream(bio_out, cms, bio_in, flags)) {
            display_error("Unable to convert CMS signature to DER format");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Get the size of bio out in data_buffer_size */
        *data_buffer_size = BIO_ctrl_pending(bio_out);
    }while(0);

    if (bio_out)
        BIO_free(bio_out);
    return err_value;
}

#if ENABLE_VERIFY

static int check_verified_signer(CMS_ContentInfo* cms, X509_STORE* store)
{
    int i, ret = 1;

    X509_STORE_CTX *ctx = X509_STORE_CTX_new();
    STACK_OF(CMS_SignerInfo) *infos = CMS_get0_SignerInfos(cms);
    STACK_OF(X509)* cms_certs = CMS_get1_certs(cms);

    if (!ctx) {
        LOG_DEBUG("Failed to allocate verification context\n");
        return ret;
    }

    for (i = 0; i < sk_CMS_SignerInfo_num(infos) && ret != 0; ++i) {
        CMS_SignerInfo *si = sk_CMS_SignerInfo_value(infos, i);
        X509 *signer = NULL;

        CMS_SignerInfo_get0_algs(si, NULL, &signer, NULL, NULL);
        if (!X509_STORE_CTX_init(ctx, store, signer, cms_certs)) {
            LOG_DEBUG("Failed to initialize signer verification operation\n");
            break;
        }

        X509_STORE_CTX_set_default(ctx, "smime_sign");
        if (X509_verify_cert(ctx) > 0) {
            LOG_DEBUG("Verified signature %d in signer sequence\n", i);
            ret = 0;
        } else {
            LOG_DEBUG("Failed to verify certificate %d in signer sequence\n", i);
        }

        X509_STORE_CTX_cleanup(ctx);
    }

    X509_STORE_CTX_free(ctx);

    return ret;
}

static int cms_verify_callback(int ok, X509_STORE_CTX *ctx) {
    int cert_error = X509_STORE_CTX_get_error(ctx);

    if (!ok) {
        switch (cert_error) {
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_CERT_NOT_YET_VALID:
            ok = 1;
            break;
        default:
            break;
        }
    }

    return ok;
}

X509_STORE *load_cert_chain(const char *file)
{
    X509_STORE *castore = X509_STORE_new();
    if (!castore) {
        return NULL;
    }

    /*
     * Set error callback function for verification of CRTs and CRLs in order
     * to ignore some errors depending on configuration
     */
    X509_STORE_set_verify_cb(castore, cms_verify_callback);

    BIO *castore_bio = BIO_new_file(file, "r");
    if (!castore_bio) {
        LOG_DEBUG("failed: BIO_new_file(%s)\n", file);
        return NULL;
    }

    int crt_count = 0;
    X509 *crt = NULL;
    do {
        crt = PEM_read_bio_X509(castore_bio, NULL, 0, NULL);
        if (crt) {
            crt_count++;
            char *subj = X509_NAME_oneline(X509_get_subject_name(crt), NULL, 0);
            char *issuer = X509_NAME_oneline(X509_get_issuer_name(crt), NULL, 0);
            LOG_DEBUG("Read PEM #%d: %s %s\n", crt_count, issuer, subj);
            free(subj);
            free(issuer);
            if (X509_STORE_add_cert(castore, crt) == 0) {
                LOG_DEBUG("Adding certificate to X509_STORE failed\n");
                BIO_free(castore_bio);
                X509_STORE_free(castore);
                return NULL;
            }
        }
    } while (crt);
    BIO_free(castore_bio);

    if (crt_count == 0) {
        X509_STORE_free(castore);
        return NULL;
    }
    LOG_DEBUG("The crt_count is %d\n", crt_count);

    return castore;
}

/*--------------------------
  gen_sig_data_cms
---------------------------*/
int32_t
verify_sig_data_cms(const char *in_file,
                    const char *cert_ca,
                    const char *cert_signer,
                    const char *sig_file,
                    hash_alg_t hash_alg)

{
    BIO             *bio_in = NULL;   /**< BIO for in_file data */
    BIO             *bio_sigfile = NULL;   /**< BIO for sigfile data */
    X509_STORE      *store = NULL;     /**< Ptr to X509 certificate read data */
    X509            *signer_cert = NULL;
    CMS_ContentInfo *cms = NULL;      /**< Ptr used with openssl API */
    const EVP_MD    *sign_md = NULL;  /**< Ptr to digest name */
    int32_t err_value = CAL_SUCCESS;  /**< Used for return value */
    int32_t rc = 0;
    /** Array to hold error string */
    char err_str[MAX_ERR_STR_BYTES];
    /* flags set to match Openssl command line options for generating
     *  signatures
     */
    int32_t         flags = CMS_DETACHED | CMS_NOCERTS |
                            CMS_NOSMIMECAP | CMS_BINARY;

    /* Set signature message digest alg */
    sign_md = EVP_get_digestbyname(get_digest_name(hash_alg));
    if (sign_md == NULL) {
        display_error("Invalid hash digest algorithm");
        return CAL_INVALID_ARGUMENT;
    }

    do
    {
        store = load_cert_chain(cert_ca);
        if (store == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open ca certificate file %s", cert_ca);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        signer_cert = read_certificate(cert_signer);
        if (!signer_cert) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open signer certificate file %s", cert_signer);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Read signature Data */
        if (!(bio_sigfile = BIO_new_file(sig_file, "rb"))) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open signature file %s", sig_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        flags |= CMS_NO_SIGNER_CERT_VERIFY;

        /* Parse the DER-encoded CMS message */
        cms = d2i_CMS_bio(bio_sigfile, NULL);
        if (!cms) {
            display_error("Cannot be parsed as DER-encoded CMS signature blob.\n");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        if (!CMS_add1_cert(cms, signer_cert)) {
            display_error("Cannot be inserted signer_cert into cms.\n");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Open the content file (data which was signed) */
        if (!(bio_in = BIO_new_file(in_file, "rb"))) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open data which was signed  %s", in_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        rc = CMS_verify(cms, NULL, store, bio_in, NULL, flags);
        if (!rc) {
            display_error("Failed to verify the file!\n");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        if (check_verified_signer(cms, store)) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Authentication of all signatures failed!\n");
            err_value = CAL_CRYPTO_API_ERROR;
            display_error(err_str);
            break;
        }

        LOG_DEBUG("Verified OK!\n");

    } while(0);

    /* Print any Openssl errors */
    if (err_value != CAL_SUCCESS) {
        ERR_print_errors_fp(stderr);
    }

    /* Close everything down */
    if (cms) CMS_ContentInfo_free(cms);
    if (store) X509_STORE_free(store);
    if (bio_in) BIO_free(bio_in);
    if (bio_sigfile)   BIO_free(bio_sigfile);

    return err_value;
}

#endif /* ENABLE_VERIFY */
/*--------------------------
  gen_sig_data_cms
---------------------------*/
#if !AUTOX_SIGN
int32_t
gen_sig_data_cms(const char *in_file,
                 const char *cert_file,
                 const char *key_file,
                 hash_alg_t hash_alg,
                 uint8_t *sig_buf,
                 size_t *sig_buf_bytes)
{
    BIO             *bio_in = NULL;   /**< BIO for in_file data */
    X509            *cert = NULL;     /**< Ptr to X509 certificate read data */
    EVP_PKEY        *key = NULL;      /**< Ptr to key read data */
    CMS_ContentInfo *cms = NULL;      /**< Ptr used with openssl API */
    const EVP_MD    *sign_md = NULL;  /**< Ptr to digest name */
    int32_t err_value = CAL_SUCCESS;  /**< Used for return value */
    /** Array to hold error string */
    char err_str[MAX_ERR_STR_BYTES];
    /* flags set to match Openssl command line options for generating
     *  signatures
     */
    int32_t         flags = CMS_DETACHED | CMS_NOCERTS |
                            CMS_NOSMIMECAP | CMS_BINARY;

    /* Set signature message digest alg */
    sign_md = EVP_get_digestbyname(get_digest_name(hash_alg));
    if (sign_md == NULL) {
        display_error("Invalid hash digest algorithm");
        return CAL_INVALID_ARGUMENT;
    }

    do
    {
        cert = read_certificate(cert_file);
        if (!cert) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open certificate file %s", cert_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Read key */
        key = read_private_key(key_file,
                           (pem_password_cb *)get_passcode_to_key_file,
                           key_file);
        if (!key) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open key file %s", key_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Read Data to be signed */
        if (!(bio_in = BIO_new_file(in_file, "rb"))) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open data file %s", in_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Generate CMS Signature - can only use CMS_sign if default
         * MD is used which is SHA1 */
        flags |= CMS_PARTIAL;

        cms = CMS_sign(NULL, NULL, NULL, bio_in, flags);
        if (!cms) {
            display_error("Failed to initialize CMS signature");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        if (!CMS_add1_signer(cms, cert, key, sign_md, flags)) {
            display_error("Failed to generate CMS signature");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Finalize the signature */
        if (!CMS_final(cms, bio_in, NULL, flags)) {
            display_error("Failed to finalize CMS signature");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Write CMS signature to output buffer - DER format */
        err_value = cms_to_buf(cms, bio_in, sig_buf, sig_buf_bytes, flags);
    } while(0);

    /* Print any Openssl errors */
    if (err_value != CAL_SUCCESS) {
        ERR_print_errors_fp(stderr);
    }

    /* Close everything down */
    if (cms)      CMS_ContentInfo_free(cms);
    if (cert)     X509_free(cert);
    if (key)      EVP_PKEY_free(key);
    if (bio_in)   BIO_free(bio_in);

    return err_value;
}
#endif /* !AUTOX_SIGN */
/*--------------------------
  gen_sig_data_ecdsa
---------------------------*/
int32_t
gen_sig_data_ecdsa(const char *in_file,
                   const char *key_file,
                   hash_alg_t hash_alg,
                   uint8_t    *sig_buf,
                   size_t     *sig_buf_bytes)
{
    BIO          *bio_in    = NULL;          /**< BIO for in_file data    */
    EVP_PKEY     *key       = NULL;          /**< Private key data        */
    size_t       key_size   = 0;             /**< n of bytes of key param */
    const EVP_MD *sign_md   = NULL;          /**< Digest name             */
    uint8_t      *hash      = NULL;          /**< Hash data of in_file    */
    int32_t      hash_bytes = 0;             /**< Length of hash buffer   */
    uint8_t      *sign      = NULL;          /**< Signature data in DER   */
    uint32_t     sign_bytes = 0;             /**< Length of DER signature */
    uint8_t      *r = NULL, *s = NULL;       /**< Raw signature data R&S  */
    size_t       bn_bytes = 0;               /**< Length of R,S big num   */
    ECDSA_SIG    *sign_dec  = NULL;          /**< Raw signature data R|S  */
    int32_t      err_value  = CAL_SUCCESS;   /**< Return value            */
    char         err_str[MAX_ERR_STR_BYTES]; /**< Error string            */
    const BIGNUM *sig_r, *sig_s;             /**< signature numbers defined as OpenSSL BIGNUM */

    /* Set signature message digest alg */
    sign_md = EVP_get_digestbyname(get_digest_name(hash_alg));
    if (sign_md == NULL) {
        display_error("Invalid hash digest algorithm");
        return CAL_INVALID_ARGUMENT;
    }

    do
    {
        /* Read key */
        key = read_private_key(key_file,
                               (pem_password_cb *)get_passcode_to_key_file,
                               key_file);
        if (!key) {
            snprintf(err_str, MAX_ERR_STR_BYTES,
                     "Cannot open key file %s", key_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Read Data to be signed */
        if (!(bio_in = BIO_new_file(in_file, "rb"))) {
            snprintf(err_str, MAX_ERR_STR_BYTES,
                     "Cannot open data file %s", in_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Generate hash of data from in_file */
        hash_bytes = HASH_BYTES_MAX;
        hash = OPENSSL_malloc(HASH_BYTES_MAX);

        err_value = calculate_hash(in_file, hash_alg, hash, &hash_bytes);
        if (err_value != CAL_SUCCESS) {
            break;
        }

        /* Generate ECDSA signature with DER encoding */
        sign_bytes = ECDSA_size(EVP_PKEY_get0_EC_KEY(key));
        sign = OPENSSL_malloc(sign_bytes);

        if (0 == ECDSA_sign(0 /* ignored */, hash, hash_bytes, sign, &sign_bytes, EVP_PKEY_get0_EC_KEY(key))) {
            display_error("Failed to generate ECDSA signature");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        sign_dec = d2i_ECDSA_SIG(NULL, (const uint8_t **) &sign, sign_bytes);
        if (NULL == sign_dec) {
            display_error("Failed to decode ECDSA signature");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Copy R|S to sig_buf */
        memset(sig_buf, 0, *sig_buf_bytes);

        key_size = EVP_PKEY_bits(key) >> 3;
        if (EVP_PKEY_bits(key) & 0x7) key_size += 1; /* Valid for P-521 */

        if ((key_size * 2) > *sig_buf_bytes){
            display_error("Signature buffer too small");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        *sig_buf_bytes = key_size * 2;

        ECDSA_SIG_get0(sign_dec, &sig_r, &sig_s);

        r = get_bn(sig_r, &bn_bytes);
        memcpy(sig_buf + (key_size - bn_bytes),
               r,
               bn_bytes);
        free(r);

        s = get_bn(sig_s, &bn_bytes);
        memcpy(sig_buf + key_size + (key_size - bn_bytes),
               s,
               bn_bytes);
        free(s);
    } while(0);

    /* Print any Openssl errors */
    if (err_value != CAL_SUCCESS) {
        ERR_print_errors_fp(stderr);
    }

    /* Close everything down */
    if (key)    EVP_PKEY_free(key);
    if (bio_in) BIO_free(bio_in);

    return err_value;
}

/*--------------------------
  error
---------------------------*/
void
display_error(const char *err)
{
    fprintf(stderr, "Error: %s\n", err);
}

/*--------------------------
  calculate_sig_buf_size
---------------------------*/
#define SIG_BUF_FIXED_DATA_SIZE 217

int32_t
calculate_sig_buf_size(const char *cert_file) {
    X509 *cert;
    EVP_PKEY *public_key;
    RSA *rsa_key;
    int key_length = 0;
    ASN1_INTEGER *serial_num;
    int sn_length = 0;
    char *issuer_name;
    int issuer_name_length = 0;

    /* Calculate Signature Size for padding purposes in CSF binary:
    *     It has been experimentally evalutated and noted that the variables in
    *     signature size is related to three entries in the CMS format. This is
    *     only considering signatures from the keys created from the CST PKI
    *     scripts.
    *
    *     The fixed size of the CMS structure was experimentally
    *     determined to be 217 bytes.
    *
    *     The three entries that appear to alter the size:
    *         Serial Number (SN)
    *         CA Certificate Name
    *         RSA Encryption  (this size is equal to key size in bytes)
    *
    *     Given the above the current approach to calculating the signature size:
    *
    *         <fixed size (217)> + <SN Size> + <CA Name Size> + <RSA Data Size>
    */
    /* Gather Signing Key Details */
    cert = read_certificate(cert_file);
    public_key = X509_get_pubkey(cert);
    rsa_key =  EVP_PKEY_get1_RSA(public_key);
    key_length = RSA_size(rsa_key);
    serial_num = X509_get_serialNumber(cert);
    sn_length = serial_num->length;
    issuer_name = X509_NAME_oneline(X509_get_issuer_name(cert),NULL, 0);
    issuer_name_length = strlen(issuer_name);

    return (SIG_BUF_FIXED_DATA_SIZE + key_length + sn_length + issuer_name_length);
}

/*--------------------------
  export_habv4_signature_request
---------------------------*/
int32_t
export_habv4_signature_request(const char *in_file,
                               const char *cert_file,
                               uint8_t *sig_buf,
                               size_t *sig_buf_bytes)
{
    FILE *sig_req_fp = NULL;
    int unique_ident[2];
    FILE *in_file_fp = NULL;
    FILE *cpy_data_fp = NULL;
    char tmp_filename[80];
    int in_data_size = 0;

    /* Seed rand() to generate unique data tags */
    srand(time(0));

    /* Open data file */
    if(!(in_file_fp = fopen(in_file, "rb")))
    {
      printf("Unable to open tmp data file\n");
      return -1;
    }

    /* Determine the data file size */
    if(fseek(in_file_fp,0L,SEEK_END) == -1)
    {
      printf("Unable to get in_data filesize\n");
      fclose(in_file_fp);
      return -1;
    }
    if ((in_data_size = ftell(in_file_fp)) < 0) {
      printf("Unable to get in_data filesize\n");
      fclose(in_file_fp);
      return -1;
    }

    /* Set the fp to the beginning of the data file */
    rewind(in_file_fp);

    /* Create the output data file name */
    strcpy(tmp_filename,"data_");
    strcat(tmp_filename,in_file);

    /* Copy the data file to the output data file */
    cpy_data_fp = fopen(tmp_filename, "w");

    while (in_data_size--)
    {
      char tmp;
      tmp = fgetc(in_file_fp);  // copying file character by character
      fputc(tmp, cpy_data_fp);
    }

    /* Create unique identifier */
    unique_ident[0] = rand();
    unique_ident[1] = rand();

    /* Add entry to signing request file for new data file */
    sig_req_fp = fopen("sig_request.txt", "a");
    fseek(sig_req_fp, 0L, SEEK_END);
    fprintf(sig_req_fp,"Signing Request:\n");
    fprintf(sig_req_fp,"%s\n", tmp_filename);
    fprintf(sig_req_fp,"unique tag: %08x%08x\n",unique_ident[1], unique_ident[0]);

    /* Close all files */
    fclose(in_file_fp);
    fclose(cpy_data_fp);
    fclose(sig_req_fp);

    *sig_buf_bytes = calculate_sig_buf_size(cert_file);

    memset(sig_buf, 0, *sig_buf_bytes);
    memcpy(sig_buf, unique_ident, sizeof(unique_ident) );

    return 0;
}

/*--------------------------
  export_signature_request
---------------------------*/
int32_t export_signature_request(const char *in_file,
                                 const char *cert_file)
{
    #define WRITE_LINE()                                              \
        if (strlen(line) != fwrite(line, 1, strlen(line), sig_req)) { \
            snprintf(err_str, MAX_ERR_STR_BYTES,                      \
                     "Unable to write to file %s", SIG_REQ_FILENAME); \
            display_error(err_str);                                   \
            return CAL_CRYPTO_API_ERROR;                              \
        }

    char err_str[MAX_ERR_STR_BYTES]; /**< Used in preparing error message  */
    FILE *sig_req = NULL;            /**< Output signing request           */
    char line[MAX_LINE_CHARS];       /**< Used in preparing output message */

    sig_req = fopen(SIG_REQ_FILENAME, "a");
    if (NULL ==  sig_req) {
        snprintf(err_str, MAX_ERR_STR_BYTES,
                 "Unable to create file %s", SIG_REQ_FILENAME);
        display_error(err_str);
        return CAL_CRYPTO_API_ERROR;
    }

    snprintf(line, MAX_LINE_CHARS,
             "[Signing request]\r\n");
    WRITE_LINE();
    snprintf(line, MAX_LINE_CHARS,
             "Signing certificate = %s\r\n", cert_file);
    WRITE_LINE();
    snprintf(line, MAX_LINE_CHARS,
             "Data to be signed   = %s\r\n\r\n", in_file);
    WRITE_LINE();

    fclose(sig_req);

    return CAL_SUCCESS;
}

/*===========================================================================
                              GLOBAL FUNCTIONS
=============================================================================*/
/*--------------------------
  ssl_gen_sig_data
---------------------------*/
int32_t ssl_gen_sig_data(const char* in_file,
                     const char* cert_file,
                     hash_alg_t hash_alg,
                     sig_fmt_t sig_fmt,
                     uint8_t* sig_buf,
                     size_t *sig_buf_bytes,
                     func_mode_t mode)
{
    int32_t err = CAL_SUCCESS; /**< Used for return value */
    char *key_file = NULL;     /**< Mem ptr for key filename */

    /* Check for valid arguments */
    if ((!in_file) || (!cert_file) || (!sig_buf) || (!sig_buf_bytes)) {
        return CAL_INVALID_ARGUMENT;
    }

    if (MODE_HSM == mode)
    {
        if ( TGT_AHAB == g_target ) {
            return export_signature_request(in_file, cert_file);
        } else if ( TGT_HAB == g_target ) {
            return export_habv4_signature_request(in_file, cert_file,
                                            sig_buf, sig_buf_bytes);
        }
    }

    /* Determine private key filename from given certificate filename */
    key_file = malloc(strlen(cert_file)+1);
    err = get_key_file(cert_file, key_file);
    if ( err != CAL_SUCCESS) {
        free(key_file);
        return CAL_FILE_NOT_FOUND;
    }

    if (SIG_FMT_PKCS1 == sig_fmt) {
        err = gen_sig_data_raw(in_file, key_file,
                               hash_alg, sig_buf, (int32_t *)sig_buf_bytes);
    }
    else if (SIG_FMT_RSA_PSS == sig_fmt) {
        err = gen_sig_data_pss(in_file, key_file,
                               hash_alg, sig_buf, (int32_t *)sig_buf_bytes);
    }
    else if (SIG_FMT_CMS == sig_fmt) {
#if AUTOX_SIGN
        err = autox_gen_sig_data_cms(in_file, sig_buf,
                                     sig_buf_bytes, autox_signed_file_name);
        if (err != CAL_SUCCESS) {
            goto finish;
        }
        if (*sig_buf_bytes > 1024) {
            printf("sig_buf_bytes is oversize!!! %lu\n", *sig_buf_bytes);
            err = CAL_INVALID_SIG_DATA_SIZE;
            goto finish;
        }
#else
        err = gen_sig_data_cms(in_file, cert_file, key_file,
                               hash_alg, sig_buf, sig_buf_bytes);
        do {
            char sig_name[1024];
            sprintf(sig_name, "signed_%s", in_file);
            (void)autox_write_binary_all(sig_name, sig_buf, *sig_buf_bytes);
        } while (0);
#endif /* AUTOX_SIGN */
    printf("Sign Done! Signature size is %lu\n", *sig_buf_bytes);
#if ENABLE_VERIFY
        if (err != CAL_SUCCESS) {
            goto finish;
        }
        const char *autox_ca_cert = "autox_ca_chains.crt";
        const char *autox_signer_cert = "autox_signer.crt";
        printf("\n-------------------------[Verify infomation]----------------------\n");
        printf("original file  : %s\n", in_file);
        printf("signature file : %s\n", autox_signed_file_name);
        printf("ca cert        : %s\n", autox_ca_cert);
        printf("signer cert    : %s\n", autox_signer_cert);
        printf("hash_alg       : %d\n", hash_alg);
        printf("--------------------------------------------------------------------\n\n");
        err = verify_sig_data_cms(in_file, autox_ca_cert, autox_signer_cert, autox_signed_file_name, hash_alg);
#endif /* ENABLE_VERIFY */
    }
    else if (SIG_FMT_ECDSA == sig_fmt) {
        err = gen_sig_data_ecdsa(in_file, key_file,
                                 hash_alg, sig_buf, sig_buf_bytes);
    }
    else {
        free(key_file);
        display_error("Invalid signature format");
        return CAL_INVALID_ARGUMENT;
    }
#if ENABLE_VERIFY || AUTOX_SIGN
finish:
#endif /* ENABLE_VERIFY */
    if (key_file != NULL)
        free(key_file);
    return err;
}

static int32_t autox_write_binary_all(const char *filename, uint8_t *buffer, size_t o_len)
{
    int32_t ret = 0;
    FILE *fp = NULL;

    fp = fopen(filename, "wb+");
    if (fp == NULL) {
        ret = -1;
        goto finish;
    }

    size_t blocks_write = fwrite(buffer, o_len, 1, fp);
    if (blocks_write != 1) {
        ret = -1;
        goto finish;
    }

finish:
    if (fp != NULL) fclose(fp);
    return ret;
}

#if AUTOX_SIGN
static int32_t autox_read_binary_all(const char *filename, uint8_t **buffer, size_t *o_len)
{
    int32_t ret = 0;
    struct stat info;
    FILE *fp = NULL;

    if (stat(filename, &info) != 0) {
        ret = -1;
        goto finish;
    }

    *buffer = (uint8_t *)calloc(1, info.st_size);
    if (NULL == *buffer) {
        LOG_DEBUG("Malloc Buffer failed!\n");
        ret = -1;
        goto finish;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        ret = -1;
        if (*buffer != NULL) {
            free(*buffer);
            *buffer = NULL;
        }
        goto finish;
    }

    /* Try to read a single block of info.st_size bytes */
    size_t blocks_read = fread(*buffer, info.st_size, 1, fp);
    if (blocks_read != 1) {
        ret = -1;
        if (*buffer != NULL) {
            free(*buffer);
            *buffer = NULL;
        }
        goto finish;
    }

    *o_len = info.st_size;

finish:

    if (fp != NULL) fclose(fp);
    return ret;
}

static int32_t autox_request_sign_data(int32_t srk_num,
                                       int32_t csf_or_image,
                                       const char *in_file,
                                       uint8_t *sig_buf,
                                       size_t *sig_buf_bytes,
                                       char *out_name)
{
    int32_t err = 0;
    char err_str[MAX_ERR_STR_BYTES];
    char *signed_file = NULL;
    uint8_t *buffer = NULL;
    const char *ssl_cert = SIGN_SERVER_SSL_CERT;
    const char *ssl_key = SIGN_SERVER_SSL_KEY;
    const char *root_ca = SIGN_SERVER_ROOT_CA;
    const char *url_api = SIGN_SERVER_API_URL;

    UNUSED(srk_num);
    UNUSED(in_file);

    if (NULL == in_file ||
        NULL == sig_buf ||
        NULL == sig_buf_bytes ||
        NULL == out_name) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "input parameters error!");
        display_error(err_str);
        err = CAL_INVALID_ARGUMENT;
        goto finish;
    }

    // err = autox_download_root_ca(SIGN_SERVER_API_URL, root_ca);
    // if (err != 0) {
    //     snprintf(err_str, MAX_ERR_STR_BYTES-1,
    //             "download root ca %s failed!", root_ca);
    //     display_error(err_str);
    //     err = CAL_INVALID_ARGUMENT;
    //     goto finish;
    // }

    signed_file = csf_or_image ? \
                  SIGN_SERVER_SIGNED_CSF_OUT_NAME : \
                  SIGN_SERVER_SIGNED_IMAGE_OUT_NAME;
    strcpy(out_name, signed_file);

    err = autox_sign_with_hsm(in_file,
                              root_ca,
                              ssl_cert,
                              ssl_key,
                              url_api,
                              out_name);
    if (err != 0) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "autox_sign_with_hsm_file_buffer %s failed!", in_file);
        display_error(err_str);
        err = CAL_INVALID_ARGUMENT;
        goto finish;
    }

    err = autox_read_binary_all(out_name, &buffer, sig_buf_bytes);
    if (err != 0) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "autox_read_binary_all %s failed!", out_name);
        display_error(err_str);
        err = CAL_INVALID_ARGUMENT;
        goto finish;
    }

    memcpy(sig_buf, buffer, *sig_buf_bytes);

finish:
    if (buffer != NULL) {
        free(buffer);
    }
    return err;
}

int32_t autox_gen_sig_data_cms(const char* in_file,
                               uint8_t* sig_buf,
                               size_t *sig_buf_bytes,
                               char *sig_out_file)
{
    int32_t err = CAL_SUCCESS; /**< Used for return value */
    char err_str[MAX_ERR_STR_BYTES];
    size_t o_len = 0;
    uint8_t *buffer = NULL;

    if (NULL == in_file ||
        NULL == sig_buf ||
        NULL == sig_buf_bytes ||
        NULL == sig_out_file) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "input parameters error!");
        err = CAL_INVALID_ARGUMENT;
        goto finish;
    }

    LOG_DEBUG("Bypass NXP signing, makes use of the AUTOX's signer!!!!\n");

    char *dup_in_file_name = NULL;
    int32_t csf_or_image = 0;

    if (0 == strcmp(in_file, FILE_SIG_IMG_DATA)) {
        dup_in_file_name = SIGN_SERVER_SIGNED_IMAGE_IN_NAME;
        csf_or_image = 0;
    } else if (0 == strcmp(in_file, FILE_SIG_CSF_DATA)) {
        dup_in_file_name = SIGN_SERVER_SIGNED_CSF_IN_NAME;
        csf_or_image = 1;
    } else {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "Internal Error, No %s or %s set!", FILE_SIG_IMG_DATA, FILE_SIG_CSF_DATA);
        err = CAL_INVALID_ARGUMENT;
        goto finish;
    }

    // backup the origin sign file
    err = autox_read_binary_all(in_file, &buffer, &o_len);
    if (err != 0) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "input parameters error!");
        err = CAL_FAILED_FILE_CREATE;
        goto finish;
    }

    err = autox_write_binary_all(dup_in_file_name, buffer, o_len);
    if (err != 0) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "autox_write_binary_all error!");
        err = CAL_FAILED_FILE_CREATE;
        goto finish;
    }

    err = autox_request_sign_data(1,
                                  csf_or_image,
                                  in_file,
                                  sig_buf,
                                  sig_buf_bytes,
                                  sig_out_file);
    if (err != 0) {
        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                "Internal Error, call autox_request_sign_data for %s failed!", in_file);
        display_error(err_str);
        goto finish;
    }

finish:
    if (buffer != NULL) {
        free(buffer);
    }
    return err;
}
#endif /* AUTOX_SIGN */
/*--------------------------
  generate_dek_key
---------------------------*/
int32_t generate_dek_key(uint8_t * key, int32_t len)
{
    if (gen_random_bytes(key, len) != CAL_SUCCESS) {
        return CAL_CRYPTO_API_ERROR;
    }

    return CAL_SUCCESS;
}

/*--------------------------
  write_plaintext_dek_key
---------------------------*/
int32_t write_plaintext_dek_key(uint8_t * key, size_t key_bytes,
                const char * cert_file, const char * enc_file)
{
    int32_t err_value = CAL_SUCCESS;  /**< Return value */
    char err_str[MAX_ERR_STR_BYTES];  /**< Used in preparing error message */
    FILE *fh = NULL;                  /**< File handle used with file api */
#ifdef DEBUG
    int32_t i = 0;                    /**< Used in for loops */
#endif

    UNUSED(cert_file);

    do {
        /* Save the buffer into enc_file */
        if ((fh = fopen(enc_file, "wb")) == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Unable to create binary file %s", enc_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        if (fwrite(key, 1, key_bytes, fh) !=
            key_bytes) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Unable to write to binary file %s", enc_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        fclose (fh);
   } while(0);

    return err_value;
}


/*--------------------------
  encrypt_dek_key
---------------------------*/
int32_t encrypt_dek_key(uint8_t * key, size_t key_bytes,
                const char * cert_file, const char * enc_file)
{
    X509            *cert = NULL;     /**< Ptr to X509 certificate read data */
    STACK_OF(X509) *recips = NULL;    /**< Ptr to X509 stack */
    CMS_ContentInfo *cms = NULL;      /**< Ptr to cms structure */
    const EVP_CIPHER *cipher = NULL;  /**< Ptr to EVP_CIPHER */
    int32_t err_value = CAL_SUCCESS;  /**< Return value */
    char err_str[MAX_ERR_STR_BYTES];  /**< Used in preparing error message */
    BIO *bio_key = NULL;              /**< Bio for the key data to encrypt */
    uint8_t * enc_buf = NULL;         /**< Ptr for encoded key data */
    FILE *fh = NULL;                  /**< File handle used with file api */
    size_t cms_info_size = MAX_CMS_DATA; /**< Size of cms content info*/
#ifdef DEBUG
    int32_t i = 0;                    /**< Used in for loops */
#endif

    do {
        /* Read the certificate from cert_file */
        cert = read_certificate(cert_file);
        if (!cert) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot open certificate file %s", cert_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Create recipient STACK and add recipient cert to it */
        recips = sk_X509_new_null();

        if (!recips || !sk_X509_push(recips, cert)) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Cannot instantiate object STACK_OF(%s)", cert_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /*
         * sk_X509_pop_free will free up recipient STACK and its contents
         * so set cert to NULL so it isn't freed up twice.
         */
        cert = NULL;

        /* Instantiate correct cipher */
        if (key_bytes == (AES_KEY_LEN_128 / BYTE_SIZE_BITS))
            cipher = EVP_aes_128_cbc();
        else if (key_bytes == (AES_KEY_LEN_192 / BYTE_SIZE_BITS))
            cipher = EVP_aes_192_cbc();
        else if (key_bytes == (AES_KEY_LEN_256 / BYTE_SIZE_BITS))
            cipher = EVP_aes_256_cbc();
        if (cipher == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Invalid cipher used for encrypting key %s", enc_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Allocate memory buffer BIO for input key */
        bio_key = BIO_new_mem_buf(key, key_bytes);
        if (!bio_key) {
            display_error("Unable to allocate BIO memory");
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Encrypt content of the key with certificate */
        cms = CMS_encrypt(recips, bio_key, cipher, CMS_BINARY|CMS_STREAM);
        if (cms == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Failed to encrypt key data");
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Finalize the CMS content info structure */
        if (!CMS_final(cms, bio_key, NULL,  CMS_BINARY|CMS_STREAM)) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Failed to finalize cms data");
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Alloc mem to convert cms to binary and save it into enc_file */
        enc_buf = malloc(MAX_CMS_DATA);
        if (enc_buf == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Failed to allocate memory");
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }

        /* Copy cms info into enc_buf */
        err_value = cms_to_buf(cms, bio_key, enc_buf, &cms_info_size,
            CMS_BINARY);

        /* Save the buffer into enc_file */
        if ((fh = fopen(enc_file, "wb")) == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Unable to create binary file %s", enc_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        if (fwrite(enc_buf, 1, cms_info_size, fh) !=
            cms_info_size) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Unable to write to binary file %s", enc_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        fclose (fh);
#ifdef DEBUG
        printf("Encoded key ;");
        for(i=0; i<key_bytes; i++) {
            printf("%02x ", enc_buf[i]);
        }
        printf("\n");
#endif
    } while(0);

    if (cms)
        CMS_ContentInfo_free(cms);
    if (cert)
        X509_free(cert);
    if (recips)
        sk_X509_pop_free(recips, X509_free);
    if (bio_key)
        BIO_free(bio_key);
    return err_value;
}

/*--------------------------
  gen_auth_encrypted_data
---------------------------*/
int32_t gen_auth_encrypted_data(const char* in_file,
                     const char* out_file,
                     aead_alg_t aead_alg,
                     uint8_t *aad,
                     size_t aad_bytes,
                     uint8_t *nonce,
                     size_t nonce_bytes,
                     uint8_t *mac,
                     size_t mac_bytes,
                     size_t key_bytes,
                     const char* cert_file,
                     const char* key_file,
                     int reuse_dek)
{
    int32_t err_value = CAL_SUCCESS;         /**< status of function calls */
    char err_str[MAX_ERR_STR_BYTES];         /**< Array to hold error string */
    static uint8_t key[MAX_AES_KEY_LENGTH];  /**< Buffer for random key */
    static uint8_t key_init_done = 0;        /**< Status of key initialization */
    FILE *fh = NULL;                         /**< Used with files */
    size_t file_size;                        /**< Size of in_file */
    unsigned char *plaintext = NULL;                /**< Array to read file data */
    int32_t bytes_read;
#ifdef DEBUG
    int32_t i;                                        /**< used in for loops */
#endif
    uint8_t nonce_temp[nonce_bytes];
    int32_t j = 1;

    do {
        if (AES_CCM == aead_alg) { /* HAB4 */
            /* Test random byte generation twice for functional confirmation */
            do {
                /* Generate Nonce */
                err_value = gen_random_bytes((uint8_t*)nonce, nonce_bytes);
                if (err_value != CAL_SUCCESS) {
                    snprintf(err_str, MAX_ERR_STR_BYTES-1,
                                "Failed to get nonce");
                    display_error(err_str);
                    err_value = CAL_CRYPTO_API_ERROR;
                    break;
                }
                /* Copy nonce in temp variable to compare in next iteration */
                if (1 == j) {
                    memcpy(&nonce_temp, nonce, nonce_bytes);
                }
                else {
                    /* If random numbers in two iterations are equal, throw an error */
                    if (!memcmp(&nonce_temp, nonce, nonce_bytes)) {
                        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                                    "Invalid nonce generated");
                        display_error(err_str);
                        err_value = CAL_CRYPTO_API_ERROR;
                        break;
                    }
                }
            } while (j--);
        }
        /* Exit this loop if error encountered in random bytes generation */
        if (err_value != CAL_SUCCESS)
            break;
#ifdef DEBUG
        printf("nonce bytes: ");
        for(i=0; i<nonce_bytes; i++) {
            printf("%02x ", nonce[i]);
        }
        printf("\n");
#endif
        if (0 == key_init_done) {
            if (reuse_dek) {
                fh = fopen(key_file, "rb");
                if (fh == NULL) {
                    snprintf(err_str, MAX_ERR_STR_BYTES-1,
                        "Unable to open dek file %s", key_file);
                    display_error(err_str);
                    err_value = CAL_FILE_NOT_FOUND;
                    break;
                }
                /* Read encrypted data into input_buffer */
                bytes_read = fread(key, 1, key_bytes, fh);
                if (bytes_read == 0) {
                    snprintf(err_str, MAX_ERR_STR_BYTES-1,
                        "Cannot read file %s", key_file);
                    display_error(err_str);
                    err_value = CAL_FILE_NOT_FOUND;
                    fclose(fh);
                    break;
                }
                fclose(fh);
            }
            else {
                /* Generate random aes key to use it for encrypting data */
                    err_value = generate_dek_key(key, key_bytes);
                    if (err_value) {
                        snprintf(err_str, MAX_ERR_STR_BYTES-1,
                                    "Failed to generate random key");
                        display_error(err_str);
                        err_value = CAL_CRYPTO_API_ERROR;
                        break;
                    }
            }

#ifdef DEBUG
            printf("random key : ");
            for (i=0; i<key_bytes; i++) {
                printf("%02x ", key[i]);
            }
            printf("\n");
#endif
            if (cert_file!=NULL) {
                /* Encrypt key using cert file and save it in the key_file */
                err_value = encrypt_dek_key(key, key_bytes, cert_file, key_file);
                if (err_value) {
                    snprintf(err_str, MAX_ERR_STR_BYTES-1,
                            "Failed to encrypt and save key");
                    display_error(err_str);
                    err_value = CAL_CRYPTO_API_ERROR;
                    break;
                }
            } else {
                /* Save key in the key_file */
                err_value = write_plaintext_dek_key(key, key_bytes, cert_file, key_file);
                if (err_value) {
                    snprintf(err_str, MAX_ERR_STR_BYTES-1,
                            "Failed to save key");
                    display_error(err_str);
                    err_value = CAL_CRYPTO_API_ERROR;
                    break;
                }
            }

            key_init_done = 1;
        }

        /* Get the size of in_file */
        fh = fopen(in_file, "rb");
        if (fh == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                     "Unable to open binary file %s", in_file);
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        fseek(fh, 0, SEEK_END);
        file_size = ftell(fh);
        plaintext = (unsigned char*)malloc(file_size);;
        if (plaintext == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                         "Not enough allocated memory" );
            display_error(err_str);
            err_value = CAL_CRYPTO_API_ERROR;
            break;
        }
        fclose(fh);

        fh = fopen(in_file, "rb");
        if (fh == NULL) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                         "Cannot open file %s", in_file);
            display_error(err_str);
            err_value = CAL_FILE_NOT_FOUND;
            break;
        }

        /* Read encrypted data into input_buffer */
        bytes_read = fread(plaintext, 1, file_size, fh);
        fclose(fh);
        /* Reached EOF? */
        if (bytes_read == 0) {
            snprintf(err_str, MAX_ERR_STR_BYTES-1,
                         "Cannot read file %s", out_file);
            display_error(err_str);
            err_value = CAL_FILE_NOT_FOUND;
           break;
        }

        if (AES_CCM == aead_alg) { /* HAB4 */
            err_value = encryptccm(plaintext, file_size, aad, aad_bytes,
                                key, key_bytes, nonce, nonce_bytes, out_file,
                                mac, mac_bytes, &err_value, err_str);
        }
        else if (AES_CBC == aead_alg) { /* AHAB */
            err_value = encryptcbc(plaintext, file_size, key, key_bytes, nonce,
                                   out_file, &err_value, err_str);
        }
        else {
            err_value = CAL_INVALID_ARGUMENT;
        }
        if (err_value == CAL_NO_CRYPTO_API_ERROR) {
            printf("Encryption not enabled\n");
            break;
        }
    } while(0);

    free(plaintext);

    /* Clean up */
    return err_value;
}
