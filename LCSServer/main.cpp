#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include <QJsonObject>

#include "AppService.h"
#include "GlobalDefs.h"
#include "EntityMetadata.h"

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
bool enableDebugMessages = false;

int main(int argc, char *argv[])
{
    EntityMetadata::instance();
    qInstallMessageHandler(messageOutput);
    qRegisterMetaType<ControllerStatusEnum>("ControllerStatusEnum");
    qRegisterMetaType<ControllerClassEnum>("ControllerClassEnum");
    qRegisterMetaType<DeviceClassEnum>("DeviceClassEnum");
    qRegisterMetaType<ModuleClassEnum>("ModuleClassEnum");
    qRegisterMetaType<NetActionType>("NetActionType");
    qRegisterMetaType<QJsonObject>();

    QString name(QObject::tr("LCSServer"));
    QString description(QObject::tr("GCMRR Layout Control System (LCS)"));

    QString s;
    for(int x = 0; x < argc; x++)
    {
        QString s = QString(argv[x]);
        if(s == "-d" || s == "-D")
            enableDebugMessages = true;
    }
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
            qWarning(QObject::tr("Wrong number of parameters!\nUsage: LCSServer.exe [flags] [name description]").toLatin1());
            return 1;
        }
    }

    CAppService service(argc, argv, name, description);

    return service.exec();
}

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if(enableDebugMessages)
    {
        QByteArray localMsg = msg.toLocal8Bit();
        switch (type)
        {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s\n", localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
        default:
            fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        }
    }
}
