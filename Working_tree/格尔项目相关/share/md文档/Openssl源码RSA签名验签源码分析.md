# Openssl源码RSA签名验签源码分析

使用的源码版本为:1.1.1l

RSA加解密原理基本都清楚，使用公钥或者私钥进行加解密。

这里主要解决几个问题，openssl中使用RSA的私钥进行签名时运算了多少次？验签是运算了多少次？

## 源码追踪

1，首先在`/crypto/rsa/rsa.h`文件中找到相关联几个RSA函数的定义。

```c
int RSA_public_encrypt(int flen, const unsigned char *from,
                       unsigned char *to, RSA *rsa, int padding);
int RSA_private_encrypt(int flen, const unsigned char *from,
                        unsigned char *to, RSA *rsa, int padding);
int RSA_public_decrypt(int flen, const unsigned char *from,
                       unsigned char *to, RSA *rsa, int padding);
int RSA_private_decrypt(int flen, const unsigned char *from,
                        unsigned char *to, RSA *rsa, int padding);
```

2，然后再`/crypto/rsa/rsa_crpt.c`中找到了对应函数的实现，以函数`RSA_private_encrypt`为例

```c
int RSA_private_encrypt(int flen, const unsigned char *from,
                        unsigned char *to, RSA *rsa, int padding)
{
#ifdef OPENSSL_FIPS
    if (FIPS_mode() && !(rsa->meth->flags & RSA_FLAG_FIPS_METHOD)
        && !(rsa->flags & RSA_FLAG_NON_FIPS_ALLOW)) {
        RSAerr(RSA_F_RSA_PRIVATE_ENCRYPT, RSA_R_NON_FIPS_RSA_METHOD);
        return -1;
    }
#endif
    return (rsa->meth->rsa_priv_enc(flen, from, to, rsa, padding));
}
```

3,定位到`/crypto/rsa/rsa.h`中`rsa_priv_enc`的定义处，其定义如下：

```c
struct rsa_meth_st {
    const char *name;
    int (*rsa_pub_enc) (int flen, const unsigned char *from,
                        unsigned char *to, RSA *rsa, int padding);
    int (*rsa_pub_dec) (int flen, const unsigned char *from,
                        unsigned char *to, RSA *rsa, int padding);
    int (*rsa_priv_enc) (int flen, const unsigned char *from,
                         unsigned char *to, RSA *rsa, int padding);
    int (*rsa_priv_dec) (int flen, const unsigned char *from,
                         unsigned char *to, RSA *rsa, int padding);
```

4,查找结构体`rsa_meth_st`对应的函数变量，从而找到`rsa_priv_enc`的具体实现，在`include\openssl\ossl_typ.h`中找到了对应的函数变量：

```c
typedef struct rsa_meth_st RSA_METHOD;
```

5,在`engines/e_dasync.c`里面查找到`RSA_METHOD`的set方法。

```c
static RSA_METHOD *dasync_rsa_method = NULL;
...
static int bind_dasync(ENGINE *e)
{
    /* Setup RSA_METHOD */
    if ((dasync_rsa_method = RSA_meth_new("Dummy Async RSA method", 0)) == NULL
        || RSA_meth_set_pub_enc(dasync_rsa_method, dasync_pub_enc) == 0
        || RSA_meth_set_pub_dec(dasync_rsa_method, dasync_pub_dec) == 0
        || RSA_meth_set_priv_enc(dasync_rsa_method, dasync_rsa_priv_enc) == 0
        || RSA_meth_set_priv_dec(dasync_rsa_method, dasync_rsa_priv_dec) == 0
        || RSA_meth_set_mod_exp(dasync_rsa_method, dasync_rsa_mod_exp) == 0
        || RSA_meth_set_bn_mod_exp(dasync_rsa_method, BN_mod_exp_mont) == 0
        || RSA_meth_set_init(dasync_rsa_method, dasync_rsa_init) == 0
        || RSA_meth_set_finish(dasync_rsa_method, dasync_rsa_finish) == 0) {
        DASYNCerr(DASYNC_F_BIND_DASYNC, DASYNC_R_INIT_FAILED);
        return 0;
    }
···

static int dasync_rsa_priv_dec(int flen, const unsigned char *from,
                      unsigned char *to, RSA *rsa, int padding)
{
    /* Ignore errors - we carry on anyway */
    dummy_pause_job();
    return RSA_meth_get_priv_dec(RSA_PKCS1_OpenSSL())
        (flen, from, to, rsa, padding);
}
```

6，在`/crypto/rsa/rsa.ossl.c`中对`RSA_PKCS1_OpenSSL`函数进行定义，其定义如下：

