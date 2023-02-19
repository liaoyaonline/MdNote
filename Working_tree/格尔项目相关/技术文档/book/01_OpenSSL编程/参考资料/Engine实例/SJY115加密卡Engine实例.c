#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <openssl/obj_mac.h>
#include <openssl/pem.h>
#include <openssl/dso.h>
#include <openssl/engine.h>
#include <openssl/crypto.h>
#include <openssl/bn.h>
#include <openssl/buffer.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#define ENTERFUNC(func) 

#define EXITFUNC(func) 

const char *engine_SJY115_id = "SJY115-F";
const char *engine_SJY115_name = "SJY115-F PCI hardware engine support";

int semid;

#define ENGINE_CMD_SET_LOGGER		(ENGINE_CMD_BASE  + 1)

BIO *bLog = NULL;

#define ENGINELOG(str)   \
    if (bLog != NULL)   \
    { \
        BIO_write(blog, #str); \
    }

static void hexdump(const unsigned char *b, int n)
{
    int i;
    printf("KOAL KEY: \n");
    for (i = 0; i < n; ++i) {
        printf("0x%02x,", b[i]);
        if ((i & 7) == 7)
            printf("\n");
    }
    printf("\n");
}

static int engine_cipher_nids[] = { 
	NID_scb2_ecb			//新算法ID，模式是ECB
	// NID_rc4
};
static int engine_cipher_nids_num = sizeof(engine_cipher_nids)/sizeof(engine_cipher_nids[0]);


static const ENGINE_CMD_DEFN SJY115_cmd_defns[] = 
{
	{	ENGINE_CMD_SET_LOGGER,
		"ENGINE_SET_LOGGER",
		"set log bio the path to the 'SJY115' shared library",
		ENGINE_CMD_FLAG_STRING	},
	{	0,		NULL, 		NULL, 		0	}
};


typedef struct {
 	unsigned char szKey[16];
} sjy115_crypto_state;


// forward declaration
static int SJY115_init(ENGINE *e);
static int SJY115_finish(ENGINE *e);
static int sjy115_engine_ciphers(ENGINE *e, const EVP_CIPHER **cipher, const int **nids, int nid);
static int sjy115_cipher_init(EVP_CIPHER_CTX *ctx, const unsigned char *key, const unsigned char *iv, int enc);
static int sjy115_cipher_scb2( EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, unsigned int inl);
static int sjy115_cipher_cleanup(EVP_CIPHER_CTX *ctx);
static int bind_SJY115(ENGINE *e);
static int bind_helper(ENGINE *e, const char *id);


static int SJY115_ctrl(ENGINE *e, int cmd, long i, void *p, void (*f)())
{

    switch(cmd)
    {
        case ENGINE_CMD_SET_LOGGER:

        if(p == NULL)
        {
        	return 0;
        }
        else
        {
            bLog = (BIO*)p;
            return 0;
        }
            
        default:
            break;
    }

    return 0;
}


// use the key generate scb2key
static int sjy115_cipher_init(EVP_CIPHER_CTX *ctx, const unsigned char *key, const unsigned char *iv, int enc)
{
    BIO_printf(bLog, "sjy115_cipher_init-- enter\n");
    ENTERFUNC(sjy115_cipher_init);
	
    unsigned char keyValue[16] = {0};
	
    hexdump(key, 16);
    memset(keyValue, 0x00, 16);
    memcpy(keyValue, key, 16);

    sjy115_crypto_state *state = (sjy115_crypto_state *)ctx->cipher_data;
    memcpy(state->szKey, key, 16);
    
err:

    EXITFUNC(sjy115_cipher_init);
    return 1;
}


/*
 * 功能函数，socket发送
 * conn socket连接
 * szBuf 待发送数据
 * iBufLen 待发送数据的长度
 * 成功返回0，失败返回-1
**/
int writen(int conn, unsigned char *szBuf, int iBufLen)
{	
	while(iBufLen > 0) {
		int nWritten = write(conn, szBuf, iBufLen);
		if(nWritten <= 0) {
			if( nWritten<0 && errno==EINTR) {
				nWritten = 0;
			}
			else {
				return -1;
			}
		}
		iBufLen -= nWritten;
		szBuf += nWritten;
	}

	return 0;
}



/*
 * 功能函数，socket接收
 * conn socket连接
 * szBuf 接收缓存
 * iBufLen 接收缓存长度
 * 成功返回接收到的字节数，失败返回-1
**/
int readn(int conn, unsigned char *szBuf, int iBufLen)
{	
	int nLeft = iBufLen;
	while(nLeft > 0) {
		int nRead = read(conn, szBuf, nLeft);
		if(nRead < 0) {
			if( errno==EINTR) {
				nRead = 0;
			}
			else {
				return -1;
			}
		}
		else if(nRead == 0)			//EOF
			break;
		
		nLeft  -= nRead;
		szBuf += nRead;
	}

	return iBufLen-nLeft;
}


static int sjy115_cipher_scb2( EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, unsigned int inl)
{
    BIO_printf(bLog, "sjy115_cipher_scb2-- enter\n");
    ENTERFUNC(sjy115_cipher_scb2);

	int rv = 0;

	short int enc = 0;
	long int iDataLen = 0;
	long int iRequestLen = sizeof(enc)+16+sizeof(iDataLen)+inl;
	
	unsigned char *szRequest = (unsigned char *)malloc( iRequestLen );
	unsigned char *p = szRequest;


	//信号量加锁
	int sem_rv = 0;
	struct sembuf op = {0};
    
	op.sem_flg = SEM_UNDO;
	op.sem_num = 0;
	op.sem_op = -1;
	if ( semop( semid, &op, 1) < 0 ) {
		// fprintf( stderr, "KLCRL_verify: 信号量加锁失败%d:%s\n", errno, strerror(errno) );	
		BIO_printf(bLog, "ERROR: DEBUG  SEMOP FAILED--- 1\n");
		sem_rv = 1;
	}
  
    
	int sockfd = 0;
	struct sockaddr_in srv_addr;
	memset(&srv_addr, 0, sizeof(struct sockaddr_in));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(1099);
	if( inet_pton(AF_INET, "127.0.0.1", &srv_addr.sin_addr) <= 0 ) {
	//	fprintf(stderr, "inet_pton error\n");
		goto err;
	}
	
	sockfd = socket (AF_INET, SOCK_STREAM, 0);
	if (connect(sockfd,(struct sockaddr *)&srv_addr,sizeof(srv_addr)) == -1) {
		close(sockfd); 
		perror("connect"); 
		goto err;
	}

	if (ctx->encrypt) {
		enc = 0;
	}
	else {
		enc = 1;
	}

	enc = htons(enc);
	memcpy( p, &enc, sizeof(enc) );
	p += sizeof(enc);

	memcpy( p, ((sjy115_crypto_state *)ctx->cipher_data)->szKey, 16 );
	p += 16;

	iDataLen = htonl(inl);
	memcpy( p, &iDataLen, sizeof(iDataLen) );
	p += sizeof(iDataLen);

	memcpy( p, in, inl );
	p += inl;

	//网络读写
	rv = writen( sockfd, szRequest, iRequestLen);
	if( rv != 0 ) {
		goto err;
	}

	rv = readn( sockfd, out, inl );
	if( rv < inl ) {
		goto err;
	}


err:
	if( sockfd ) 
		close(sockfd);

	if( szRequest )
		free(szRequest);


	//信号量解锁
	if( !sem_rv ) {
		op.sem_flg = SEM_UNDO;
		op.sem_num = 0;
		op.sem_op = 1;
		if ( semop(semid, &op, 1) < 0 ) {
			// fprintf( stderr, "KLCRL_verify: 信号量解锁失败%d:%s\n", errno, strerror(errno) );
			BIO_printf(bLog, "ERROR: DEBUG  SEMOP FAILED--- 2\n");
		}
	}
    
	EXITFUNC(sjy115_cipher_scb2);
	BIO_printf(bLog, "sjy115_cipher_scb2 %d-- exit\n", rv);

	return 1;
}


// clean the object used by this time(one encipher/decipher )
static int sjy115_cipher_cleanup(EVP_CIPHER_CTX *ctx)
{
    // only one secret object
    ENTERFUNC(sjy115_cipher_cleanup);

 err:
    EXITFUNC(sjy115_cipher_cleanup);
    return 1;
}


const EVP_CIPHER sjy115_scb2 = {
	NID_scb2_ecb,	// 新算法ID值 ssl握手协议中不支持(除非在openssl中增加新的加密算法ID)
	16,         // 块长度    scb2 ctx只设置然后调用docipher 
	16,         // 密钥长度  scb2中无用?? 句柄长度??
	0,          // iv长度(CBC加密模式时一般都需要iv，其他加密模式没有)
	EVP_CIPH_ECB_MODE,       // 加密模式
	sjy115_cipher_init,      // 加密初始化函数
	sjy115_cipher_scb2,      // 加密实现函数
	sjy115_cipher_cleanup,   // 加密清除函数
	sizeof(sjy115_crypto_state) ,  // 该值(一般用于存放对称加密密钥，这样就不需要定义全局变量了)
	NULL,
	NULL,
	NULL,                    // ctl
	NULL                     // application data
};


static int sjy115_engine_ciphers(ENGINE *e, const EVP_CIPHER **cipher, const int **nids, int nid)
{
    ENTERFUNC(sjy115_engine_ciphers);
	if (!cipher)	
    {
		/* We are returning a list of supported nids */
		*nids = engine_cipher_nids;
        EXITFUNC(sjy115_engine_ciphers);
		return engine_cipher_nids_num ;
	}

	switch (nid) {
	case NID_scb2_ecb:
		*cipher = &sjy115_scb2;
		break;
	default:
		*cipher = NULL;
		break;
	}
	EXITFUNC(sjy115_engine_ciphers);
	return (*cipher != NULL);
}


static int SJY115_init(ENGINE *e)
{
	int rv = 0;
	
    union semun {
		int val;
		struct semid_fds *buf;
		unsigned short *array;
	};
	union semun ick = {0};

    const int CRL_SEM_KEY = 0x20090514;

    ENTERFUNC(SJY115_init);

	if ((semid = semget(CRL_SEM_KEY, 1, IPC_CREAT | IPC_EXCL | S_IRWXU | S_IRWXG | S_IRWXO )) >= 0) { 
		ick.val = 1;
		if (semctl(semid, 0, SETVAL, ick) < 0)	{
			fprintf( stderr, "[%d]KLCRL_init : 初始化信号量失败:%d:%s\n", getpid(), errno, strerror(errno) );
			rv = -1;
			goto err;
		}
	}
	else if( errno == EEXIST ) {
		if ((semid = semget(CRL_SEM_KEY, 1,  S_IRWXU | S_IRWXG | S_IRWXO )) < 0) { 
			fprintf( stderr, "[%d]KLCRL_init : 获取已存在信号量失败:%d:%s\n", getpid(), errno, strerror(errno) );
			rv = -2;
			goto err;
		}
	}
	else {
		fprintf( stderr, "[%d]KLCRL_init : 获取信号量失败:%d:%s\n", getpid(), errno, strerror(errno) );
		rv = -3;
		goto err;
	}
    
    
    
err:    
    EXITFUNC(SJY115_init);

    return 1;
}

static int SJY115_finish(ENGINE *e)
{
    ENTERFUNC(SJY115_finish);

 err:   
    EXITFUNC(SJY115_finish);
	return 1;
}


static int SJY115_destroy(ENGINE *e)
{
    return 1;
}

static int bind_SJY115(ENGINE *e)
{
    ENTERFUNC(bind_SJY115);
    if ( !ENGINE_set_id(e, engine_SJY115_id)
        || !ENGINE_set_name(e, engine_SJY115_name)
        || !ENGINE_set_ciphers(e, sjy115_engine_ciphers)
		|| !ENGINE_set_init_function(e, SJY115_init)
		|| !ENGINE_set_finish_function(e, SJY115_finish) 
		|| !ENGINE_set_destroy_function(e, SJY115_destroy)
		|| !ENGINE_set_ctrl_function(e, SJY115_ctrl)                   // add for log
		|| !ENGINE_set_cmd_defns(e, SJY115_cmd_defns))
    {
        EXITFUNC(bind_SJY115);
        return 0;
    }
    EXITFUNC(bind_SJY115);
    return 1; // SUCCESS
}

// we always use the dynmic engine so always build this 
static int bind_helper(ENGINE *e, const char *id)
{
    ENTERFUNC(bind_helper);
    // fprintf(stderr, "id = %s \n", id);
    if(id && (strcmp(id, engine_SJY115_id) != 0)) 
    {
        EXITFUNC(bind_helper);
    	return 0;
    }
    if(!bind_SJY115(e))
    {
        EXITFUNC(bind_helper);
    	return 0;
    }
    // fprintf(stderr, "bind_fn:bind_helper() success \n");
    EXITFUNC(bind_helper);
    return 1;
}
// make function bind_engine 
IMPLEMENT_DYNAMIC_CHECK_FN()
IMPLEMENT_DYNAMIC_BIND_FN(bind_helper)

