#ifndef FOLDERFILE_H
#define FOLDERFILE_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fileutils.h"
#include "llist.h"
#include "appdef.h"
#include "upgradebase.h"

//#define USE_APP_UPGRADE

struct FileHeader {
    char filePath[256];
    unsigned int size;
    unsigned int dataOffset;
    unsigned int mode;
};

class FolderFile {
protected:
    LList<FileHeader*> headers;
    LList<char*> directories;
    LList<char*> symlinks;
    int nextOffset;
    char root[256];
    bool compressed;
    bool decompressed;

    void addFile(char* path);
    void addFolder(char* path);
    bool unify(char* filePath, uf_file_header header, char *upgrade_app);
public:
    FolderFile() {nextOffset = 0; compressed = decompressed = false; progress = 0;checksum = 0;}
    FolderFile(char* root) {strcpy(this->root, root); nextOffset = 0; compressed = decompressed = false; progress = 0;checksum = 0;}

    unsigned char* decompressUpgradeApp(char* path, uf_file_header &header);
    bool compressDirectory(char* folder, char* destination, uf_file_header uf, char *upgrade_app);
#if (USE_16M_FLASH == 0)
    bool decompressDirectory(char* filePath, char* destination, uf_file_header &header);
#else//USE_16M_FLASH
    bool decompressDirectory(unsigned char* source, char* destination, uf_file_header &header);
#endif//USE_16M_FLASH
    static void encryptBuf(unsigned char* buf, int size);
    static void decryptBuf(unsigned char* buf, int size);
    void removeUselessFiles(char* destination);
    void my_fsync(int fd);

    int progress;
    int checksum;
};

#endif // FOLDERFILE_H
