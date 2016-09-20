#include "util.h"


/*
 *  Strip line-ending whitespace.
 */
char *strip_ending_whitespace(char *line)
{
  char *p;

  if (line == NULL || strlen(line) == 0)
    return(line);

  p = &LASTCHAR(line);

  while (*p == ' ' || *p == '\t') {
    *p = '\0';
    if (p == line)
      break;
    p--;
  }

  return(line);
}

/*
 *  Strip line-beginning whitespace.
 */
char *strip_beginning_whitespace(char *line)
{
  char buf[BUFSIZE];
  char *p;

  if (line == NULL || strlen(line) == 0)
    return(line);

  strcpy(buf, line);
  p = &buf[0];
  while (*p == ' ' || *p == '\t')
    p++;
  strcpy(line, p);

  return(line);
}

/*
 *  Strip line-ending linefeeds in a string.
 */
char *strip_linefeeds(char *line)
{
	char *p;

	if (line == NULL || strlen(line) == 0)
		return(line);

	p = &LASTCHAR(line);

	while (*p == '\n') {
		*p = NULLCHAR;
		if (--p < line)
			break; 
	}

	return(line);
}

/*
 *  Strip line-beginning and line-ending whitespace and linefeeds.
 */
char *clean_line(char *line)
{
	strip_beginning_whitespace(line);
        strip_linefeeds(line);
        strip_ending_whitespace(line);
        return(line);
}

/*
 *  Strip line-ending whitespace and linefeeds.
 */
char *strip_line_end(char *line)
{
	strip_linefeeds(line);
	strip_ending_whitespace(line);
	return(line);
}


/*
 *  Stall for a number of microseconds.
 */
void stall(unsigned long microseconds)
{
  struct timeval delay;

  delay.tv_sec = 0;
  delay.tv_usec = (__time_t)microseconds;

  (void) select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, &delay);
}



int is_directory(char *file)
{
    struct stat sbuf;
 
    if (!file || !strlen(file))
        return(false);

    if (stat(file, &sbuf) == -1)
        return(false);                         /* This file doesn't exist. */
            
    return((sbuf.st_mode & S_IFMT) == S_IFDIR ? true : false);
}


/*
 *  Determine whether a string contains only decimal characters.
 *  If count is non-zero, limit the search to count characters.
 */
int decimal(char *s, int count)
{
    	char *p;
	int cnt, digits;

	if (!count) {
		strip_line_end(s);
		cnt = 0;
	} else
		cnt = count;

    	for (p = &s[0], digits = 0; *p; p++) {
	        switch(*p)
	        {
	            case '0':
	            case '1':
	            case '2':
	            case '3':
	            case '4':
	            case '5':
	            case '6':
	            case '7':
	            case '8':
	            case '9':
			digits++;
	            case ' ':
	                break;
	            default:
	                return false;
	        }

		if (count && (--cnt == 0))
			break;
    	}

    	return (digits ? true : false);
}


/*
 *  Determine whether a file exists, using the caller's stat structure if
 *  one was passed in.
 */
int file_exists(char *file, struct stat *sp)
{
        struct stat sbuf;

        if (stat(file, sp ? sp : &sbuf) == 0)
                return true;

        return false;
}


char *dupstr(char *s)
{
  char *r;

  r = malloc(strlen (s) + 1);
  strcpy (r, s);
  return (r);
}


/* reads a file, making sure it is terminated with \n \0 */
void *read_file(const char *fn, unsigned *_sz)
{
  char *data;
  int sz;
  int fd;
  struct stat sb;

  data = 0;
  fd = open(fn, O_RDONLY);
  if(fd < 0) return 0;

  // for security reasons, disallow world-writable
  // or group-writable files
  if (fstat(fd, &sb) < 0) {
    //ERROR("fstat failed for '%s'\n", fn);
    goto oops;
  }
  if ((sb.st_mode & (S_IWGRP | S_IWOTH)) != 0) {
    //ERROR("skipping insecure file '%s'\n", fn);
    goto oops;
  }

  sz = lseek(fd, 0, SEEK_END);
  if(sz < 0) goto oops;

  if(lseek(fd, 0, SEEK_SET) != 0) goto oops;

  data = (char*) malloc(sz + 2);
  if(data == 0) goto oops;

  if(read(fd, data, sz) != sz) goto oops;
  close(fd);
  data[sz] = '\n';
  data[sz+1] = 0;
  if(_sz) *_sz = sz;
  return data;

oops:
  close(fd);
  if(data != 0) free(data);
  return 0;
}










