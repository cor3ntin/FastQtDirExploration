#include <QtCore/QCoreApplication>
#include "fastfolderexploration.h"

int main(int argc, char *argv[])
{
    LightFileInfoList list;
    bool success = listEntries(QDir("."), list, QDir::NoDotAndDotDot);
}