```c
const RSA_METHOD *RSA_PKCS1_OpenSSL(void)
{
    return &rsa_pkcs1_ossl_meth;
}
...
static RSA_METHOD rsa_pkcs1_ossl_meth = {
    "OpenSSL PKCS#1 RSA",
    rsa_ossl_public_encrypt,
    rsa_ossl_public_decrypt,     /* signature verification */
    rsa_ossl_private_encrypt,    /* signing */
    rsa_ossl_private_decrypt,
    rsa_ossl_mod_exp,
    BN_mod_exp_mont,            /* XXX probably we should not use Montgomery
                                 * if e == 3 */
    rsa_ossl_init,
    rsa_ossl_finish,
    RSA_FLAG_FIPS_METHOD,       /* flags */
    NULL,
    0,                          /* rsa_sign */
    0,                          /* rsa_verify */
    NULL,                       /* rsa_keygen */
    NULL                        /* rsa_multi_prime_keygen */
};
```

7,在这一步已经找到具体的实现函数，`rsa_ossl_public_encrypt,rsa_ossl_public_decrypt,rsa_ossl_private_decrypt,rsa_ossl_mod_exp`以`rsa_ossl_private_encrypt`为例，

```c
struct bignum_st {
    BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
                                 * chunks. */
    int top;                    /* Index of last used d +1. */
    /* The next are internal book keeping for bn_expand. */
    int dmax;                   /* Size of the d array. */
    int neg;                    /* one if the number is negative */
    int flags;
};
```



```c
static int rsa_ossl_private_encrypt(int flen, const unsigned char *from,
                                   unsigned char *to, RSA *rsa, int padding)
{
    BIGNUM *f, *ret, *res;//大数其定义在bn_local.h中的bignum_st中
    int i, num = 0, r = -1;
    unsigned char *buf = NULL;
    BN_CTX *ctx = NULL;
    int local_blinding = 0;
    /*
     * Used only if the blinding structure is shared. A non-NULL unblind
     * instructs rsa_blinding_convert() and rsa_blinding_invert() to store
     * the unblinding factor outside the blinding structure.
     */
    BIGNUM *unblind = NULL;
    BN_BLINDING *blinding = NULL;

    if ((ctx = BN_CTX_new()) == NULL)
        goto err;
    BN_CTX_start(ctx);
    f = BN_CTX_get(ctx);//获取临时BIGNUM变量
    ret = BN_CTX_get(ctx);
    num = BN_num_bytes(rsa->n);//返回值为这个大数实际需要的字节数
    buf = OPENSSL_malloc(num);//分配内存空间
    if (ret == NULL || buf == NULL) {
        RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, ERR_R_MALLOC_FAILURE);
        goto err;
    }

    switch (padding) {
    case RSA_PKCS1_PADDING:
        i = RSA_padding_add_PKCS1_type_1(buf, num, from, flen);
        break;
    case RSA_X931_PADDING:
        i = RSA_padding_add_X931(buf, num, from, flen);
        break;
    case RSA_NO_PADDING:
        i = RSA_padding_add_none(buf, num, from, flen);
        break;
    case RSA_SSLV23_PADDING:
    default:
        RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, RSA_R_UNKNOWN_PADDING_TYPE);
        goto err;
    }
    if (i <= 0)
        goto err;

    if (BN_bin2bn(buf, num, f) == NULL)//将字节数组转化为bn,其原型是BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret);
        goto err;

    if (BN_ucmp(f, rsa->n) >= 0) {
        /* usually the padding functions would catch this */
        RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT,
               RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
        goto err;
    }

    if (rsa->flags & RSA_FLAG_CACHE_PUBLIC)
        if (!BN_MONT_CTX_set_locked(&rsa->_method_mod_n, rsa->lock,
                                    rsa->n, ctx))
            goto err;

    if (!(rsa->flags & RSA_FLAG_NO_BLINDING)) {
        blinding = rsa_get_blinding(rsa, &local_blinding, ctx);
        if (blinding == NULL) {
            RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, ERR_R_INTERNAL_ERROR);
            goto err;
        }
    }

    if (blinding != NULL) {
        if (!local_blinding && ((unblind = BN_CTX_get(ctx)) == NULL)) {
            RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, ERR_R_MALLOC_FAILURE);
            goto err;
        }
        if (!rsa_blinding_convert(blinding, f, unblind, ctx))
            goto err;
    }

    if ((rsa->flags & RSA_FLAG_EXT_PKEY) ||
        (rsa->version == RSA_ASN1_VERSION_MULTI) ||
        ((rsa->p != NULL) &&
         (rsa->q != NULL) &&
         (rsa->dmp1 != NULL) && (rsa->dmq1 != NULL) && (rsa->iqmp != NULL))) {
        if (!rsa->meth->rsa_mod_exp(ret, f, rsa, ctx))
            goto err;
    } else {
        BIGNUM *d = BN_new();
        if (d == NULL) {
            RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, ERR_R_MALLOC_FAILURE);
            goto err;
        }
        if (rsa->d == NULL) {
            RSAerr(RSA_F_RSA_OSSL_PRIVATE_ENCRYPT, RSA_R_MISSING_PRIVATE_KEY);
            BN_free(d);
            goto err;
        }
        BN_with_flags(d, rsa->d, BN_FLG_CONSTTIME);

        if (!rsa->meth->bn_mod_exp(ret, f, d, rsa->n, ctx,
                                   rsa->_method_mod_n)) {
            BN_free(d);
            goto err;
        }
        /* We MUST free d before any further use of rsa->d */
        BN_free(d);
    }

    if (blinding)
        if (!rsa_blinding_invert(blinding, ret, unblind, ctx))
            goto err;

    if (padding == RSA_X931_PADDING) {
        if (!BN_sub(f, rsa->n, ret))
            goto err;
        if (BN_cmp(ret, f) > 0)
            res = f;
        else
            res = ret;
    } else {
        res = ret;
    }

    /*
     * BN_bn2binpad puts in leading 0 bytes if the number is less than
     * the length of the modulus.
     */
    r = BN_bn2binpad(res, to, num);
 err:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
    OPENSSL_clear_free(buf, num);
    return r;
}
```

