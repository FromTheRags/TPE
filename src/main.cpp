#include <QApplication>
#include "mainwindow.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("TPE");
    app.setApplicationVersion(TPE_VERSION);
    app.setOrganizationName("FromTheRags");

    QString iconPath = getAssetPath("icone.png");
    if (!iconPath.isEmpty()) {
        app.setWindowIcon(QIcon(iconPath));
    }

    MainWindow window;
    window.show();

    return app.exec();
}
