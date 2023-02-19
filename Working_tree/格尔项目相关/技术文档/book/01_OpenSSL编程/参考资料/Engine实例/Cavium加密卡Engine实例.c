#include <stdio.h>
#include <sys/syscall.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/dso.h>
#include <openssl/engine.h>

#ifndef OPENSSL_NO_HW
#ifndef OPENSSL_NO_HW_CAVIUM

#include "cavium_common.h"

#define CAVIUM_LIB_NAME "cavium engine"
#include "engine_cavium_err.h"

static const char *engine_cavium_id = "cavium";
static const char *engine_cavium_name = "Cavium hardware engine support";

/* For OpenSSL Engine Framework	*/
static int cavium_ctrl(ENGINE *e, int cmd, long i, void *p, void (*f)());
static int cavium_destroy(ENGINE *e);
static int cavium_init(ENGINE *e);
static int cavium_finish(ENGINE *e);

typedef struct cavium_crypto_state_t {
	Uint32 dev_id;
	Uint64 context_handle;
	Uint8 iv[EVP_MAX_IV_LENGTH];
	Uint8 key[EVP_MAX_KEY_LENGTH];
} cavium_crypto_state;

typedef struct rsa_config_t {
	int prienc_disabled;
	int pridec_disabled;
	int pubenc_disabled;
	int pubdec_disabled;
} rsa_config;

static int engine_cipher_nids [ ] = { 
//	NID_des_ede3_cbc,
//	NID_rc4,
//	NID_rc4_40,
//	NID_aes_128_cbc, 
//	NID_aes_192_cbc, 
//	NID_aes_256_cbc
} ;
static int engine_cipher_nids_num = sizeof(engine_cipher_nids)/sizeof(engine_cipher_nids[0]);

static int cavium_cipher_init(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc);
static int cavium_cipher_cleanup(EVP_CIPHER_CTX *ctx);

#define CAVIUM_CIPHER( name, nid, blocklen, keylen, ivlen, mode )	\
	static int cavium_cipher_##name(EVP_CIPHER_CTX *ctx,				\
									unsigned char *out,					\
								    const unsigned char *in,			\
								    unsigned int inlen);				\
 																		\
	const EVP_CIPHER cavium_##name = {									\
		nid,															\
		blocklen, 														\
		keylen, 														\
		ivlen,															\
		mode,															\
		cavium_cipher_init,											\
		cavium_cipher_##name,											\
		cavium_cipher_cleanup,										\
		sizeof(cavium_crypto_state),									\
		EVP_CIPHER_set_asn1_iv,											\
		EVP_CIPHER_get_asn1_iv,											\
		NULL															\
	};											\

CAVIUM_CIPHER( des_ede3_cbc, NID_des_ede3_cbc, 8, 24, 8, EVP_CIPH_CBC_MODE )

CAVIUM_CIPHER( rc4, NID_rc4, 1, 16, 0, EVP_CIPH_VARIABLE_LENGTH )

CAVIUM_CIPHER( rc4_40, NID_rc4_40, 1, 5, 0, EVP_CIPH_VARIABLE_LENGTH )

CAVIUM_CIPHER( aes_128_cbc, NID_aes_128_cbc, 16, 16, 16, EVP_CIPH_CBC_MODE )

CAVIUM_CIPHER( aes_192_cbc, NID_aes_192_cbc, 16, 24, 16, EVP_CIPH_CBC_MODE )

CAVIUM_CIPHER( aes_256_cbc, NID_aes_256_cbc, 16, 32, 16, EVP_CIPH_CBC_MODE )

/* RSA stuff */
#ifndef OPENSSL_NO_RSA

static int cavium_rsa_init(RSA *rsa);
static int cavium_rsa_finish(RSA *rsa);

static int cavium_rsa_pub_enc(int flen,const unsigned char *from,unsigned char *to,
			RSA *rsa,int padding);
static int cavium_rsa_pub_dec(int flen,const unsigned char *from,unsigned char *to,
			RSA *rsa,int padding);
static int cavium_rsa_crt_pri_enc(int flen,const unsigned char *from,unsigned char *to,
			RSA *rsa,int padding);
static int cavium_rsa_crt_pri_dec(int flen,const unsigned char *from,unsigned char *to,
			RSA *rsa,int padding);

/* Our internal RSA_METHOD that we provide pointers to */
static RSA_METHOD cavium_rsa =
	{
	"Cavium RSA method",
	cavium_rsa_pub_enc,		/* pub_enc*/
	cavium_rsa_pub_dec,		/* pub_dec*/
	cavium_rsa_crt_pri_enc,	/* pri_enc*/
	cavium_rsa_crt_pri_dec,	/* pri_dec*/
	NULL, 				/* mod_exp*/
	NULL, 				/* mod_exp_mongomery*/
	NULL, 				/* init*/
	NULL,				/* finish*/
	0,				/* RSA flag*/
	NULL, 
	NULL,				/* OpenSSL sign*/
	NULL				/* OpenSSL verify*/
	};

