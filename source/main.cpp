#include <QApplication>
#include "MainWindow.h"
#include <qdir.h>

int main(int argc, char* argv[])
{
   QApplication app(argc, argv);
   QDir::setCurrent(QCoreApplication::applicationDirPath());
   QIcon icon("C:/Users/szymu/source/repos/CISBenchmark/CISBenchmark/img/logo.png");
   //QIcon icon("../img/logo.png");
   app.setWindowIcon(icon);
   MainWindow mainWindow;
   mainWindow.setWindowTitle("CIS Benchmark");
   mainWindow.show();

   return app.exec();
}