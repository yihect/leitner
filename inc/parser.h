#ifndef _LTD_PARSER_H_
#define _LTD_PARSER_H_

struct parse_context {
  const char *filename;
  char *ptr;
  char *text;
  int line;
  int nexttoken;
  void *context;
  void (*parse_line)(struct parse_context *state, int nargs, char **args);
  void (*old_parse_line)(struct parse_context *state, int nargs, char **args);
};

struct kword {
  const char *name;
  int len;
  unsigned char nargs;
  unsigned char flags;
  void* (*func)(struct parse_context *state, int nargs, char **args);
  void (*line_func)(struct parse_context *state, int nargs, char **args);
};

#if 0
#define INIT_PARSER_MAXARGS 64

struct action;

struct action *action_remove_queue_head(void);
void action_add_queue_tail(struct action *act);
void action_for_each_trigger(const char *trigger,
                             void (*func)(struct action *act));
int action_queue_empty(void);
void queue_property_triggers(const char *name, const char *value);
void queue_all_property_triggers();
void queue_builtin_action(int (*func)(int nargs, char **args), char *name);

int init_parse_config_file(const char *fn);
int expand_props(char *dst, const char *src, int len);
#endif

static void *parse_rfx(struct parse_context *state, int nargs, char **args);
static void parse_line_rfx(struct parse_context *state, int nargs, char **args);

static void *parse_voc(struct parse_context *state, int nargs, char **args);
static void parse_line_voc(struct parse_context *state, int nargs, char **args);

static void *parse_vgrp(struct parse_context *state, int nargs, char **args);
static void parse_line_vgrp(struct parse_context *state, int nargs, char **args);

static void *parse_gram(struct parse_context *state, int nargs, char **args);
static void parse_line_gram(struct parse_context *state, int nargs, char **args);

static void *parse_vs(struct parse_context *state, int nargs, char **args);
static void parse_line_vs(struct parse_context *state, int nargs, char **args);

static void *parse_item(struct parse_context *state, int nargs, char **args);
static void parse_line_item(struct parse_context *state, int nargs, char **args);

int ltd_parse_file(const char *fn);




#endif