核心调用在于这一句

```c
if (!rsa->meth->bn_mod_exp(ret, f, d, rsa->n, ctx,
                                   rsa->_method_mod_n)) {
            BN_free(d);
            goto err;
        }
```

`bn_mod_exp`是大整数模幂运算，在`/crypto/bn/bn_exp.c`中实现

```c
int BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p, const BIGNUM *m,
               BN_CTX *ctx)
{
    int ret;

    bn_check_top(a);
    bn_check_top(p);
    bn_check_top(m);

    /*-
     * For even modulus  m = 2^k*m_odd, it might make sense to compute
     * a^p mod m_odd  and  a^p mod 2^k  separately (with Montgomery
     * exponentiation for the odd part), using appropriate exponent
     * reductions, and combine the results using the CRT.
     *
     * For now, we use Montgomery only if the modulus is odd; otherwise,
     * exponentiation using the reciprocal-based quick remaindering
     * algorithm is used.
     *
     * (Timing obtained with expspeed.c [computations  a^p mod m
     * where  a, p, m  are of the same length: 256, 512, 1024, 2048,
     * 4096, 8192 bits], compared to the running time of the
     * standard algorithm:
     *
     *   BN_mod_exp_mont   33 .. 40 %  [AMD K6-2, Linux, debug configuration]
     *                     55 .. 77 %  [UltraSparc processor, but
     *                                  debug-solaris-sparcv8-gcc conf.]
     *
     *   BN_mod_exp_recp   50 .. 70 %  [AMD K6-2, Linux, debug configuration]
     *                     62 .. 118 % [UltraSparc, debug-solaris-sparcv8-gcc]
     *
     * On the Sparc, BN_mod_exp_recp was faster than BN_mod_exp_mont
     * at 2048 and more bits, but at 512 and 1024 bits, it was
     * slower even than the standard algorithm!
     *
     * "Real" timings [linux-elf, solaris-sparcv9-gcc configurations]
     * should be obtained when the new Montgomery reduction code
     * has been integrated into OpenSSL.)
     */

#define MONT_MUL_MOD
#define MONT_EXP_WORD
#define RECP_MUL_MOD

#ifdef MONT_MUL_MOD
    if (BN_is_odd(m)) {
# ifdef MONT_EXP_WORD
        if (a->top == 1 && !a->neg
            && (BN_get_flags(p, BN_FLG_CONSTTIME) == 0)
            && (BN_get_flags(a, BN_FLG_CONSTTIME) == 0)
            && (BN_get_flags(m, BN_FLG_CONSTTIME) == 0)) {
            BN_ULONG A = a->d[0];
            ret = BN_mod_exp_mont_word(r, A, p, m, ctx, NULL);
        } else
# endif
            ret = BN_mod_exp_mont(r, a, p, m, ctx, NULL);
    } else
#endif
#ifdef RECP_MUL_MOD
    {
        ret = BN_mod_exp_recp(r, a, p, m, ctx);
    }
#else
    {
        ret = BN_mod_exp_simple(r, a, p, m, ctx);
    }
#endif

    bn_check_top(r);
    return ret;
}
```

9，在模幂运算里如果模数是偶数使用蒙哥马利快速模幂算法`BN_mod_exp_mont`，否则使用基于`reciprocal`的快速模幂算法`BN_mod_exp_recp`。

