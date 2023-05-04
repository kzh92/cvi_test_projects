#ifndef FOLDERFILE_H
#define FOLDERFILE_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "fileutils.h"
#include "llist.h"
#include "appdef.h"

struct FileHeader {
    char filePath[256];
    unsigned int size;
    unsigned int dataOffset;
    unsigned int mode;
};

typedef struct st_uf_file_header {
    char m_magic[8];
    char m_model[64];
    int m_major;
    int m_minor;
    int m_build;
    int m_patch;
    char m_subtype[32];
    int app_start;
    int app_size;
    int n_file;
    int n_dir;
    int n_link;
    char m_back_ver[32];
} uf_file_header;

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
    void encryptBuf(unsigned char* buf, int size);
    void decryptBuf(unsigned char* buf, int size);
    void removeUselessFiles(char* destination);
    void my_fsync(int fd);

    int progress;
    int checksum;
};

#endif // FOLDERFILE_H
