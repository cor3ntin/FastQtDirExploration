#include "fastfolderexploration.h"
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

QString LightFileInfo::fileName() const{
    int slash = path.lastIndexOf(QLatin1Char('/'));
    if (slash != -1)
        return path.mid(slash + 1);
    return QString::null;
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
        printf ("%s\n", file.path.toLocal8Bit().constData());

        delete fullpath;
    }
    closedir(d);
    return true;
}
