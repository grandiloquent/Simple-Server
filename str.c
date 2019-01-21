char *strpbrk(const char *s1, const char *s2) {
  const char *scanp;
  int c, sc;

  while ((c = *s1++) != 0) {
    for (scanp = s2; (sc = *scanp++) != 0;)
      if (sc == c) return ((char *)(s1 - 1));
  }
  return (NULL);
}

void *memchr(const void *s, int c, size_t n) {
  if (n != 0) {
    const unsigned char *p = s;

    do {
      if (*p++ == (unsigned char)c) return ((void *)(p - 1));
    } while (--n != 0);
  }
  return (NULL);
}
size_t strspn(const char *s1, const char *s2) {
  const char *p = s1, *spanp;
  char c, sc;

  /*
   * Skip any characters in s2, excluding the terminating \0.
   */
cont:
  c = *p++;
  for (spanp = s2; (sc = *spanp++) != 0;)
    if (sc == c) goto cont;
  return (p - 1 - s1);
}
char *strtok(char *s, const char *delim) {
  static char *last;

  return strtok_r(s, delim, &last);
}
char *strtok_r(char *s, const char *delim, char **last) {
  const char *spanp;
  int c, sc;
  char *tok;

  if (s == NULL && (s = *last) == NULL) return (NULL);

  /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
   */
cont:
  c = *s++;
  for (spanp = delim; (sc = *spanp++) != 0;) {
    if (c == sc) goto cont;
  }

  if (c == 0) { /* no non-delimiter characters */
    *last = NULL;
    return (NULL);
  }
  tok = s - 1;

  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
  for (;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = '\0';
        *last = s;
        return (tok);
      }
    } while (sc != 0);
  }
  /* NOTREACHED */
}
char *stpncpy(char *dst, const char *src, size_t n) {
  if (n != 0) {
    char *d = dst;
    const char *s = src;

    dst = &dst[n];
    do {
      if ((*d++ = *s++) == 0) {
        dst = d - 1;
        /* NUL pad the remaining n-1 bytes */
        while (--n != 0) *d++ = 0;
        break;
      }
    } while (--n != 0);
  }
  return (dst);
}