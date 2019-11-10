#ifndef SERVERREPLY_P_H
#define SERVERREPLY_P_H

#include <QString>

namespace SimpleMail {

class ServerReplyPrivate
{
public:
    QString responseText;
    int responseCode = 0;
    bool error = false;
};

}

#endif // SERVERREPLY_P_H
