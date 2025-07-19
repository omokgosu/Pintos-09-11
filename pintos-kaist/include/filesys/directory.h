#ifndef FILESYS_DIRECTORY_H
#define FILESYS_DIRECTORY_H

#include <stdbool.h>
#include <stddef.h>
#include "devices/disk.h"

/* 파일 이름 구성 요소의 최대 길이.
 * 이는 전통적인 UNIX 최대 길이입니다.
 * 디렉터리가 구현된 후에도 이 최대 길이는 유지될 수 있지만,
 * 훨씬 더 긴 전체 경로 이름들이 허용되어야 합니다. */
#define NAME_MAX 14

struct inode;

/* 디렉터리 열기 및 닫기. */
bool dir_create (disk_sector_t sector, size_t entry_cnt);
struct dir *dir_open (struct inode *);
struct dir *dir_open_root (void);
struct dir *dir_reopen (struct dir *);
void dir_close (struct dir *);
struct inode *dir_get_inode (struct dir *);

/* 읽기 및 쓰기. */
bool dir_lookup (const struct dir *, const char *name, struct inode **);
bool dir_add (struct dir *, const char *name, disk_sector_t);
bool dir_remove (struct dir *, const char *name);
bool dir_readdir (struct dir *, char name[NAME_MAX + 1]);

#endif /* filesys/directory.h */
