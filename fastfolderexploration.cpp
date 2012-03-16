#include "fastfolderexploration.h"
#include <iostream>
#include <QDebug>




QString LightFileInfo::fileName() const{
    int slash = path.lastIndexOf(QLatin1Char('/'));
    if (slash != -1)
        return path.mid(slash + 1);
    return QString::null;
}



#ifdef Q_OS_UNIX
#include "dirent.h"

bool LightFileInfo::isFile() const{
    return S_ISREG(m_unix_mode);
}
bool LightFileInfo::isDirectory() const{
    return S_ISDIR(m_unix_mode);
}
bool LightFileInfo::isSymlink() const{
    return S_ISLNK(m_unix_mode);
}

bool LightFileInfo::isHidden() const{
    for(int i = path.size()-1; i>=0; i--){
        const QChar & c = path.at(i);
        if(c == '.')
            return true;
        if(c == '/')
            return false;
    }
    return false;
}





#define LIST_FAIL do{ list.clear(); return false; }while(false);

bool listEntries(const QDir & dir, LightFileInfoList & list, QDir::Filters filters){
    DIR *d;
    struct dirent* ent;
    QByteArray dir_path=dir.path().toLocal8Bit();
    int alloc_lenght = strlen(dir_path.constData()) + 2;

    d = opendir (dir.absolutePath().toLocal8Bit());
    if (d == NULL)
        LIST_FAIL
    while ((ent = readdir (d)) != NULL) {

            if((filters & QDir::NoDotAndDotDot || (filters &QDir::NoDot)) && strcmp(ent->d_name, ".")==0)
                continue;
            if((filters & QDir::NoDotAndDotDot || (filters &QDir::NoDotDot)) && strcmp(ent->d_name, "..")==0)
                continue;

        char *fullpath = new char[strlen(ent->d_name) + alloc_lenght];
        sprintf(fullpath, "%s/%s", dir_path.constData(), ent->d_name);
        struct stat64 st;
        if(stat64(fullpath, &st) ==-1){
            LIST_FAIL
        }
        LightFileInfo file;
        file.path= QString::fromLocal8Bit(fullpath);
        file.modificationDate.setTime_t(st.st_mtime);
        file.size =  st.st_size;
        file.m_unix_mode = st.st_mode;
        list.append(file);

        delete fullpath;
    }
    closedir(d);
    return true;
}

#endif



#ifdef Q_OS_WIN

bool LightFileInfo::isFile() const{
    return m_win_attributes & FILE_ATTRIBUTE_NORMAL;
}
bool LightFileInfo::isDirectory() const{
    return m_win_attributes & FILE_ATTRIBUTE_DIRECTORY;
}
bool LightFileInfo::isSymlink() const{
    return false;
}
bool LightFileInfo::isHidden() const{
   return m_win_attributes & FILE_ATTRIBUTE_HIDDEN;
}

static inline QDateTime fileTimeToQDateTime(const FILETIME *time)
{
    QDateTime ret;
    SYSTEMTIME sTime, lTime;
    FileTimeToSystemTime(time, &sTime);
    SystemTimeToTzSpecificLocalTime(0, &sTime, &lTime);
    ret.setDate(QDate(lTime.wYear, lTime.wMonth, lTime.wDay));
    ret.setTime(QTime(lTime.wHour, lTime.wMinute, lTime.wSecond, lTime.wMilliseconds));
    return ret;
}

void item(const QDir & dir, const WIN32_FIND_DATA & d, LightFileInfoList & list, QDir::Filters filters){
    if((filters & QDir::NoDotAndDotDot || (filters &QDir::NoDot)) && wcscmp(d.cFileName, L".")==0)
        return;
    if((filters & QDir::NoDotAndDotDot || (filters &QDir::NoDotDot)) && wcscmp(d.cFileName, L"..")==0)
        return;

     LightFileInfo i;
     i.path = dir.absoluteFilePath(QString::fromWCharArray(d.cFileName)); //dir.filePath();

     i.size = (d.nFileSizeHigh << 32) + d.nFileSizeLow;
     i.modificationDate = fileTimeToQDateTime(&d.ftLastWriteTime);
     i.m_win_attributes = d.dwFileAttributes;

     list.append(i);
}

bool listEntries(const QDir & dir, LightFileInfoList & list, QDir::Filters filters){

    WIN32_FIND_DATA data;
    QString search = dir.absolutePath()+"\\*";
    HANDLE hFind = FindFirstFile((const wchar_t *)search.utf16(), &data);
    if (hFind == INVALID_HANDLE_VALUE) {
        return false;
    }
    item(dir, data, list, filters);
    while (FindNextFile(hFind, &data)){
       item(dir, data, list, filters);
    }
    FindClose(hFind);
    return true;
}

#endif

