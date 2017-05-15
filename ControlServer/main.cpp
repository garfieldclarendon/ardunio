#include <QCoreApplication>
#include "AppService.h"
#include "GlobalDefs.h"

int main(int argc, char *argv[])
{
    qRegisterMetaType<ClassEnum>("ClassEnum");
    qRegisterMetaType<NetActionType>("NetActionType");

    QString name(QObject::tr("ControlServer"));
    QString description(QObject::tr("GCMRR Layout Control Server"));

    QString s;
    if(argc > 2 && s != "-c")
    {

        if(argc == 4)
        {
            QString s = QString(argv[2]);
            if(s != "-d")
            {
                name = argv[2];
                description = argv[3];
                argv[2][0] = '\0';
                argv[3][0] = '\0';
            }
        }
        else if(argc == 3)
        {
            QString s = QString(argv[2]);
            if(s != "-d")
            {
                name = argv[1];
                description = argv[2];
                argv[2][0] = '\0';
            }
        }
        else
        {
            qWarning(QObject::tr("Wrong number of parameters!\nUsage: ControlServer.exe [flags] [name description]").toLatin1());
            return 1;
        }
    }

    CAppService service(argc, argv, name, description);

    return service.exec();
}

