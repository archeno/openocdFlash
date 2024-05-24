#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

void loadStyleSheet(QApplication &app, QString styleSheetPath)
{
    QFile file(styleSheetPath);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream outStream(&file);
        QString styleStr = outStream.readAll();
        file.close();
        app.setStyleSheet(styleStr);
    } else {
        qDebug() << "load styleSheet failed!" << endl;
    }
}
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadStyleSheet(a, ":/styles/main.qss");
    MainWindow w;
    w.show();
    return a.exec();
}
