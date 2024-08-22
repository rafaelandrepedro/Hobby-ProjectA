//QT
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QPainter>
#include <QPainterPath>
#include <QFontDatabase>
#include <QImage>

#include <QLabel>
#include <QPaintEvent>
#include <QMouseEvent>
#include "mainwindow.h"
#include "customLabel.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.resize(800, 450);

    //w.setWindowFlags(Qt::FramelessWindowHint);

    CustomLabel *label = new CustomLabel();

    label->setAlignment(Qt::AlignCenter);

    w.setCentralWidget(label);

    w.show();
    return a.exec();
}




//set PATH=C:\Qt\6.7.1\mingw_64\bin;%PATH%
//cd C:\Qt\6.7.1\mingw_64\bin
//windeployqt C:\Users\0\Documents\QT\ProjectA\build\Desktop_Qt_6_7_1_MinGW_64_bit-Release
