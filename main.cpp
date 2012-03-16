#include <QtCore/QCoreApplication>
#include <QTextCodec>
#include "fastfolderexploration.h"
#include <QDebug>
#include <QElapsedTimer>

using namespace fast_dir_exploration;

void explore(const QDir & dir){
    LightFileInfoList list;
    //qDebug()<<dir.absolutePath();
    bool success = listEntries(dir, list, QDir::NoDotAndDotDot);
    Q_FOREACH(const LightFileInfo & i, list){
        qDebug()<<i.path() << i.size() << i.lastModified();
        if(i.isDir()){
            explore(QDir(i.path()));
        }
    }
}

void explore_slow(const QDir & dir){
   QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
   Q_FOREACH(const QFileInfo & i, list){
       qDebug()<<i.absoluteFilePath() << i.size() << i.lastModified();
       if(i.isDir()){
           explore_slow(QDir(i.absoluteFilePath()));
       }
   }
}

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForCStrings(codec);
    QElapsedTimer timer;
    timer.start();
    explore(QDir("."));
    qDebug() << "The fast operation took" << timer.elapsed() << "milliseconds";

    qDebug() <<"***********************************************************************";
    qDebug() <<"***********************************************************************";
    qDebug() <<"***********************************************************************";
    qDebug() <<"***********************************************************************";
    qDebug() <<"***********************************************************************";


    timer.restart();
    explore_slow(QDir("."));
    qDebug() << "The slow operation took" << timer.elapsed() << "milliseconds";
}
