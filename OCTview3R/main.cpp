//Qt includes
#include <QApplication>
#include <QCoreApplication>
#include "OCTview3R.h"

int main(int argc, char** argv )
{
  QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName(QStringLiteral("UKE"));
  QCoreApplication::setApplicationName(QStringLiteral("OCTview3R"));
  QCoreApplication::setApplicationVersion(QStringLiteral("1.1.0"));
  OCTview3R appOCTview3R;
  appOCTview3R.show();
  return app.exec();
}