static Uint32 dev_count = 1;
static Uint8 dev_mask = 0;

static int get_dev_id()
{
	/*	获取当前线程ID	*/
	int tid = syscall(SYS_gettid);
	if (tid == 0) {
		tid = getpid();
	}

//	CAVIUM_PRINT( "[Koal Software. LTD] tid=%d, dev_id=%d\n", tid, tid % dev_count );
	
	return tid % dev_count;
}

static rsa_config cavium_rsa_config;

#endif	/*	OPENSSL_NO_RSA	*/

/* Now, to our own code */
static char *ENGINE_LIBNAME = NULL;

static char *get_ENGINE_LIBNAME(void)
	{
	if(ENGINE_LIBNAME)
		return ENGINE_LIBNAME;
	return "cavium";
	}
static void free_ENGINE_LIBNAME(void)
	{
	if(ENGINE_LIBNAME)
		OPENSSL_free((void*)ENGINE_LIBNAME);
	ENGINE_LIBNAME = NULL;
	}
static long set_ENGINE_LIBNAME(const char *name)
	{
	free_ENGINE_LIBNAME();
	ENGINE_LIBNAME = (char *)BUF_strdup(name);
	if (ENGINE_LIBNAME != NULL)
		return 1;
	else
		return 0;
	}

/* The definitions for control commands specific to this engine */
#define ENGINE_CMD_SO_PATH		ENGINE_CMD_BASE
#define ENGINE_CMD_SWITCH_ALG	(ENGINE_CMD_BASE + 1)

static const ENGINE_CMD_DEFN cavium_cmd_defns[] = {
	{	ENGINE_CMD_SO_PATH,
		"SO_PATH",
		"Specifies the path to the 'cavium' shared library",
		ENGINE_CMD_FLAG_STRING},
	{0, NULL, NULL, 0}
	};

/**
 * hex_dump:				以16进制打印缓冲区
 *
 * @param data				缓冲区
 * @param data_cb			长度
 * @param desc				描述
 */
static void hex_dump(const unsigned char *data, int data_cb, const char *desc)
{
   	int i = 0;

	if( desc )
		CAVIUM_PRINT( "%s(size = %d)\n", desc, data_cb);
	else 
		CAVIUM_PRINT( "size = %d\n", data_cb);
    
	for( i =0; i<data_cb; i++ ) {
		if( i>0 && i%32 == 0 )
			CAVIUM_PRINT( "\n" );

		CAVIUM_PRINT( "%02X ",*(data+i) );
	}

	CAVIUM_PRINT( "\n" );
}

static int cavium_init(ENGINE *e)
{
	int cav_ret = 0;
	int i = 0;
	
	CAVIUM_PRINT( "[Koal Software. LTD] Initializing Cavium Engine ...\n" );

	/*	初始化第一块加密卡		*/
	if (cav_ret = Csp1Initialize(CAVIUM_DIRECT, 0)) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_INIT, CAVIUM_R_INIT_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_init: Csp1Initialize failed: %d\n", 0, cav_ret );
		return 0;
	}

	/*	获取剩余的加密卡数量	*/
	if (cav_ret = Csp1GetDevCnt(&dev_count, &dev_mask)) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_INIT, CAVIUM_R_INIT_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_init: Csp1GetDevCnt failed: %d\n", 0, cav_ret );
		return 0;
	} 

	CAVIUM_PRINT( "DEV[%d] cavium_init: Csp1GetDevCnt return %d\n", 0, dev_count );

	/*	打开剩余的加密卡	*/
	for (i = 1; i<dev_count; i++) {
		if (cav_ret = Csp1Initialize(CAVIUM_DIRECT, i)) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_INIT, CAVIUM_R_INIT_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_init: Csp1Initialize failed: %d\n", i, cav_ret );
			return 0;
		}
	}

	memset(&cavium_rsa_config, 0 , sizeof(cavium_rsa_config));
	
	return 1;
}

static int cavium_destroy(ENGINE *e)
{
	int cav_ret = 0;
	int i = 0;
	
	CAVIUM_PRINT( "[Koal Software. LTD] Destroying Cavium Engine ...\n" );

	/*	卸载默认加密卡		*/
	if (cav_ret = Csp1Shutdown(0) ) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_DESTROY, CAVIUM_R_SHUTDOWN_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_destroy: Csp1Shutdown failed: %d\n", 0, cav_ret );
	}

	/*	卸载剩余的加密卡		*/
	for (i = 1; i<dev_count; i++) {
		if (cav_ret = Csp1Shutdown(i)) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_DESTROY, CAVIUM_R_SHUTDOWN_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_destroy: Csp1Shutdown failed: %d\n", i, cav_ret );
		}
	}

	free_ENGINE_LIBNAME();
	ERR_unload_CAVIUM_strings();

	return 1;
}

