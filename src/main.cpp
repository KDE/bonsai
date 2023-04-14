
#include <QDate>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>

#include <QApplication>

#include <MauiKit/Core/mauiapp.h>

#include <KAboutData>
#include <KI18n/KLocalizedString>

#include "../bonsai_version.h"

#include "controllers/projectmanager.h"
#include "controllers/branchesmanager.h"
#include "controllers/project.h"
#include "models/projectsmodel.h"

#include <libkommit/models/logsmodel.h>

#define BONSAI_URI "org.maui.bonsai"

Q_DECL_EXPORT int main(int argc, char *argv[])
{    
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

        QApplication app(argc, argv);

        app.setOrganizationName(QStringLiteral("Maui"));
        app.setWindowIcon(QIcon(":/assets/assets/bonsai.svg"));

        KLocalizedString::setApplicationDomain("bonsai");
        KAboutData about(QStringLiteral("bonsai"), i18n("Bonsai"), BONSAI_VERSION_STRING, i18n("Git version control manager."), KAboutLicense::LGPL_V3, i18n("© 2021-%1 Maui Development Team",QString::number(QDate::currentDate().year())), QString(GIT_BRANCH) + "/" + QString(GIT_COMMIT_HASH));

        about.addAuthor(i18n("Camilo Higuita"), i18n("Developer"), QStringLiteral("milo.h@aol.com"));
        about.setHomepage("https://mauikit.org");
        about.setProductName("maui/bonsai");
        about.setBugAddress("https://invent.kde.org/maui/bonsai/-/issues");
        about.setOrganizationDomain(BONSAI_URI);
        about.setProgramLogo(app.windowIcon());
        about.addComponent("libkommit");

        about.addCredit(QStringLiteral("Hamed Masafi"), i18n("Kommit Developer"));

        KAboutData::setApplicationData(about);
        MauiApp::instance()->setIconName("qrc:/assets/assets/bonsai.png");

        QCommandLineParser parser;
        parser.setApplicationDescription(about.shortDescription());
        parser.process(app);
        about.processCommandLine(&parser);

        QQmlApplicationEngine engine;
        const QUrl url(QStringLiteral("qrc:/main.qml"));
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                                         &app, [url](QObject *obj, const QUrl &objUrl)
        {
                if (!obj && url == objUrl)
                        QCoreApplication::exit(-1);

        }, Qt::QueuedConnection);

        engine.rootContext()->setContextObject(new KLocalizedContext(&engine));

        qmlRegisterAnonymousType<ProjectsModel>(BONSAI_URI, 1);
        qmlRegisterAnonymousType<Git::LogsModel>(BONSAI_URI, 1);
        qmlRegisterAnonymousType<BranchesManager>(BONSAI_URI, 1);
        qmlRegisterUncreatableType<StatusMessage>(BONSAI_URI, 1, 0, "StatusMessage", "Can not be created only referenced");
        qmlRegisterType<Project>(BONSAI_URI, 1, 0, "Project");
        qmlRegisterType<ProjectManager>(BONSAI_URI, 1, 0, "ProjectManager");
        qmlRegisterSingletonInstance<GlobalSettings>(BONSAI_URI, 1, 0, "Settings", GlobalSettings::instance());

    engine.load(url);

        return app.exec();
}
