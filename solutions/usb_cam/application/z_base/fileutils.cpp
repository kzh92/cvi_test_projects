#include "fileutils.h"
#include <string.h>
#include <unistd.h>

bool mkdir_m(char* filePath) {
#ifdef _WIN32
    return CreateDirectory(filePath, NULL);
#else
    int ret = mkdir(filePath, 0777);
    return (ret > -1);
#endif
}

void ls_m(char* root, char* directoryPath, LList<char*>* files, LList<char*>* directories, LList<char*>* symlinks) {
#ifdef _PACK_OTA_
    char fullPath[1024];
    char search[256];
    char buf[512];
#endif

#ifdef _WIN32
    HANDLE hFind;
    WIN32_FIND_DATA find;
    sprintf(search, "%s%s*", root, directoryPath);

    if((hFind = FindFirstFile(search, &find)) != INVALID_HANDLE_VALUE){
        do{
            if (strcmp(find.cFileName, ".") != 0 && strcmp(find.cFileName, "..") != 0) {
                if(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    sprintf(fullPath, "%s%s", directoryPath, find.cFileName);

                    char* copy = new char[1024];
                    strcpy(copy, fullPath);
                    directories->addToEnd(copy);

                    sprintf(fullPath, "%s\\", fullPath);
                    ls_m(root, fullPath, files, directories);
                }
                else {
                    sprintf(fullPath, "%s%s", directoryPath, find.cFileName);

                    char* copy = new char[1024];
                    strcpy(copy, fullPath);
                    files->addToEnd(copy);
                }
            }
        } while(FindNextFile(hFind, &find));

        FindClose(hFind);
    }
#elif (defined(_PACK_OTA_))
    struct dirent *de=NULL;
    DIR *d = NULL;
    char* copy;
    int count;
//    FILE* fp;
    char symlinkFile[1024];

    sprintf(search, "%s/%s", root, directoryPath);

    d = opendir(search);

    if(d == NULL)
        return;

    // Loop while not NULL
    while(de = readdir(d)) {
        switch(de->d_type) {
        case DT_DIR:
            if (strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
                sprintf(fullPath, "%s%s/", directoryPath, de->d_name);
                printf("DIR %s\n", fullPath);
                copy = strdup(fullPath);
                directories->addToEnd(copy);
                ls_m(root, fullPath, files, directories, symlinks);
            }
            break;
        case DT_REG:
            sprintf(fullPath, "%s%s", directoryPath, de->d_name);
            printf("FILE %s\n", fullPath);
            copy = strdup(fullPath);
            files->addToEnd(copy);
            break;
        case DT_LNK:
            sprintf(fullPath, "%s/%s%s", root, directoryPath, de->d_name);
            memset(buf,0,sizeof(buf));
            count = readlink(fullPath, buf, sizeof(buf) - 1);

            if (count >= 0) {
                buf[count] = '\0';
                sprintf(symlinkFile, "%s%s -> %s", directoryPath, de->d_name, buf);
                copy = strdup(symlinkFile);
                symlinks->addToEnd(copy);
            }
            break;
        };
    }

    closedir(d);
#endif
}