static int cavium_finish(ENGINE *e)
{
	int cav_ret = 0;
	int i = 0;
	
	CAVIUM_PRINT( "[Koal Software. LTD] Finishing Cavium Engine ...\n" );

	/*	卸载默认加密卡		*/
	if (cav_ret = Csp1Shutdown(0) ) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_FINISH, CAVIUM_R_SHUTDOWN_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_finish: Csp1Shutdown failed: %d\n", 0, cav_ret );
	}
	
	/*	卸载剩余的加密卡		*/
	for (i = 1; i<dev_count; i++) {
		if (cav_ret = Csp1Shutdown(i)) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_DESTROY, CAVIUM_R_SHUTDOWN_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_finish: Csp1Shutdown failed: %d\n", i, cav_ret );
		}
	}
	
	free_ENGINE_LIBNAME();

	return 1;
}

static int cavium_ctrl(ENGINE *e, int cmd, long i, void *p, void (*f)())
{
	const char *start = NULL;
	CAVIUM_PRINT( "[Koal Software. LTD] Process Cavium Engine's ctrl commands ...\n" );

	switch(cmd)
	{
	case ENGINE_CMD_SO_PATH:
		if(p == NULL)
			{
			ENGINEerr(ENGINE_F_ENGINE_CTRL, ERR_R_PASSED_NULL_PARAMETER);
			return 0;
			}
		return set_ENGINE_LIBNAME((const char *)p);
	case ENGINE_CMD_SWITCH_ALG:
		if(p == NULL)
			{
			ENGINEerr(ENGINE_F_ENGINE_CTRL, ERR_R_PASSED_NULL_PARAMETER);
			return 0;
			}
		
		/*	启用硬件私钥/公钥加解密			*/
		if ( (start = strstr(p, "PRIVATEKEY")) ) {
			if (strstr(start, "ENABLE")) {
				CAVIUM_PRINT( "[Koal Software. LTD] Enable PrivateKey Algor in Cavium Engine's\n" );
				cavium_rsa_config.prienc_disabled = 0;
				cavium_rsa_config.pridec_disabled = 0;
			}
			else if (strstr(start, "DISABLE")) {
				CAVIUM_PRINT( "[Koal Software. LTD] Disable PrivateKey Algor in Cavium Engine's\n" );
				cavium_rsa_config.prienc_disabled = 1;
				cavium_rsa_config.pridec_disabled = 1;
			}
		}
		else if ( (start = strstr(p, "PUBLICKEY")) ) {
			if (strstr(start, "ENABLE")) {
				CAVIUM_PRINT( "[Koal Software. LTD] Enable PublicKey Algor in Cavium Engine's\n" );
				cavium_rsa_config.pubenc_disabled = 0;
				cavium_rsa_config.pubdec_disabled = 0;
			}
			else if (strstr(start, "DISABLE")) {
				CAVIUM_PRINT( "[Koal Software. LTD] Disable PublicKey Algor in Cavium Engine's\n" );
				cavium_rsa_config.pubenc_disabled = 1;
				cavium_rsa_config.pubdec_disabled = 1;
			}
		}
			
		return 1;
	default:
		break;
	}

	ENGINEerr(ENGINE_F_ENGINE_CTRL,ENGINE_R_CTRL_COMMAND_NOT_IMPLEMENTED);

	return 0;
}


static int cavium_engine_ciphers(ENGINE *e, const EVP_CIPHER **cipher,
    const int **nids, int nid)
{
	CAVIUM_PRINT( "[Koal Software. LTD] Process Cavium Engine's list ciphers ...\n" );

	if (!cipher)	{
		/* We are returning a list of supported nids */
		*nids = engine_cipher_nids;
		return engine_cipher_nids_num ;
	}

	switch (nid) {
	case NID_des_ede3_cbc:
		*cipher = &cavium_des_ede3_cbc;
		break;
	case NID_rc4:
		*cipher = &cavium_rc4;
		break;
	case NID_rc4_40:
		*cipher = &cavium_rc4_40;
		break;
	case NID_aes_128_cbc:
		*cipher = &cavium_aes_128_cbc;
		break;
	case NID_aes_192_cbc:
		*cipher = &cavium_aes_192_cbc;
		break;
	case NID_aes_256_cbc:
		*cipher = &cavium_aes_256_cbc;
		break;
	default:
		*cipher = NULL;
		break;
	}
	
	return (*cipher != NULL);
}

