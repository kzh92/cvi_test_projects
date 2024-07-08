#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "fileutils.h"
#include "llist.h"
#include "folderfile.h"
#include "aescrypt.h"
#include "common_types.h"
#include "upgradebase.h"
#ifndef _PACK_OTA_
#include "sha1.h"
#endif

unsigned char* g_bimage = NULL;
int g_bimage_len = 0;
unsigned char* g_bBoot = NULL;
int g_bBoot_len = 0;

unsigned char* g_bscript = NULL;
int g_bscript_len = 0;
unsigned char* g_bscript2 = NULL;
int g_bscript_len2 = 0;

unsigned char* g_buboot = NULL;
int g_buboot_len = 0;
int g_hdic1_copied = 0;
int g_wnodic_copied = 0;

int g_iCheckError = 0;

void FolderFile::addFile(char* path) {
    FileHeader* fh;
    char completePath[1024];

    sprintf(completePath, "%s/%s", this->root, path);

    struct stat buffer;

    if (stat(completePath, &buffer) != 0)
        return;

    fh = new FileHeader();
#ifndef _PACK_FIRM_
    strcpy(fh->filePath, path);
#else
    *fh->filePath = '/';
    strcpy(fh->filePath+1, path);
#endif
    fh->dataOffset = this->nextOffset;
    fh->size = buffer.st_size;
    fh->mode = buffer.st_mode;
    this->nextOffset += fh->size;
    headers.addToEnd(fh);
}

void FolderFile::addFolder(char* path) {
    directories.addToEnd(path);
}

bool FolderFile::unify(char* path, uf_file_header header, char* upgrade_app_path) {
#ifdef _PACK_OTA_
    FILE* fp1 = fopen(path, "wb");
    FileHeader* p;
    header.n_file = headers.getSize();
    header.n_dir = directories.getSize();
    header.n_link = symlinks.getSize();

    char completePath[1024];

    if (!fp1)
        return false;

#ifdef USE_APP_UPGRADE
    FILE* upgrade_app_fp = fopen(upgrade_app_path, "rb");

    if (!upgrade_app_fp)
        return false;

    fseek(upgrade_app_fp, 0, SEEK_END);
    header.app_size = ftell(upgrade_app_fp);
    header.app_start = 512;
    fseek(upgrade_app_fp, 0, SEEK_SET);
#else // USE_APP_UPGRADE
    header.app_size = 0;
    header.app_start = 0;
#endif // USE_APP_UPGRADE

#ifdef USE_APP_UPGRADE
///////////////////////////
    unsigned char* buffer = new unsigned char[header.app_size];
    fread(buffer, 1, header.app_size, upgrade_app_fp);
//////
    unsigned char* encryptApp = NULL;
    int encLen = 0;

    unsigned char b = (unsigned char)header.m_build;
    unsigned char m = (unsigned char)header.m_major;
    unsigned char n = (unsigned char)header.m_minor;
    unsigned char pp = (unsigned char)header.m_patch;

    unsigned char abAesKey[AES_BLOCK_SIZE] = { b, m, n, pp,
                                               m, n, pp, b,
                                               n, pp, b, m,
                                               pp, b, m, n };

    AES_Encrypt((unsigned char*)abAesKey, buffer, header.app_size, &encryptApp, &encLen);
    if (!encryptApp || encLen == 0)
    {
        printf("encrypt failed.\n");
        return false;
    }
//////
    header.app_size = encLen;
    fwrite(&header, sizeof(header), 1, fp1);
    fseek(fp1, header.app_start, SEEK_SET);
    fwrite(encryptApp, 1, header.app_size, fp1);
    fclose(upgrade_app_fp);
    delete []buffer;
    free(encryptApp);
///////////////////////////
#else // USE_APP_UPGRADE

#ifndef _PACK_FIRM_
    {
        //upgrade firmware
        fwrite(&header, sizeof(header), 1, fp1);
    }
#else // !_PACK_FIRM_
    {
        int nFiles = headers.getSize();
        int nDirectories = directories.getSize();
        //writing firmware
        fwrite(header.m_magic, sizeof(header.m_magic), 1, fp1);
        fprintf(fp1, "\x09\xF9\x11\x02");
        fwrite(&nFiles, sizeof(unsigned int), 1, fp1);
        fwrite(&nDirectories, sizeof(unsigned int), 1, fp1);
    }
#endif // !_PACK_FIRM_

#endif // USE_APP_UPGRADE

    headers.moveCursor(HEAD);
    while (p = headers.stepForward()) {
        printf("WRITING\n\tPATH: %s\n\tSIZE: %d\n\tOFFSET: %d\n", p->filePath, p->size, p->dataOffset);
#ifndef _PACK_FIRM_
        fwrite(p, 1, sizeof(FileHeader), fp1);
#else
        fwrite(p, 1, sizeof(FileHeader)-sizeof(p->mode), fp1);//for windows, ignore mode.
#endif
    }

    char *s;
    directories.moveCursor(HEAD);
    while (s = directories.stepForward()) {
        printf("WRITING\n\tDIRECTORY: %s\n", s);
        // encryptBuf((unsigned char*)s, strlen(s));
        fwrite(s, 1, 1024, fp1);
    }

    headers.moveCursor(HEAD);
    while (p = headers.stepForward()) {
        sprintf(completePath, "%s/%s", this->root, p->filePath);
        FILE* fp2 = fopen(completePath, "rb");

        if (!fp2) {
            char c = 0;
            for (int i = 0; i < (int)p->size; i++) {
                fwrite(&c, 1, 1, fp1);
            }
            continue;
        }

        char* buffer = new char[p->size];
        fread(buffer, 1, p->size, fp2);
        // encryptBuf((unsigned char*)buffer, p->size);
        fwrite(buffer, 1, p->size, fp1);

        fclose(fp2);
        delete buffer;
    }

    while (s = symlinks.stepForward())
    {
        printf("WRITING\n\tSYMLINK: %s\n", s);
        // encryptBuf((unsigned char*)s, strlen(s));
        fwrite(s, 1, 1024, fp1);
    }

    if (header.m_model[0] != 0)
        fwrite(&checksum, 4, 1, fp1);

    fclose(fp1);
#endif // _PACK_OTA_
    return true;
}

