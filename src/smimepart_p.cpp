#include "smimepart_p.h"

using namespace SimpleMail;

SMimePartPrivate::~SMimePartPrivate()
{
    _input          = nullptr;
    _privateKey     = nullptr;
    _certificate    = nullptr;
    _certificateCA  = nullptr;
    _recipsReceiver = nullptr;
}