static int cavium_cipher_init(EVP_CIPHER_CTX *ctx, const unsigned char *key,
    const unsigned char *iv, int enc)
{
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;
	Uint64 context_handle = 0;
	Uint32 request_id = 0;
	int cav_ret = 0;

	state->dev_id = get_dev_id();
	
	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's cipher(%d) init ...\n", 
					state->dev_id, ctx->cipher->nid );

	cav_ret = Csp1AllocContext(CONTEXT_SSL, &context_handle, state->dev_id); 
	if (cav_ret) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_CIPHER_INIT, CAVIUM_R_ALLOC_CONTEXT );
		CAVIUM_PRINT( "DEV[%d] cavium_cipher_init: Csp1AllocContext failed: %d, 0x%X\n", 
				state->dev_id, cav_ret, cav_ret );

		return 0;
	}

	switch (ctx->cipher->nid) {
	case NID_des_ede3_cbc:
//		cav_ret = Csp1Initialize3DES(CAVIUM_BLOCKING, context_handle, (Uint8 *)iv, (Uint8 *)key, &request_id, state->dev_id);
		break;
	case NID_rc4:
		cav_ret = Csp1InitializeRc4(CAVIUM_BLOCKING, context_handle, ctx->key_len, (Uint8 *)key, &request_id, state->dev_id);
		break;
	case NID_rc4_40:
		cav_ret = Csp1InitializeRc4(CAVIUM_BLOCKING, context_handle, ctx->key_len, (Uint8 *)key, &request_id, state->dev_id);
		break;
	case NID_aes_128_cbc:
		cav_ret = Csp1InitializeAES(CAVIUM_BLOCKING, context_handle, AES_128, (Uint8 *)iv, (Uint8 *)key, &request_id, state->dev_id);
		break;
	case NID_aes_192_cbc:
		cav_ret = Csp1InitializeAES(CAVIUM_BLOCKING, context_handle, AES_192, (Uint8 *)iv, (Uint8 *)key, &request_id, state->dev_id);
		break;
	case NID_aes_256_cbc:
		cav_ret = Csp1InitializeAES(CAVIUM_BLOCKING, context_handle, AES_256, (Uint8 *)iv, (Uint8 *)key, &request_id, state->dev_id);
		break;
	default:
		CAVIUM_ERR( CAVIUM_F_CAVIUM_CIPHER_INIT, CAVIUM_R_CIPHER_NOTSUPPORT );
		CAVIUM_PRINT( "DEV[%d] cavium_cipher_init failed: unknown cipher(%d)\n", 
					state->dev_id, ctx->cipher->nid );

		break;
	}

	if (cav_ret) {
		Csp1FreeContext(CONTEXT_SSL, context_handle, state->dev_id);

		CAVIUM_ERR( CAVIUM_F_CAVIUM_CIPHER_INIT, CAVIUM_R_INIT_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_cipher_init: Csp1InitializeCipher(%d) failed: %d, 0x%X\n", 
					state->dev_id, ctx->cipher->nid, 
					cav_ret, cav_ret );

		return 0;
	}

	state->context_handle = context_handle;	
	memcpy( state->iv, iv, ctx->cipher->iv_len );
	memcpy( state->key, key, ctx->cipher->key_len );

	return 1;
}


static int cavium_cipher_cleanup(EVP_CIPHER_CTX *ctx)
{
	int cav_ret = 0;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "[Koal Software. LTD] Process Cavium Engine's cipher(%d) cleanup ...\n", ctx->cipher->nid );

	cav_ret = Csp1FreeContext(CONTEXT_SSL, 
				state->context_handle,
				state->dev_id );

	if (cav_ret) {
		CAVIUM_PRINT( "DEV[%d] cavium_cipher_cleanup: Csp1FreeContext failed: %d, 0x%X\n", 
				state->dev_id, cav_ret, cav_ret );
	}

	return 1;
}

static int cavium_cipher_des_ede3_cbc(	EVP_CIPHER_CTX *ctx, 
											unsigned char *out,
											const unsigned char *in, 
											unsigned int inlen )
{
	int cav_ret = 0;
	unsigned int outlen = 0;
	Uint32 request_id = 0;
	unsigned char iiv[8] = {0};

	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's dec_ede3_cbc cipher ...\n", state->dev_id );


	if (ctx->encrypt) {
		hex_dump( in, inlen, "data before Csp1Encrypt3Des" );
		hex_dump( state->iv, 8, "iv before Csp1Encrypt3Des" );
		cav_ret = Csp1Encrypt3Des(	CAVIUM_BLOCKING,
									state->context_handle,
									CAVIUM_NO_UPDATE, 
									inlen, 
									(Uint8 *)in, 
									(Uint8 *)out, 
									state->iv,
									state->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_DES_EDE3_CBC, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_des_ede3_cbc: Csp1Encrypt3Des failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}

//		outlen = (inlen+8)&~(0x7);
		outlen = inlen;
		hex_dump( out, outlen, "data after Csp1Encrypt3Des" );
		memcpy(state->iv, (Uint8 *)out + outlen - 8, 8);
		hex_dump( state->iv, 8, "iv after Csp1Encrypt3Des" );
	}
	else {
		hex_dump( in, inlen, "data before Csp1Decrypt3Des" );

//		memcpy(state->iv, (Uint8 *)in + inlen - 8, 8);
		hex_dump( state->iv, 8, "iv before Csp1Decrypt3Des" );

		cav_ret = Csp1Decrypt3Des(	CAVIUM_BLOCKING,
									state->context_handle,
									CAVIUM_NO_UPDATE, 
									inlen, 
									(Uint8 *)in, 
									(Uint8 *)out, 
									state->iv,
									state->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_DES_EDE3_CBC, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_des_ede3_cbc: Csp1Decrypt3Des failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}

//		outlen = (inlen+7)&~(0x7);
//		outlen = inlen;
		hex_dump( out, outlen, "data after Csp1Decrypt3Des" );
		hex_dump( state->iv, 8, "iv after Csp1Decrypt3Des" );
	}

	return 1;
}

