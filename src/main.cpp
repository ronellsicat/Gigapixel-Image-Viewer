#include <QApplication>

#include "mainapplication.h"

int main(int argc, char *argv[]) {
  
  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QApplication app(argc, argv);
  app.setOrganizationName("KAUST");
  app.setApplicationName("GigaPatchExplorer");

  MainApplication main_application;
  main_application.show();
  
  return app.exec();
}