void FolderFile::my_fsync(int fd)
{
    int ret;
    do {
        ret = fsync(fd);
    } while(ret == -1 && errno == EINTR);
    if (ret)
    {
        printf("my_fsync failed:%d(%s).\n", errno, strerror(errno));
    }
}

bool FolderFile::compressDirectory(char* folder, char* destination, uf_file_header header, char* upgrade_app) {
#ifdef _PACK_OTA_
    LList<char*>* directories = new LList<char*>();
    LList<char*>* files = new LList<char*>();

    if (!decompressed) {
        ls_m(this->root, folder, files, directories, &symlinks);
        char* p;

        while (p = files->stepForward()) {
            printf("FILE: %s\n", p);
            addFile(p);
        }

        while (p = directories->stepForward()) {
            printf("DIR: %s\n", p);
            addFolder(p);
        }

        unify(destination, header, upgrade_app);

        compressed = true;

        return true;
    }
#endif // _PACK_OTA_
    return false;
}

unsigned char* FolderFile::decompressUpgradeApp(char *path, uf_file_header &header)
{
    if (!compressed) {
        FILE* fp1 = fopen(path, "rb");

        if (!fp1)
            return NULL;

        fread(&header, sizeof(header), 1, fp1);
        fseek(fp1, header.app_start, SEEK_SET);

        unsigned char* buffer = new unsigned char[header.app_size];
        fread(buffer, 1, header.app_size, fp1);
        fclose(fp1);
        return buffer;
    }
    return NULL;
}

void FolderFile::encryptBuf(unsigned char* buf, int size)
{
//    unsigned char subCheckSum = 0;
//    for(int i = 0; i < size; i++)
//    {
//        subCheckSum ^= buf[i];

//        buf[i] = buf[i] ^ ((i * i) % 128);
//    }

//    checksum += subCheckSum;

    if(size >= 2 * 1024)
    {
        unsigned char abAesKey[AES_BLOCK_SIZE];
        upg_get_aes_key(abAesKey);
        int out_len = 0;
        unsigned char* out_buf = NULL;
        size = size - size % AES_BLOCK_SIZE;
        AES_Encrypt(abAesKey, buf, size, &out_buf, &out_len);
        if (out_buf)
        {
            memcpy(buf, out_buf, size);
            free(out_buf);
        }
        else
        {
            printf("[%s] failed\n", __func__);
        }
    }
}

void FolderFile::decryptBuf(unsigned char* buf, int size)
{
    if(size >= 2 * 1024)
    {
        unsigned char abAesKey[AES_BLOCK_SIZE];
        upg_get_aes_key(abAesKey);
        int out_len = 0;
        unsigned char* out_buf = NULL;
        size = size - size % AES_BLOCK_SIZE;
        AES_Decrypt(abAesKey, buf, size, &out_buf, &out_len);
        if (out_buf)
        {
            memcpy(buf, out_buf, size);
            free(out_buf);
        }
        else
        {
            printf("[%s] failed\n", __func__);
        }
    }

//    unsigned char subCheckSum = 0;

//    for(int i = 0; i < size; i++)
//    {
//        buf[i] = buf[i] ^ ((i * i) % 128);
//        subCheckSum ^= buf[i];
//    }
//    checksum += subCheckSum;
}

void FolderFile::removeUselessFiles(char* destination)
{
#ifdef _PACK_OTA_
    //Remove files
    char rem_file_path[256];
    char rem_filelist_path[256];
    char* line;
    size_t len = 0;
    ssize_t read;

    sprintf(rem_filelist_path, "%s/__useless_files.txt", destination);
    FILE* rem_files_fp = fopen(rem_filelist_path, "r");

    if(rem_files_fp)
    {
        while ((read = getline(&line, &len, rem_files_fp)) != -1) {

            char *pos;
            if ((pos=strchr(line, '\n')) != NULL)
                *pos = '\0';

            if(strlen(line) <= 0)
                continue;

            sprintf(rem_file_path, "rm -rf \"%s/%s\"", destination, line);

//            printf("REMOVE FILE: %s\n", rem_file_path);

            system(rem_file_path);
        }

        fclose(rem_files_fp);
        remove(rem_filelist_path);
        if(line)
            free(line);
    }
#endif // _PACK_OTA_
}

