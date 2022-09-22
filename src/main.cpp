
#include <QDate>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QCommandLineParser>

#ifdef Q_OS_ANDROID
#include <QGuiApplication>
#else
#include <QApplication>
#endif

#ifdef Q_OS_MACOS
#include <MauiKit/Core/mauimacos.h>
#endif

#include <MauiKit/Core/mauiapp.h>

#include <KAboutData>
#include <KI18n/KLocalizedString>

#include "../bonsai_version.h"

#include "controllers/gitoperations.h"
#include "controllers/projectmanager.h"
#include "controllers/branchesmanager.h"
#include "controllers/project.h"
#include "models/projectsmodel.h"

#define BONSAI_URI "org.maui.bonsai"

Q_DECL_EXPORT int main(int argc, char *argv[])
{    
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);

#ifdef Q_OS_ANDROID
        QGuiApplication app(argc, argv);
        if (!MAUIAndroid::checkRunTimePermissions({"android.permission.WRITE_EXTERNAL_STORAGE"}))
                return -1;
#else
        QApplication app(argc, argv);
#endif

        app.setOrganizationName(QStringLiteral("Maui"));
        app.setWindowIcon(QIcon(":/assets/assets/bonsai.svg"));
        MauiApp::instance()->setIconName("qrc:/assets/assets/bonsai.svg");

        KLocalizedString::setApplicationDomain("bonsai");
        KAboutData about(QStringLiteral("bonsai"), i18n("Bonsai"), BONSAI_VERSION_STRING, i18n("Quick Git control version manager."), KAboutLicense::LGPL_V3, i18n("© 2021-%1 Maui Development Team",QString::number(QDate::currentDate().year())), QString(GIT_BRANCH) + "/" + QString(GIT_COMMIT_HASH));

        about.addAuthor(i18n("Camilo Higuita"), i18n("Developer"), QStringLiteral("milo.h@aol.com"));
        about.setHomepage("https://mauikit.org");
        about.setProductName("maui/bonsai");
        about.setBugAddress("https://invent.kde.org/maui/bonsai/-/issues");
        about.setOrganizationDomain(BONSAI_URI);
        about.setProgramLogo(app.windowIcon());

        KAboutData::setApplicationData(about);

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

        qmlRegisterType<GitOperations>(BONSAI_URI, 1, 0, "GitOperations");
        qmlRegisterAnonymousType<ProjectsModel>(BONSAI_URI, 1);
        qmlRegisterAnonymousType<CommitHistoryModel>(BONSAI_URI, 1);
        qmlRegisterAnonymousType<BranchesManager>(BONSAI_URI, 1);
        qmlRegisterType<Project>(BONSAI_URI, 1, 0, "Project");
        qmlRegisterType<ProjectManager>(BONSAI_URI, 1, 0, "ProjectManager");

    engine.load(url);

#ifdef Q_OS_MACOS
        //    MAUIMacOS::removeTitlebarFromWindow();
        //    MauiApp::instance()->setEnableCSD(true); //for now index can not handle cloud accounts

#endif
        return app.exec();
}
