#include "fastfolderexploration.h"


namespace fast_dir_exploration{

QString LightFileInfo::path() const{
	return m_path;
}
quint64 LightFileInfo::size() const{
	return m_size;
}
QDateTime LightFileInfo::lastModified() const{
	return m_modificationDate;
}

QString LightFileInfo::fileName() const{
    int slash = m_path.lastIndexOf(QLatin1Char('/'));
    if (slash != -1)
        return m_path.mid(slash + 1);
    return QString::null;
}



#ifdef Q_OS_UNIX
#include "dirent.h"

bool LightFileInfo::isFile() const{
    return S_ISREG(m_unix_mode);
}
bool LightFileInfo::isDir() const{
    return S_ISDIR(m_unix_mode);
}
bool LightFileInfo::isSymlink() const{
    return S_ISLNK(m_unix_mode);
}

bool LightFileInfo::isHidden() const{
    for(int i = m_path.size()-1; i>=0; i--){
        const QChar & c = m_path.at(i);
        if(c == '.' && i>0 && m_path.at(i-1)=='/')
            return true;
        if(c == '/')
            return false;
    }
    return false;
}





bool listEntries(const QDir & dir, LightFileInfoList & list, QDir::Filters filters){
    DIR *d;
    struct dirent* ent;
    QByteArray dir_path=dir.path().toLocal8Bit();
    int alloc_lenght = strlen(dir_path.constData()) + 2;

    d = opendir (dir.absolutePath().toLocal8Bit());
    if (d == NULL){
    	list.clear(); return false;
    }
    while ((ent = readdir (d)) != NULL) {

            if(((filters & QDir::NoDotAndDotDot) || (filters &QDir::NoDot)) && strcmp(ent->d_name, ".")==0)
                continue;
            if(((filters & QDir::NoDotAndDotDot) || (filters &QDir::NoDotDot)) && strcmp(ent->d_name, "..")==0)
                continue;

        char *fullpath = new char[strlen(ent->d_name) + alloc_lenght];
        sprintf(fullpath, "%s/%s", dir_path.constData(), ent->d_name);
        struct stat64 st;
        if(stat64(fullpath, &st) ==-1){
        	list.clear();
        	return false;
        }
        LightFileInfo file;
        file.m_path= QString::fromLocal8Bit(fullpath);
        file.m_modificationDate.setTime_t(st.st_mtime);
        file.m_size =  st.st_size;
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
bool LightFileInfo::isDir() const{
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
    if(((filters & QDir::NoDotAndDotDot) || (filters &QDir::NoDot)) && wcscmp(d.cFileName, L".")==0)
        return;
    if(((filters & QDir::NoDotAndDotDot) || (filters &QDir::NoDotDot)) && wcscmp(d.cFileName, L"..")==0)
        return;

     LightFileInfo i;
     i.m_path = dir.absoluteFilePath(QString::fromWCharArray(d.cFileName)); //dir.filePath();

     i.m_size = (d.nFileSizeHigh << 32) + d.nFileSizeLow;
     i.m_modificationDate = fileTimeToQDateTime(&d.ftLastWriteTime);
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

}
