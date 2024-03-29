#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include "util.h"
#include "parser.h"
#include "ltsys.h"

#define SECTION 0x01
#define OPTION  0x02

#define T_EOF	0
#define T_TEXT	1
#define T_NEWLINE 2
#define T_CONTLINE 3	// continue line when without \n char

#define INTT_PARSER_MAXARGS	12

#include "keywords.h"

#define KEYWORD(symbol, symlen, flags, level, nargs, func, line_func) \
    [ K_##symbol ] = { #symbol, symlen, flags, level, nargs + 1, func, line_func},

static struct kword keyword_info[KEYWORD_COUNT] = {
    [ K_UNKNOWN ] = { "UNKNOWN", 0, 0, 0, 0, 0, 0 },
#include "keywords.h"
};
#undef keyword

#define kw_is(kw, type) (keyword_info[kw].flags & (type))
#define kw_name(kw) (keyword_info[kw].name)
#define kw_func(kw) (keyword_info[kw].func)
#define kw_line_func(kw) (keyword_info[kw].line_func)
#define kw_nargs(kw) (keyword_info[kw].nargs)
#define kw_symlen(kw) (keyword_info[kw].symlen)
#define kw_level(kw) (keyword_info[kw].level)

static int lookup_keyword(const char *s)
{
  switch (*s++) {
    case 'E':	//ENUM
      return K_ENUM;
    case 'G':	//GRAM, GREF
      return (*(s+1)=='A')? K_GRAM : K_GREF;
    case 'I':	//ITEM
      return K_ITEM;
    case 'S':	//SITEM
      return K_SITEM;
    case 'R':	//RFX, RFREF
      if (*(s+1) == 'X')	return K_RFX;
      else	return K_RFREF;
    case 'V':	//VOC, VGRP, VS, VREF, VGREF
      switch (*s++) {
	case 'O':	return K_VOC;
	case 'G':	return (*(s+1)=='P')? K_VGRP : K_VGREF;
	case 'S':	return K_VS;
	case 'R':	return K_VREF;
      }
  }
  return K_UNKNOWN;
}

static void parse_line_no_op(struct parse_context *state, int nargs, char **args)
{
}

static void parse_new_section(struct parse_context *state, int kw, int nargs, char **args)
{
  void *ctxt = NULL;
  int kwlevel = kw_level(kw);

  ctxt = (void *)(kw_func(kw))(state, nargs, args);
  if (ctxt) {
    if (kwlevel != state->cur_level)
      state->cur_level = kwlevel;
    state->pcn[state->cur_level].context = ctxt;
    state->pcn[state->cur_level].parse_line = kw_line_func(kw);
    return;
  }

  /* if kw_func() fails, clear ctxt/parse_line of level kwlevel */
  state->pcn[kwlevel].context = NULL;
  state->pcn[kwlevel].parse_line = parse_line_no_op;
}

void parse_error(struct parse_context *state, const char *fmt, ...)
{
    va_list ap;
    char buf[128];
    int off;

    snprintf(buf, 128, "%s: %d: ", state->filename, state->line);
    buf[127] = 0;
    off = strlen(buf);

    va_start(ap, fmt);
    vsnprintf(buf + off, 128 - off, fmt, ap);
    va_end(ap);
    buf[127] = 0;
    ERROR(buf);
}

static int valid_name(const char *name)
{
#if 0
    if (strlen(name) > 16) {
        return 0;
    }
#endif
    while (*name) {
        if (!isalpha(*name) && (*name != '_') && (*name != '-')) {
            return 0;
        }
        name++;
    }
    return 1;
}

int next_token(struct parse_context *state)
{
  char *x = state->ptr;
  char *s;
  static unsigned brace_cnt = 0;	// count for brace character ({|})
  static unsigned line_real_termed = 0; // flag to indicate line really termed when encount a semicolon

  if (state->nexttoken) {
    int t = state->nexttoken;
    state->nexttoken = 0;
    return t;
  }

  for (;;) {
    switch (*x) {
      case 0:
	state->ptr = x;
	return T_EOF;
      case '\n':
	if (brace_cnt>0 && !line_real_termed) {
		*x = ' ';
		x++;
		state->ptr = x;
		return T_CONTLINE;
	} else {
		/* reach here means real line end */
		x++;
		state->ptr = x;
		line_real_termed = 0;	// reset
		return T_NEWLINE;
	}
      case ' ':
      case '\t':
      case '\r':
	x++;
	continue;
      case '#':
	while (*x && (*x != '\n')) x++;
	if (*x == '\n') {
	  state->ptr = x+1;
	  return T_NEWLINE;
	} else {
	  state->ptr = x;
	  return T_EOF;
	}
      default:
	goto text;
    }
  }

textdone:
  state->ptr = x;
  *s = 0;
  return T_TEXT;
text:
  state->text = s = x;
textresume:
  for (;;) {
    switch (*x) {
      case 0:
	goto textdone;
      case ' ':
      case '\t':
      case '\r':
      case '.':
      case ':':
	x++;
	goto textdone;
      case ';':
	x++;
	line_real_termed = 1;	// indicate line true termed
	goto textdone;
      case '{':
	brace_cnt++;
	x++;
	s++;
	break;
      case '}':
	brace_cnt--;
	x++;
	s++;
	break;
      case '\n':
	state->nexttoken = T_NEWLINE;
	x++;
	goto textdone;
      case '"':
      case '[':
	x++;
	for (;;) {
	  switch (*x) {
	    case 0:
	      /* unterminated quoted thing */
	      state->ptr = x;
	      return T_EOF;
	    case '"':
	    case ']':
	      x++;
	      goto textresume;
	    default:
	      *s++ = *x++;
	  }
	}
	break;
      case '\\':
	x++;
	switch (*x) {
	  case 0:
	    goto textdone;
	  case 'n':
	    *s++ = '\n';
	    break;
	  case 'r':
	    *s++ = '\r';
	    break;
	  case 't':
	    *s++ = '\t';
	    break;
	  case '\\':
	    *s++ = '\\';
	    break;
	  case '\r':
	    /* \ <cr> <lf> -> line continuation */
	    if (x[1] != '\n') {
	      x++;
	      continue;
	    }
	  case '\n':
	    /* \ <lf> -> line continuation */
	    state->line++;
	    x++;
	    /* eat any extra whitespace */
	    while((*x == ' ') || (*x == '\t')) x++;
	    continue;
	  default:
	    /* unknown escape -- just copy */
	    *s++ = *x++;
	}
	continue;
      default:
	*s++ = *x++;
    }
  }
  return T_EOF;
}

#define debug_parse 1
static void parse_ltd(const char *fn, char *s)
{
  struct parse_context state;
  char *args[INTT_PARSER_MAXARGS];
  int nargs;
#if debug_parse
  char buf[128];
#endif

  nargs = 0;
  state.filename = fn;
  state.line = 0;
  state.ptr = s;
  state.nexttoken = 0;
  state.cur_level = 0;
  for (int i=0; i<3; i++) {
    state.pcn[i].context = NULL;
    state.pcn[i].parse_line = parse_line_no_op;
  }

  for(;;) {
    switch (next_token(&state)) {
      case T_EOF:
	state.pcn[state.cur_level].parse_line(&state, nargs, args);
	goto parser_done;
      case T_NEWLINE:
	state.line++;
	if (nargs) {
#if debug_parse
	  memset(buf, 0, sizeof(buf));
	  for (int i=0; i<nargs; i++) {
	    strcat(buf, "|");
	    strcat(buf, args[i]);
	  }
	  PINFO(buf);
#endif
	  int kw = lookup_keyword(args[0]);
	  if(kw_is(kw, SECTION)) {
	    parse_new_section(&state, kw, nargs, args);
	    state.pcn[state.cur_level].parse_line(&state, nargs, args);
	  } else if(kw != K_UNKNOWN) {
	    /* for XREF keywords, they must be placed in front of level one
	     * block, before any ITEM clauses.
	     * So we can use parse_line() ptr at current level in pcn[] array*/
	    state.pcn[state.cur_level].parse_line(&state, nargs, args);
	  }
	  nargs = 0;
	}
	break;
      case T_TEXT:
	if (nargs < INTT_PARSER_MAXARGS) {
	  args[nargs++] = state.text;
	}
	break;
      case T_CONTLINE:
	//do nothing
	break;
    }
  }

parser_done: return;
}

int ltd_parse_file(const char *fn)
{
    char *data;
    data = read_file(fn, 0);
    if (!data) return -1;

    parse_ltd(fn, data);
    //dump();
    return 0;
}

static void *parse_rfx(struct parse_context *state, int nargs, char **args)
{
  return 0;
}

static void parse_line_rfx(struct parse_context *state, int nargs, char **args)
{
}

static void *parse_voc(struct parse_context *state, int nargs, char **args)
{
  struct v_entry *ve = NULL;

  if (!valid_name(args[1])) {
    parse_error(state, "invalid voc entry name '%s'\n", args[1]);
    return 0;
  }

#if 0
  ve = voc_find_by_name(args[1]);
  if (ve) {
    parse_error(state, "ignored duplicate definition of voc entry '%s'\n", args[1]);
    return 0;
  }
#endif

  ve = malloc(sizeof(*ve));
  return ve;
}

static void parse_line_voc(struct parse_context *state, int nargs, char **args)
{
  struct v_entry *ve = (struct v_entry *)state->pcn[0].context;
}

static void *parse_vgrp(struct parse_context *state, int nargs, char **args)
{
  return 0;
}

static void parse_line_vgrp(struct parse_context *state, int nargs, char **args)
{
}

static void *parse_gram(struct parse_context *state, int nargs, char **args)
{
  return 0;
}
static void parse_line_gram(struct parse_context *state, int nargs, char **args)
{
}

static void *parse_vs(struct parse_context *state, int nargs, char **args)
{
  return 0;
}
static void parse_line_vs(struct parse_context *state, int nargs, char **args)
{
}

static void *parse_item(struct parse_context *state, int nargs, char **args)
{
  return 0;
}
static void parse_line_item(struct parse_context *state, int nargs, char **args)
{
}

static void *parse_sitem(struct parse_context *state, int nargs, char **args)
{
  return 0;
}
static void parse_line_sitem(struct parse_context *state, int nargs, char **args)
{
}

#if 0
static void *parse_service(struct parse_context *state, int nargs, char **args)
{
  /*
    struct service *svc;
    if (nargs< 3) {
        parse_error(state, "services must have a name and a program\n");
        return 0;
    }
    if (!valid_name(args[1])) {
        parse_error(state, "invalid service name '%s'\n", args[1]);
        return 0;
    }

    svc = service_find_by_name(args[1]);
    if (svc) {
        parse_error(state, "ignored duplicate definition of service '%s'\n", args[1]);
        return 0;
    }

    nargs -= 2;
    svc = calloc(1, sizeof(*svc) + sizeof(char*) * nargs);
    if (!svc) {
        parse_error(state, "out of memory\n");
        return 0;
    }
    svc->name = args[1];
    svc->classname = "default";
    memcpy(svc->args, args + 2, sizeof(char*) * nargs);
    svc->args[nargs] = 0;
    svc->nargs = nargs;
    svc->onrestart.name = "onrestart";
    list_init(&svc->onrestart.commands);
    list_add_tail(&service_list, &svc->slist);
    return svc;
    */
  return 0;
}

static void parse_line_service(struct parse_context *state, int nargs, char **args)
{
#if 0
    struct service *svc = state->context;
    struct command *cmd;
    int i, kw, kw_nargs;

    if (nargs == 0) {
        return;
    }

    svc->ioprio_class = ioschedclass_none;

    kw = lookup_keyword(args[0]);
    switch (kw) {
    case k_capability:
        break;
    case k_class:
        if (nargs != 2) {
            parse_error(state, "class option requires a classname\n");
        } else {
            svc->classname = args[1];
        }
        break;
    case k_console:
        svc->flags |= svc_console;
        break;
    case k_disabled:
        svc->flags |= svc_disabled;
        svc->flags |= svc_rc_disabled;
        break;
    case k_ioprio:
        if (nargs != 3) {
            parse_error(state, "ioprio optin usage: ioprio <rt|be|idle> <ioprio 0-7>\n");
        } else {
            svc->ioprio_pri = strtoul(args[2], 0, 8);

            if (svc->ioprio_pri < 0 || svc->ioprio_pri > 7) {
                parse_error(state, "priority value must be range 0 - 7\n");
                break;
            }

            if (!strcmp(args[1], "rt")) {
                svc->ioprio_class = ioschedclass_rt;
            } else if (!strcmp(args[1], "be")) {
                svc->ioprio_class = ioschedclass_be;
            } else if (!strcmp(args[1], "idle")) {
                svc->ioprio_class = ioschedclass_idle;
            } else {
                parse_error(state, "ioprio option usage: ioprio <rt|be|idle> <0-7>\n");
            }
        }
        break;
    case k_group:
        if (nargs < 2) {
            parse_error(state, "group option requires a group id\n");
        } else if (nargs > nr_svc_supp_gids + 2) {
            parse_error(state, "group option accepts at most %d supp. groups\n",
                        nr_svc_supp_gids);
        } else {
            int n;
            svc->gid = decode_uid(args[1]);
            for (n = 2; n < nargs; n++) {
                svc->supp_gids[n-2] = decode_uid(args[n]);
            }
            svc->nr_supp_gids = n - 2;
        }
        break;
    case k_keycodes:
        if (nargs < 2) {
            parse_error(state, "keycodes option requires atleast one keycode\n");
        } else {
            svc->keycodes = malloc((nargs - 1) * sizeof(svc->keycodes[0]));
            if (!svc->keycodes) {
                parse_error(state, "could not allocate keycodes\n");
            } else {
                svc->nkeycodes = nargs - 1;
                for (i = 1; i < nargs; i++) {
                    svc->keycodes[i - 1] = atoi(args[i]);
                }
            }
        }
        break;
    case k_oneshot:
        svc->flags |= svc_oneshot;
        break;
    case k_onrestart:
        nargs--;
        args++;
        kw = lookup_keyword(args[0]);
        if (!kw_is(kw, command)) {
            parse_error(state, "invalid command '%s'\n", args[0]);
            break;
        }
        kw_nargs = kw_nargs(kw);
        if (nargs < kw_nargs) {
            parse_error(state, "%s requires %d %s\n", args[0], kw_nargs - 1,
                kw_nargs > 2 ? "arguments" : "argument");
            break;
        }

        cmd = malloc(sizeof(*cmd) + sizeof(char*) * nargs);
        cmd->func = kw_func(kw);
        cmd->nargs = nargs;
        memcpy(cmd->args, args, sizeof(char*) * nargs);
        list_add_tail(&svc->onrestart.commands, &cmd->clist);
        break;
    case k_critical:
        svc->flags |= svc_critical;
        break;
    case k_setenv: { /* name value */
        struct svcenvinfo *ei;
        if (nargs < 3) {
            parse_error(state, "setenv option requires name and value arguments\n");
            break;
        }
        ei = calloc(1, sizeof(*ei));
        if (!ei) {
            parse_error(state, "out of memory\n");
            break;
        }
        ei->name = args[1];
        ei->value = args[2];
        ei->next = svc->envvars;
        svc->envvars = ei;
        break;
    }
    case k_socket: {/* name type perm [ uid gid context ] */
        struct socketinfo *si;
        if (nargs < 4) {
            parse_error(state, "socket option requires name, type, perm arguments\n");
            break;
        }
        if (strcmp(args[2],"dgram") && strcmp(args[2],"stream")
                && strcmp(args[2],"seqpacket")) {
            parse_error(state, "socket type must be 'dgram', 'stream' or 'seqpacket'\n");
            break;
        }
        si = calloc(1, sizeof(*si));
        if (!si) {
            parse_error(state, "out of memory\n");
            break;
        }
        si->name = args[1];
        si->type = args[2];
        si->perm = strtoul(args[3], 0, 8);
        if (nargs > 4)
            si->uid = decode_uid(args[4]);
        if (nargs > 5)
            si->gid = decode_uid(args[5]);
        if (nargs > 6)
            si->socketcon = args[6];
        si->next = svc->sockets;
        svc->sockets = si;
        break;
    }
    case k_user:
        if (nargs != 2) {
            parse_error(state, "user option requires a user id\n");
        } else {
            svc->uid = decode_uid(args[1]);
        }
        break;
    case k_seclabel:
        if (nargs != 2) {
            parse_error(state, "seclabel option requires a label string\n");
        } else {
            svc->seclabel = args[1];
        }
        break;

    default:
        parse_error(state, "invalid option '%s'\n", args[0]);
    }
#endif
}

#endif


