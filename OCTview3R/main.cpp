//Qt includes
#include <QApplication>
#include "OCTview3R.h"

int main(int argc, char** argv )
{
  QApplication app(argc, argv);
  OCTview3R appOCTview3R;
  appOCTview3R.show();
  return app.exec();
}