static int cavium_cipher_rc4(	EVP_CIPHER_CTX *ctx, 
								unsigned char *out,
								const unsigned char *in, 
								unsigned int inlen )
{
	int cav_ret = 0;
	Uint32 request_id;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's rc4 cipher ...\n", state->dev_id );

	if (ctx->encrypt) {
		cav_ret = Csp1EncryptRc4(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_rc4: Csp1EncryptRc4 failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}
	else {
		cav_ret = Csp1EncryptRc4(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									&request_id,
									state->dev_id );

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_rc4: Csp1EncryptRc4 failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}

	return 1;
}

static int cavium_cipher_rc4_40(	EVP_CIPHER_CTX *ctx, 
								unsigned char *out,
								const unsigned char *in, 
								unsigned int inlen )
{
	int cav_ret = 0;
	Uint32 request_id;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's rc4_40 cipher ...\n", state->dev_id );

	if (ctx->encrypt) {
		cav_ret = Csp1EncryptRc4(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4_40, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_rc4_40: Csp1EncryptRc4 failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}
	else {
		cav_ret = Csp1EncryptRc4(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4_40, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_rc4_40: Csp1EncryptRc4 failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}

	return 1;
}

static int cavium_cipher_aes_128_cbc(	EVP_CIPHER_CTX *ctx, 
										unsigned char *out,
										const unsigned char *in, 
										unsigned int inlen )
{
	int cav_ret = 0;
	Uint32 request_id;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's aes_128_cbc cipher ...\n", state->dev_id );

	if (ctx->encrypt) {
		cav_ret = Csp1EncryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_128, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_128_cbc: Csp1EncryptAes failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}
	else {
		cav_ret = Csp1DecryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_128, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_128_cbc: Csp1DecryptAes failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}

	return 1;
}

static int cavium_cipher_aes_192_cbc(	EVP_CIPHER_CTX *ctx, 
										unsigned char *out,
										const unsigned char *in, 
										unsigned int inlen )
{
	int cav_ret = 0;
	Uint32 request_id;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's aes_192_cbc cipher ...\n", state->dev_id );

	if (ctx->encrypt) {
		cav_ret = Csp1EncryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_192, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_192_cbc: Csp1EncryptAesfailed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}
	else {
		cav_ret = Csp1DecryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_192, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_192_cbc: Csp1DecryptAes failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}

	return 1;
}

static int cavium_cipher_aes_256_cbc(	EVP_CIPHER_CTX *ctx, 
										unsigned char *out,
										const unsigned char *in, 
										unsigned int inlen )
{
	int cav_ret = 0;
	Uint32 request_id = 0;
	cavium_crypto_state *state = (cavium_crypto_state *)ctx->cipher_data;

	CAVIUM_PRINT( "DEV[%d] [Koal Software. LTD] Process Cavium Engine's aes_256_cbc cipher ...\n", state->dev_id );

	if (ctx->encrypt) {
		cav_ret = Csp1EncryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_256, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_ENC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_256__cbc: Csp1EncryptAes failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}
	else {
		cav_ret = Csp1DecryptAes(	CAVIUM_BLOCKING,
									((cavium_crypto_state *)ctx->cipher_data)->context_handle,
									CAVIUM_UPDATE, AES_256, ROUNDUP16(inlen), (Uint8 *)in, (Uint8 *)out, 
									((cavium_crypto_state *)ctx->cipher_data)->iv,
									((cavium_crypto_state *)ctx->cipher_data)->key,
									&request_id,
									state->dev_id);

		if (cav_ret) {
			CAVIUM_ERR( CAVIUM_F_CAVIUM_RC4, CAVIUM_R_DEC_FAILED );
			CAVIUM_PRINT( "DEV[%d] cavium_cipher_aes_256__cbc: Csp1DecryptAes failed: %d, 0x%X\n", 
						state->dev_id, cav_ret, cav_ret );
			return 0;
		}
	}

	return 1;
}

/*	RSA Stuff	*/

#ifndef OPENSSL_NO_RSA

static int cavium_rsa_pub_enc(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
{
	unsigned int cav_ret = 0;
	int ret = 0;
	unsigned char *n = NULL, *e = NULL;
	Uint16 modulus_size = 0, exponent_size = 0;
	Uint32 request_id = 0;

	int dev_id = get_dev_id();
	
//	CAVIUM_PRINT( "DEV[%d] Enter cavium_rsa_pub_enc\n", dev_id );
	
	/*	检查算法开关，如果禁用硬件实现则调用OpenSSL的软算法实现	*/
	if (cavium_rsa_config.pubenc_disabled) {
		const RSA_METHOD *meth1 = RSA_PKCS1_SSLeay();
		if (meth1)
		{
			return meth1->rsa_pub_enc(flen, from, to, rsa, padding);
		}
	}

	/*	计算密钥模长	*/
	modulus_size = BN_num_bytes(rsa->n);
	exponent_size = BN_num_bytes(rsa->e);
	
	/*	对n, e进行赋值	*/
	n = (unsigned char *)alloca(modulus_size);
	if (!n)
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->n, n);
	
	e = (unsigned char *)alloca(modulus_size);
	if (!e)
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->e, e);

	/*	call cavium API	*/
//	hex_dump( from, flen, "DEV[%d] data before Csp1Pkcs1v15Enc", dev_id );
	
	/*	Notes: Pub Encode operation use blocktype = BT2	*/
#ifdef MC2
	cav_ret = Csp1Pkcs1v15Enc( CAVIUM_BLOCKING,
						BT2,
						modulus_size, 
						exponent_size, 
						(Uint16)flen, 
						n, 
						e, 
						(Uint8 *)from,
						to,
						&request_id,
						dev_id);
#else
	//TODO
#endif

//	hex_dump( to, modulus_size, "DEV[%d] data after Csp1Pkcs1v15Enc", dev_id );

	if (cav_ret) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_RSA_PUB_ENC, CAVIUM_R_ENC_FAILED );
		CAVIUM_PRINT( "DEV[%d] Csp1Pkcs1v15Enc failed: %d, 0x%X\n", dev_id, cav_ret, cav_ret );
		ret = 0;
	}
	else {
		ret = modulus_size;
	}

cleanup:
//	CAVIUM_PRINT( "DEV[%d] Leave cavium_rsa_pub_enc with %d\n", dev_id, ret );

	return ret;
}

