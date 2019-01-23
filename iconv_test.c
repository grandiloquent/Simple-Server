// -liconv -I./include
/*
下载复制include,lib文件夹到代码目录
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "iconv.h"
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

  size_t len = out_size;
  unsigned char b[len];
  memcpy(b, buf, len);
  for (size_t i = 0; i < len; i++) {
    printf("%I64d = %d\n", i, b[i]);
  }

  free(buf);
}