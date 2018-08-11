/*
  mathfs: Screwing around with file systems

  gcc -Wall mathfs.c `pkg-config fuse --cflags --libs` -o mathfs
*/

#define FUSE_USE_VERSION 26

#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *A_PATH = "/a.txt";
static const char *B_PATH = "/b.txt";
static const char *SUM_PATH = "/sum.txt";
static const char *MUL_PATH = "/mul.txt";

static size_t A_VAL = 1;
static size_t B_VAL = 1;

static size_t int_digits(int value) {
    if (value == 0) {
        return 1;
    } else {
        return floor(log10(value)) + 1;
    }
}

static int mathfs_getattr(const char *path, struct stat *stbuf) {
    int res = 0;

    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 2;
    } else if (strcmp(path, A_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = int_digits(A_VAL);
    } else if (strcmp(path, B_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = int_digits(B_VAL);
    } else if (strcmp(path, SUM_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = int_digits(A_VAL + B_VAL);
    } else if (strcmp(path, MUL_PATH) == 0) {
        stbuf->st_mode = S_IFREG | 0777;
        stbuf->st_nlink = 1;
        stbuf->st_size = int_digits(A_VAL * B_VAL);
    } else {
        res = -ENOENT;
    }

    return res;
}

static int mathfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                          off_t offset, struct fuse_file_info *fi) {
    (void)offset;
    (void)fi;

    if (strcmp(path, "/") != 0) return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);
    filler(buf, A_PATH + 1, NULL, 0);
    filler(buf, B_PATH + 1, NULL, 0);
    filler(buf, SUM_PATH + 1, NULL, 0);
    filler(buf, MUL_PATH + 1, NULL, 0);
    return 0;
}

static int mathfs_open(const char *path, struct fuse_file_info *fi) {
    (void)fi;
    if (strcmp(path, A_PATH) == 0) {
        return 0;
    } else if (strcmp(path, B_PATH) == 0) {
        return 0;
    } else if (strcmp(path, SUM_PATH) == 0) {
        return 0;
    } else if (strcmp(path, MUL_PATH) == 0) {
        return 0;
    } else {
        return -ENOENT;
    }
}

static int mathfs_read(const char *path, char *buf, size_t size, off_t offset,
                       struct fuse_file_info *fi) {
    (void)fi;
    int len;
    int val;
    if (strcmp(path, A_PATH) == 0) {
        val = A_VAL;
    } else if (strcmp(path, B_PATH) == 0) {
        val = B_VAL;
    } else if (strcmp(path, SUM_PATH) == 0) {
        val = A_VAL + B_VAL;
    } else if (strcmp(path, MUL_PATH) == 0) {
        val = A_VAL * B_VAL;
    } else {
        return -ENOENT;
    }

    len = int_digits(val);
    char *s = malloc((len * sizeof(char)) + 1);
    sprintf(s, "%d", val);

    if (offset < len) {
        if (offset + (int)size > len) size = len - offset;
        memcpy(buf, s + offset, size);
    } else
        size = 0;

    free(s);

    return size;
}

static int mathfs_write(const char *path, const char *buf, size_t size,
                        off_t offset, struct fuse_file_info *fi) {
    (void)fi;
    (void)offset;
    printf("Write: %s\n", buf);
    int val = atoi(buf);

    if (strcmp(path, A_PATH) == 0) {
        A_VAL = val;
    } else if (strcmp(path, B_PATH) == 0) {
        B_VAL = val;
    } else if (strcmp(path, SUM_PATH) == 0) {
        return -EPERM;
    } else if (strcmp(path, MUL_PATH) == 0) {
        return -EPERM;
    } else {
        return -ENOENT;
    }
    return size;
}

static int mathfs_truncate(const char *path, off_t size) {
    (void)size;

    if (strcmp(path, A_PATH) == 0) {
        return 0;
    } else if (strcmp(path, B_PATH) == 0) {
        return 0;
    } else if (strcmp(path, SUM_PATH) == 0) {
        return 0;
    } else if (strcmp(path, MUL_PATH) == 0) {
        return 0;
    } else {
        return -ENOENT;
    }
}

static struct fuse_operations mathfs_oper = {
    .getattr = mathfs_getattr,
    .truncate = mathfs_truncate,
    .readdir = mathfs_readdir,
    .open = mathfs_open,
    .read = mathfs_read,
    .write = mathfs_write,
};

int main(int argc, char *argv[]) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

    fuse_opt_parse(&args, NULL, NULL, NULL);
    fuse_opt_add_arg(&args, "-odirect_io");

    return fuse_main(args.argc, args.argv, &mathfs_oper, NULL);
}
