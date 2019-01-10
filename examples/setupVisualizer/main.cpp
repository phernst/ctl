#include "mainwindow.h"
#include <QApplication>
#include <QObject>
#include <QScreen>

#include "mat/mat.h"
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    // window positioning
    auto windowHeight = 768;
    auto windowWidth = 1024;
    auto xPos = 50;
    auto yPos = (QGuiApplication::screens().first()->geometry().height() - windowHeight) / 2;
    w.setGeometry(xPos, yPos, windowWidth, windowHeight);

    // show it
    w.show();

    return a.exec();
}