static int cavium_rsa_pub_dec(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
{
	unsigned int cav_ret = 0;
	int ret = 0;
	unsigned char *n = NULL, *e = NULL;
	Uint16 modulus_size = 0, exponent_size = 0;
	Uint16 to_len = 0;
	Uint32 request_id = 0;

	int dev_id = get_dev_id();

//	CAVIUM_PRINT( "DEV[%d] Enter cavium_rsa_pub_dec\n", dev_id );
	
	/*	检查算法开关，如果禁用硬件实现则调用OpenSSL的软算法实现	*/
	if (cavium_rsa_config.pubdec_disabled) {
		const RSA_METHOD *meth1 = RSA_PKCS1_SSLeay();
		if (meth1)
		{
			return meth1->rsa_pub_dec(flen, from, to, rsa, padding);
		}
	}

	/*	计算密钥模长	*/
	modulus_size = BN_num_bytes(rsa->n);
	exponent_size = BN_num_bytes(rsa->e);
	
	/*	对n, e进行赋值	*/
	n = (unsigned char *)alloca(modulus_size);
	if (!n)
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->n, n);
	
	e = (unsigned char *)alloca(modulus_size);
	if (!e)
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->e, e);

	to_len = modulus_size;

//	hex_dump( from, to_len, "data before Csp1Pkcs1v15Dec" );

	/*	Notes: Pub Decode operation use blocktype = BT1	*/
#ifdef MC2
	cav_ret = Csp1Pkcs1v15Dec( CAVIUM_BLOCKING,
						BT1,
						modulus_size, 
						exponent_size, 
						n, 
						e, 
						(Uint8 *)from,
						&to_len,
						to,
						&request_id,
						dev_id);
#else
	//TODO
#endif

//	hex_dump( to, (Uint32)ntohs(to_len), "data after Csp1Pkcs1v15Dec" );

	if (cav_ret) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_RSA_PUB_DEC, CAVIUM_R_DEC_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_rsa_pub_dec: Csp1Pkcs1v15Dec failed: %d, 0x%X\n", 
					dev_id, cav_ret, cav_ret );
		ret = 0;
	}
	else {
		ret = (Uint32)ntohs(to_len);
	}

cleanup:
//	CAVIUM_PRINT( "DEV[%d] Leave cavium_rsa_pub_dec with %d\n", dev_id, ret );

	return ret;
}

/**
 *	私钥运算使用支持中国剩余定理(CRT)的cavium api
 */
