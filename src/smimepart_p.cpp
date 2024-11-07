#include "smimepart_p.h"

#include <openssl/bio.h>

using namespace SimpleMail;

SMimePrivate::~SMimePrivate()
{
    BIO_free(_input);
    EVP_PKEY_free(_privateKey);
    X509_free(_certificate);
    sk_X509_pop_free(_certificateCA, X509_free);
    sk_X509_pop_free(_recipsReceiver, X509_free);
}