#if (USE_16M_FLASH == 0)
bool FolderFile::decompressDirectory(char* path, char* destination, uf_file_header &header) {
#else
bool FolderFile::decompressDirectory(unsigned char* source, char* destination, uf_file_header &header) {
#endif//USE_16M_FLASH
    FileHeader* p;
    int nBytes = 0;

    if (!compressed) {
#if (USE_16M_FLASH == 0)
        FILE* fp1 = fopen(path, "rb");
        if (!fp1)
            return false;

        mkdir_m(destination);
        fread(&header, sizeof(header), 1, fp1);
        fseek(fp1, header.app_start + header.app_size, SEEK_SET);
#else//USE_16M_FLASH
        unsigned char* current_addr = source;
        mkdir_m(destination);
        memcpy(&header, current_addr, sizeof(header));
        current_addr += sizeof(header) + header.app_start + header.app_size;
#endif//USE_16M_FLASH
        int count = 0;

        //Read in file headers
        while (count < header.n_file) {
            p = new FileHeader();
#if (USE_16M_FLASH == 0)
            nBytes = fread(p, 1, sizeof(FileHeader), fp1);
#else//USE_16M_FLASH
            memcpy(p, current_addr, sizeof(FileHeader));
            current_addr += sizeof(FileHeader);
#endif//USE_16M_FLASH
            headers.addToEnd(p);
            count++;
        }

        count = 0;

        //Read in directories that need to be created
        while(count < header.n_dir) {
            char* s = new char[1025];
#if (USE_16M_FLASH == 0)
            nBytes = fread(s, 1, 1024, fp1);
#else//USE_16M_FLASH
            memcpy(s, current_addr, 1024);
            current_addr += 1024;
            nBytes = 1024;
#endif//USE_16M_FLASH
            // decryptBuf((unsigned char*)s, strlen(s));
            s[nBytes] = 0;
            directories.addToEnd(s);
            count++;
        }

        int nTotal = directories.getSize() + headers.getSize() + header.n_link;
        int nProcessed = 0;
        progress = nProcessed * 100 / nTotal;

        //Make directories
        char* s;
        while (1) {
            s = directories.stepForward();
            if (!s)
                break;
            char filename[1024];
            sprintf(filename, "%s/%s", destination, s);

//            printf("DIR: %s\n", filename);
            mkdir_m(filename);
            progress = (++nProcessed) * 100 / nTotal;
        }

        //Write files
        while (1) {
            p = headers.stepForward();
            if (!p)
                break;
            char* buffer = new char[p->size];
            char filename[1024];
            sprintf(filename, "%s/%s", destination, p->filePath);

            dbug_printf("WRITE FILE: %s\n", filename);
#if (USE_16M_FLASH == 0)
            fread(buffer, 1, p->size, fp1);
#else//USE_16M_FLASH
            memcpy(buffer, current_addr, p->size);
            current_addr += p->size;
#endif//USE_16M_FLASH

#ifndef _PACK_OTA_
            upg_update_part(p->filePath, (unsigned char*)buffer, p->size, &header);
#endif // !_PACK_OTA_
            delete []buffer;
            progress = (++nProcessed) * 100 / nTotal;
        }

        char* full_path;
        char target[256];
        char target2[256];
        char src[256];
        //Write symlinks
        count = 0;

        //Read in directories that need to be created
        while(count < header.n_link) {
            char* s = new char[1025];
#if (USE_16M_FLASH == 0)
            nBytes = fread(s, 1, 1024, fp1);
#else//USE_16M_FLASH
            memcpy(s, current_addr, 1024);
            current_addr += 1024;
#endif//USE_16M_FLASH
            // decryptBuf((unsigned char*)s, strlen(s));
            s[nBytes] = 0;

            char* ptr = strstr(s, " -> ");
            *ptr = '\0';

            full_path = s;
            sprintf(src, "%s/%s", destination, full_path);
            strcpy(target, src);

            *(strrchr(target, '/') + 1) = 0;
            sprintf(target2, "%s%s", target, ptr+4);
#ifdef _PACK_OTA_
            symlink(target2, src);
#endif

            progress = (++nProcessed) * 100 / nTotal;
            count++;
        }

        int readCheckSum = 0;
#if (USE_16M_FLASH == 0)
        fread(&readCheckSum, 4, 1, fp1);

        fclose(fp1);
#else//USE_16M_FLASH
        memcpy(&readCheckSum, current_addr, 4);
        current_addr += 4;
#endif//USE_16M_FLASH
        decompressed = true;

        if(readCheckSum != checksum)
        {
            checksum = 0;
            return false;
        }
        checksum = 0;
        return true;
    }

    return false;
}
