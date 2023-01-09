/*
 * filename   : test.c
 * created at : 2023-01-09 19:01:59
 * author     : xiliuya <xiliuya@163.com>
 */

#include <emacs-module.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Declare mandatory GPL symbol.  */
int plugin_is_GPL_compatible;

/* New emacs lisp function. All function exposed to Emacs must have this prototype. */
static emacs_value
Fmymod_test (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  printf("hi");
  /* system("alacritty"); */
  return env->make_integer (env, 42);
}

/* Bind NAME to FUN.  */
static void
bind_function (emacs_env *env, const char *name, emacs_value Sfun)
{
  /* Set the function cell of the symbol named NAME to SFUN using
     the 'fset' function.  */

  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern (env, "fset");
  emacs_value Qsym = env->intern (env, name);

  /* Prepare the arguments array */
  emacs_value args[] = { Qsym, Sfun };

  /* Make the call (2 == nb of arguments) */
  env->funcall (env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void
provide (emacs_env *env, const char *feature)
{
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern (env, feature);
  emacs_value Qprovide = env->intern (env, "provide");
  emacs_value args[] = { Qfeat };

  env->funcall (env, Qprovide, 1, args);
}

/* Message FEATURE to Emacs.  */
static void
message (emacs_env *env, const char *name)
{
  /* call 'message' with FEATURE converted to a symbol */

  size_t name_len = strlen (name);
  emacs_value Qname = env->make_string (env, name, (ptrdiff_t) name_len);
  emacs_value Qmessage = env->intern (env, "message");
  emacs_value args[] = { Qname };

  env->funcall (env, Qmessage, 1, args);
}


int
emacs_module_init (struct emacs_runtime *ert)
{
  emacs_env *env = ert->get_environment (ert);

  /* create a lambda (returns an emacs_value) */
  emacs_value fun = env->make_function (env,
              0,            /* min. number of arguments */
              0,            /* max. number of arguments */
              Fmymod_test,  /* actual function pointer */
              "doc",        /* docstring */
              NULL          /* user pointer of your choice (data param in Fmymod_test) */
  );

  bind_function (env, "mymod-test", fun);
  message (env, "Hi it my mod");
  /* if (!getenv("XDG_RUNTIME_DIR")) */
  /*       	die("XDG_RUNTIME_DIR must be set"); */
  /* setup(); */
  /* run("alacritty"); */
  /* cleanup(); */
  provide (env, "mymod");
  /* loaded successfully */
  return 0;
}

