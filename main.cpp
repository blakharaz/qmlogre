#include <QGuiApplication>
#include <QtQuick/QQuickWindow>

#include "ogrecontext.h"

int main(int argc, char **argv)
{

    QQuickWindow::setGraphicsApi(QSGRendererInterface::Software);
    QQuickWindow::setSceneGraphBackend("software");

    QGuiApplication app(argc, argv);

    OgreContext my_app;
    my_app.initApp();
    my_app.setUiSource("qrc://ogreqml/main.qml");
    my_app.startTimer(40);

    return QGuiApplication::exec();
}