static int cavium_rsa_crt_pri_enc(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
{
	unsigned int cav_ret = 0;
	int ret = 0;
	unsigned char *q = NULL, *eq = NULL, *p = NULL, *ep = NULL, *iqmp = NULL;
	Uint16 modulus_size = 0, crt_size = 0;
	Uint32 request_id = 0;

	int dev_id = get_dev_id();

//	CAVIUM_PRINT( "DEV[%d] Enter cavium_rsa_crt_pri_enc\n", dev_id );

	/*	检查算法开关，如果禁用硬件实现则调用OpenSSL的软算法实现	*/
	if (cavium_rsa_config.prienc_disabled) {
		const RSA_METHOD *meth1 = RSA_PKCS1_SSLeay();
		if (meth1)
		{
			return meth1->rsa_priv_enc(flen, from, to, rsa, padding);
		}
	}

	/*	计算长度	*/
	modulus_size = BN_num_bytes(rsa->n);
	crt_size = modulus_size/2;
	
	/*	对q, eq, p, eq, igmp 进行赋值	*/
	q = (unsigned char *)alloca(crt_size);
	if ( !q )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->q, q);
	
	eq = (unsigned char *)alloca(crt_size);
	if ( !eq )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->dmq1, eq);

	p = (unsigned char *)alloca(crt_size);
	if ( !p )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->p, p);

	ep = (unsigned char *)alloca(crt_size);
	if ( !ep )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->dmp1, ep);

	iqmp  = (unsigned char *)alloca(crt_size);
	if ( !iqmp  )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->iqmp, iqmp );

	/*	call cavium API	*/
//	hex_dump( from, flen, "DEV[%d] data before Csp1Pkcs1v15CrtEnc" );
	
	/*	Notes: Pri Encode operation use blocktype = BT1	*/
#ifdef MC2
	cav_ret = Csp1Pkcs1v15CrtEnc(CAVIUM_BLOCKING,
				       BT1,
				       modulus_size, 
					   (Uint16)flen, 
				       q, 
				       eq, 
				       p, 
				       ep, 
				       iqmp, 
				       from,
					to,
					&request_id,
					dev_id);
#else
	//TODO
	
#endif
//	hex_dump( to, modulus_size, "data after Csp1Pkcs1v15CrtEnc" );

	if(cav_ret) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_RSA_PRI_ENC, CAVIUM_R_ENC_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_rsa_crt_pri_enc: Csp1Pkcs1v15CrtEnc failed: %d, 0x%X\n", 
					dev_id, cav_ret, cav_ret );
		ret = 0;
	}
	else {
		ret = modulus_size;
	}

cleanup:
//	CAVIUM_PRINT( "DEV[%d] Leave cavium_rsa_crt_pri_enc with %d\n", dev_id, ret );

	return ret;
}

static int cavium_rsa_crt_pri_dec(int flen, const unsigned char *from,
	     unsigned char *to, RSA *rsa, int padding)
{
	unsigned int cav_ret = 0;
	int ret = 0;
	unsigned char *q = NULL, *eq = NULL, *p = NULL, *ep = NULL, *iqmp = NULL;
	Uint16 modulus_size = 0, crt_size = 0;
	Uint16 to_len = 0;
	Uint32 request_id = 0;

	int dev_id = get_dev_id();

//	CAVIUM_PRINT( "DEV[%d] Enter cavium_rsa_crt_pri_dec\n", dev_id );

	/*	检查算法开关，如果禁用硬件实现则调用OpenSSL的软算法实现	*/
	if (cavium_rsa_config.pridec_disabled) {
		const RSA_METHOD *meth1 = RSA_PKCS1_SSLeay();
		if (meth1)
		{
			return meth1->rsa_priv_dec(flen, from, to, rsa, padding);
		}
	}

	/*	计算长度	*/
	modulus_size = BN_num_bytes(rsa->n);
	crt_size = modulus_size/2;
	
	/*	对q, eq, p, eq, igmp 进行赋值	*/
	q = (unsigned char *)alloca(crt_size);
	if ( !q )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->q, q);
	
	eq = (unsigned char *)alloca(crt_size);
	if ( !eq )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->dmq1, eq);

	p = (unsigned char *)alloca(crt_size);
	if ( !p )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->p, p);

	ep = (unsigned char *)alloca(crt_size);
	if ( !ep )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->dmp1, ep);

	iqmp  = (unsigned char *)alloca(crt_size);
	if ( !iqmp  )
	{
		ret = 0;
		goto cleanup;
	}
	BN_bn2bin(rsa->iqmp, iqmp );

	/*	call cavium API	*/
//	hex_dump( from, flen, "DEV[%d] data before Csp1Pkcs1v15CrtDec", dev_id );
	to_len = modulus_size;
	
	/*	Notes: Pri Encode operation use blocktype = BT1	*/
#ifdef MC2
	cav_ret = Csp1Pkcs1v15CrtDec(CAVIUM_BLOCKING,
				       BT2,
				       modulus_size, 
				       q, 
				       eq, 
				       p, 
				       ep, 
				       iqmp, 
				       from,
					&to_len,
					to,
					&request_id,
					dev_id);
#else
	//TODO
	
#endif
//	hex_dump( to, (Uint32)ntohs(to_len), "DEV[%d] data after cavium_rsa_crt_pri_dec", dev_id );

	if(cav_ret) {
		CAVIUM_ERR( CAVIUM_F_CAVIUM_RSA_PRI_DEC, CAVIUM_R_DEC_FAILED );
		CAVIUM_PRINT( "DEV[%d] cavium_rsa_crt_pri_dec: Csp1Pkcs1v15CrtDec failed: %d, 0x%X\n", 
					dev_id, cav_ret, cav_ret );
		ret = 0;
	}
	else {
		ret = (Uint32)ntohs(to_len);
	}

