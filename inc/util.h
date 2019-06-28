#ifndef __UTIL_H_
#define __UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define unlikely
#define likely

/* from kernel.h, note the usage of __typeof__ macro
 * This looks more complex than it should be. But we need to
 * get the type for the ~ right in round_down (it needs to be
 * as wide as the result!), and we want to evaluate the macro
 * arguments just once each.
 */
#define __round_mask(x, y) ((__typeof__(x))((y)-1))
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)
#define round_down(x, y) ((x) & ~__round_mask(x, y))


#define ERROR(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define INFO(msg) \
  do { perror(msg); } while (0)

#define PINFO(msg) \
  do { printf("%s\n", msg); } while (0)

#define BUFSIZE (1500)
static inline int string_exists(char *s) { return (s ? true : false); }

#define NULLCHAR ('\0')
#define FIRSTCHAR(s)    (s[0])
#define LASTCHAR(s)	(s[strlen(s)-1])
#define STREQ(A, B)      (string_exists((char *)A) && string_exists((char *)B) && \
         (strcmp((char *)(A), (char *)(B)) == 0))
#define STRNEQ(A, B)     (string_exists((char *)A) && string_exists((char *)B) && \
        (strncmp((char *)(A), (char *)(B), strlen((char *)(B))) == 0))
#define PID_ALIVE(x) (kill(x, 0) == 0)


char *strip_ending_whitespace(char *line);
char *strip_beginning_whitespace(char *line);
char *strip_linefeeds(char *line);
char *strip_line_end(char *line);
char *clean_line(char *line);

void stall(unsigned long microseconds);

int is_directory(char *file);
int file_exists(char *file, struct stat *sp);
void *read_file(const char *fn, unsigned *_sz);

int decimal(char *s, int count);
char *dupstr(char *s);

#endif



