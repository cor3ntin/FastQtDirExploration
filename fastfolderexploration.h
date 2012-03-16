#ifndef _FASTFOLDEREXPLORATION_H
#define _FASTFOLDEREXPLORATION_H

#include <QDir>
#include <QString>
#include <QList>
#include <QDateTime>

#ifdef Q_OS_UNIX
#include <sys/stat.h>
#endif

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

namespace fast_dir_exploration{


struct LightFileInfo{

    bool isFile() const;
    bool isDir() const;
    bool isSymlink() const;
    bool isHidden() const;
    QString fileName() const;
    QString path() const;
    quint64 size() const;
    QDateTime lastModified() const;

private:
    QString m_path;
    quint64 m_size;
    QDateTime m_modificationDate;
#ifdef Q_OS_UNIX
    mode_t m_unix_mode;
#endif
#ifdef Q_OS_WIN
    DWORD m_win_attributes;

    friend void item(const QDir &, const WIN32_FIND_DATA &, QList<LightFileInfo> &, QDir::Filters);
#endif


    friend bool listEntries(const QDir &, QList<LightFileInfo> &, QDir::Filters);
};

typedef QList<LightFileInfo> LightFileInfoList;

bool listEntries(const QDir & dir, LightFileInfoList & list, QDir::Filters filters);

}


#endif // FASTFOLDEREXPLORATION_H