cleanup:
//	CAVIUM_PRINT( "DEV[%d] Leave cavium_rsa_crt_pri_dec with %d\n", dev_id, ret );

	return ret;
}

#endif	/*	OPENSSL_NO_RSA	*/
/* As this is only ever called once, there's no need for locking
 * (indeed - the lock will already be held by our caller!!!) */

static int bind_cavium(ENGINE *e)
{
#ifndef OPENSSL_NO_RSA
	const RSA_METHOD *meth1;
#endif

	CAVIUM_PRINT( "[Koal Software. LTD] Binding Cavium Engine to openssl engine framework ...\n" );

	if(!ENGINE_set_id(e, engine_cavium_id) ||
		!ENGINE_set_name(e, engine_cavium_name) ||
#ifndef OPENSSL_NO_RSA
		!ENGINE_set_RSA(e, &cavium_rsa) ||
#endif
		!ENGINE_set_ciphers(e, cavium_engine_ciphers) ||
		!ENGINE_set_destroy_function(e, cavium_destroy) ||
		!ENGINE_set_init_function(e, cavium_init) ||
		!ENGINE_set_finish_function(e, cavium_finish) ||
		!ENGINE_set_ctrl_function(e, cavium_ctrl) ||
		!ENGINE_set_cmd_defns(e, cavium_cmd_defns))
	  return 0;

#ifndef OPENSSL_NO_RSA
	/* We know that the "PKCS1_SSLeay()" functions hook properly
	 * to the cswift-specific mod_exp and mod_exp_crt so we use
	 * those functions. NB: We don't use ENGINE_openssl() or
	 * anything "more generic" because something like the RSAref
	 * code may not hook properly, and if you own one of these
	 * cards then you have the right to do RSA operations on it
	 * anyway! */ 
	meth1 = RSA_PKCS1_SSLeay();
	if (meth1)
	{
		cavium_rsa.rsa_mod_exp = meth1->rsa_mod_exp;
		cavium_rsa.bn_mod_exp = meth1->bn_mod_exp;
		
		// 如果以下行被注释掉,表示使用加密卡
		//cavium_rsa.rsa_pub_dec=meth1->rsa_pub_dec;
		//cavium_rsa.rsa_pub_enc=meth1->rsa_pub_enc;
		//cavium_rsa.rsa_priv_dec=meth1->rsa_priv_dec;
		//cavium_rsa.rsa_priv_enc=meth1->rsa_priv_enc;

		cavium_rsa.rsa_sign = meth1->rsa_sign;
		cavium_rsa.rsa_verify = meth1->rsa_verify;

		// cavium_rsa.flags|=RSA_FLAG_SIGN_VER;

		cavium_rsa.init = meth1->init;
		cavium_rsa.finish = meth1->finish;
	}
#endif

	/* Ensure the cavium error handling is set up */
	ERR_load_CAVIUM_strings();
	return 1;
}

#ifdef ENGINE_DYNAMIC_SUPPORT
static int bind_helper(ENGINE *e, const char *id)
	{
	if(id && (strcmp(id, engine_cavium_id) != 0))
		return 0;
	if(!bind_cavium(e))
		return 0;
	return 1;
	}       
IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(bind_helper)
#else
static ENGINE *engine_cavium(void)
	{
	ENGINE *cav_ret = NULL;

	CAVIUM_PRINT( "[Koal Software. LTD] Process engine_cavium\n" );

	cav_ret = ENGINE_new();
	if(!cav_ret)
		return NULL;

	if(!bind_cavium(cav_ret))
		{
		ENGINE_free(cav_ret);
		return NULL;
		}

	return cav_ret;
	}

void ENGINE_load_cavium(void)
	{
	/* Copied from eng_[openssl|dyn].c */
	ENGINE *toadd = NULL;
	
	CAVIUM_PRINT( "[Koal Software. LTD] Process ENGINE_load_cavium\n" );

	toadd = engine_cavium();
	if(!toadd) 
		return;
	
	ENGINE_add(toadd);
	ENGINE_free(toadd);
	ERR_clear_error();
	}
#endif	/* ENGINE_DYNAMIC_SUPPORT */

#endif /* !OPENSSL_NO_HW_CAVIUM */
#endif /* !OPENSSL_NO_HW */



// $Log: engine_cavium_fun.c,v $
// Revision 1.1  2008/02/29 11:07:03  zhangym
// create
//
// Revision 1.3  2007/03/26 07:34:11  yanhm
// *** empty log message ***
//
// Revision 1.1  2005/11/11 12:17:05  fanzj
// 新开发库
// conezxy整理
// fanzj提交
//
// Revision 1.4  2005/10/11 08:59:54  zhangxy
// add CVS style comments
//
