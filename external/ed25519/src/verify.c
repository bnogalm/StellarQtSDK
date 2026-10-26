#include "ed25519.h"
#include "sha512.h"
#include "ge.h"
#include "sc.h"

static int consttime_equal(const unsigned char *x, const unsigned char *y) {
    unsigned char r = 0;

    r = x[0] ^ y[0];
    #define F(i) r |= x[i] ^ y[i]
    F(1);
    F(2);
    F(3);
    F(4);
    F(5);
    F(6);
    F(7);
    F(8);
    F(9);
    F(10);
    F(11);
    F(12);
    F(13);
    F(14);
    F(15);
    F(16);
    F(17);
    F(18);
    F(19);
    F(20);
    F(21);
    F(22);
    F(23);
    F(24);
    F(25);
    F(26);
    F(27);
    F(28);
    F(29);
    F(30);
    F(31);
    #undef F

    return !r;
}

/* 1 when the little-endian scalar s is below the group order
   L = 2^252 + 27742317777372353535851937790883648493, as libsodium's
   sc25519_is_canonical. S is public: no need for constant time. */
static int sc_is_canonical(const unsigned char *s) {
    static const unsigned char L[32] = {
        0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58,
        0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
    };
    int i;

    for (i = 31; i >= 0; i--) {
        if (s[i] < L[i]) {
            return 1;
        }
        if (s[i] > L[i]) {
            return 0;
        }
    }
    return 0;
}

/* 1 when the encoding s holds a y below p = 2^255 - 19 (the sign bit
   ignored): libsodium's ge25519_is_canonical (crypto_core/ed25519/ref10/
   ed25519_ref10.c). ge_frombytes_negate_vartime does not check it: it
   reduces y mod p, so y and y + p were the same key with two account ids. */
static int ge_is_canonical(const unsigned char *s) {
    unsigned char c;
    unsigned char d;
    unsigned int  i;

    c = (unsigned char) ((s[31] & 0x7f) ^ 0x7f);
    for (i = 30; i > 0; i--) {
        c |= (unsigned char) (s[i] ^ 0xff);
    }
    c = (unsigned char) ((((unsigned int) c) - 1U) >> 8);
    d = (unsigned char) ((0xed - 1U - (unsigned int) s[0]) >> 8);

    return 1 - (c & d & 1);
}

/* 1 when the encoding s (the sign bit ignored) is one of the points of
   order 1, 2, 4 or 8, or the non-canonical p and p + 1 forms of two of
   them: libsodium's ge25519_has_small_order (ed25519_ref10.c), with its
   blocklist. */
static int ge_has_small_order(const unsigned char *s) {
    static const unsigned char blocklist[7][32] = {
        /* 0 (order 4) */
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        /* 1 (order 1) */
        { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        /* 2707385501144840649318225287225658788936804267575313519463743609750303402022
           (order 8) */
        { 0x26, 0xe8, 0x95, 0x8f, 0xc2, 0xb2, 0x27, 0xb0,
          0x45, 0xc3, 0xf4, 0x89, 0xf2, 0xef, 0x98, 0xf0,
          0xd5, 0xdf, 0xac, 0x05, 0xd3, 0xc6, 0x33, 0x39,
          0xb1, 0x38, 0x02, 0x88, 0x6d, 0x53, 0xfc, 0x05 },
        /* 55188659117513257062467267217118295137698188065244968500265048394206261417927
           (order 8) */
        { 0xc7, 0x17, 0x6a, 0x70, 0x3d, 0x4d, 0xd8, 0x4f,
          0xba, 0x3c, 0x0b, 0x76, 0x0d, 0x10, 0x67, 0x0f,
          0x2a, 0x20, 0x53, 0xfa, 0x2c, 0x39, 0xcc, 0xc6,
          0x4e, 0xc7, 0xfd, 0x77, 0x92, 0xac, 0x03, 0x7a },
        /* p - 1 (order 2) */
        { 0xec, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f },
        /* p (= 0, order 4) */
        { 0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f },
        /* p + 1 (= 1, order 1) */
        { 0xee, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f }
    };
    unsigned char c[7] = { 0 };
    unsigned int  k;
    size_t        i, j;

    for (j = 0; j < 31; j++) {
        for (i = 0; i < 7; i++) {
            c[i] |= (unsigned char) (s[j] ^ blocklist[i][j]);
        }
    }
    for (i = 0; i < 7; i++) {
        c[i] |= (unsigned char) ((s[j] & 0x7f) ^ blocklist[i][j]);
    }
    k = 0;
    for (i = 0; i < 7; i++) {
        k |= ((unsigned int) c[i]) - 1U;
    }
    return (int) ((k >> 8) & 1);
}

int ed25519_verify(const unsigned char *signature, const unsigned char *message, size_t message_len, const unsigned char *public_key) {
    unsigned char h[64];
    unsigned char checker[32];
    sha512_context hash;
    ge_p3 A;
    ge_p2 R;

    /* S below L, as libsodium (stellar-core), js and py require (QtStellar
       change to orlp/ed25519): with only S < 2^253 checked, (R, S + L)
       verified wherever (R, S) did, a second signature anybody could make
       from the first and which the network refuses. */
    if ((signature[63] & 224) || !sc_is_canonical(signature + 32)) {
        return 0;
    }

    /* R not of small order, as libsodium's crypto_sign_ed25519_verify_detached
       (stellar-core) and py; js 17.1.0 accepts it, core wins (QtStellar
       change to orlp/ed25519). With S = h * a, R = the identity verified:
       a signature only the key holder could make, which the network refuses
       (tx_bad_auth) though this SDK called it valid. */
    if (ge_has_small_order(signature)) {
        return 0;
    }

    /* A canonical and not of small order, as libsodium's
       crypto_sign_ed25519_verify_detached (crypto_sign/ed25519/ref10/open.c),
       through which stellar-core verifies, and js and py (QtStellar change
       to orlp/ed25519). With A of order 1, 2, 4 or 8, [h]A is one of 8
       points, so anybody could sign for GAAQ...HV4, GAAAA...WHF and the
       other such account ids: R = [S]B plus the right one of them. */
    if (!ge_is_canonical(public_key) || ge_has_small_order(public_key)) {
        return 0;
    }

    if (ge_frombytes_negate_vartime(&A, public_key) != 0) {
        return 0;
    }

    sha512_init(&hash);
    sha512_update(&hash, signature, 32);
    sha512_update(&hash, public_key, 32);
    sha512_update(&hash, message, message_len);
    sha512_final(&hash, h);
    
    sc_reduce(h);
    ge_double_scalarmult_vartime(&R, h, &A, signature + 32);
    ge_tobytes(checker, &R);

    if (!consttime_equal(checker, signature)) {
        return 0;
    }

    return 1;
}
