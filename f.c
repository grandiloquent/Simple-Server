// -liconv -I./include
/*
下载复制include,lib文件夹到代码目录
*/
#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "iconv.h"

void listdir(const char *path) {
  DIR *dir;
  struct dirent *entry;
  // https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/stat-functions?view=vs-2017
  struct stat st;
  if (!(dir = opendir(path))) return;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
      continue;
    char name[256];
    snprintf(name, sizeof(name), "%s/%s", path, entry->d_name);
    if (stat(name, &st) == 0) {
      if (S_ISDIR(st.st_mode))
        listdir(name);
      else if (S_ISREG(st.st_mode)) {
        printf("%s\n", strrchr(name, '/'));
      }
    }
  }
}

char *utf82gbk(const char *data, size_t *outsize) {
  iconv_t cd = iconv_open("GBK", "UTF-8");
  size_t in_size = strlen(data);
  size_t out_size = in_size;
  size_t malloc_size = out_size;
  char *outbuf = (char *)malloc(out_size);
  char *inputr = (char *)data;
  char *outptr = outbuf;
  size_t n = iconv(cd, &inputr, &in_size, &outptr, &out_size);

  // 如果输入字符未0结尾
  // 会出现 EINVAL An incomplete multibyte sequence has been encountered in the
  // input

  if (n == -1) {
    if (errno == E2BIG) printf("errno == E2BIG\n");
    if (errno == EILSEQ) printf("errno == EILSEQ\n");
    if (errno == EINVAL) printf("errno == EINVAL\n");
    iconv_close(cd);
    return NULL;
  }
  iconv_close(cd);
  *outsize = malloc_size - out_size;
  return outbuf;
}

int main(int argc, char *argv[]) {
  char utf8[] = {229, 164, 167, 233, 163, 142, 232, 189, 166, 0};
  size_t out_size = 0;
  const char *data = utf8;
  char *buf = utf82gbk(data, &out_size);

  size_t i = 0;
  size_t len = sizeof(buf);
  unsigned char b[len];
  memcpy(b, buf, len);
  for (size_t i = 0; i < len; i++) {
    printf("%I64d = %d\n", i, b[i]);
  }

  free(buf);
}