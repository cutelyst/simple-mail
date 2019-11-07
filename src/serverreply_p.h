#ifndef SERVERREPLY_P_H
#define SERVERREPLY_P_H

#include <QString>

namespace SimpleMail {

class ServerReplyPrivate
{
public:
    QString errorText;
    bool error = false;
};

}

#endif // SERVERREPLY_P_H
