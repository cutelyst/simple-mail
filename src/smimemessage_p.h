#ifndef SMIMEMESSAGE_P_H
#define SMIMEMESSAGE_P_H

#include "mimemessage_p.h"
#include "smimepart.h"

namespace SimpleMail {

class SMimeMessagePrivate : public MimeMessagePrivate
{
public:
    SMimeMessagePrivate();
    std::shared_ptr<SMimePart> _smimePart;
};

} // namespace SimpleMail

#endif // SMIMEMESSAGE_P_H
