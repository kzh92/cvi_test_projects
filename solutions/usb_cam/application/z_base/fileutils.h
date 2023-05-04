#ifndef FILEUTILS_H
#define FILEUTILS_H

#include "llist.h"
#include <stdio.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#endif

bool mkdir_m(char* filePath);
void ls_m(char* root, char* directoryPath, LList<char*>* files, LList<char*>* directories, LList<char *> *symlinks);
#endif
