#include "smimemessage_p.h"

using namespace SimpleMail;

SMimeMessagePrivate::SMimeMessagePrivate()
{
    _smimePart = std::make_shared<SMimePart>();
}
