/*
 * filename   : test.c
 * created at : 2023-01-09 19:01:59
 * author     : xiliuya <xiliuya@163.com>
 */

#include "dwl.c"
#include <emacs-module.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Declare mandatory GPL symbol.  */
int plugin_is_GPL_compatible;
/* Bind NAME to FUN.  */
static void bind_function(emacs_env *env, const char *name, emacs_value Sfun) {
  /* Set the function cell of the symbol named NAME to SFUN using
     the 'fset' function.  */

  /* Convert the strings to symbols by interning them */
  emacs_value Qfset = env->intern(env, "fset");
  emacs_value Qsym = env->intern(env, name);

  /* Prepare the arguments array */
  emacs_value args[] = {Qsym, Sfun};

  /* Make the call (2 == nb of arguments) */
  env->funcall(env, Qfset, 2, args);
}

/* Provide FEATURE to Emacs.  */
static void provide(emacs_env *env, const char *feature) {
  /* call 'provide' with FEATURE converted to a symbol */

  emacs_value Qfeat = env->intern(env, feature);
  emacs_value Qprovide = env->intern(env, "provide");
  emacs_value args[] = {Qfeat};

  env->funcall(env, Qprovide, 1, args);
}

/* Message FEATURE to Emacs.  */
static void message(emacs_env *env, const char *name) {
  /* call 'message' with FEATURE converted to a symbol */

  size_t name_len = strlen(name);
  emacs_value Qname = env->make_string(env, name, (ptrdiff_t)name_len);
  emacs_value Qmessage = env->intern(env, "message");
  emacs_value args[] = {Qname};

  env->funcall(env, Qmessage, 1, args);
}

emacs_value emacs_message(emacs_env *env, const char *msg, int nargs, ...) {
  emacs_value Smessage = env->intern(env, "message");

  int i;

  emacs_value args[nargs + 1];

  args[0] = env->make_string(env, msg, strlen(msg));

  va_list ap;
  va_start(ap, nargs);

  for (i = 0; i < nargs; i++)
    args[i + 1] = va_arg(ap, emacs_value); /* Get the next argument value. */

  va_end(ap); /* Clean up. */

  emacs_value result = env->funcall(env, Smessage, nargs + 1, args);

  return result;
}
// Extract a string from arg. if it is a string we get it.
// Otherwise we format it with %S.
char *extract_string(emacs_env *env, emacs_value arg) {
  emacs_value type = env->type_of(env, arg);
  ptrdiff_t size = 0;

  if (env->eq(env, type, env->intern(env, "string"))) {
    // the first copy puts the string length into the variable
    env->copy_string_contents(env, arg, NULL, &size);

    // then we can allocate the string and copy into it.
    char *result = malloc(size);
    env->copy_string_contents(env, arg, result, &size);
    return result;
  }

  else {
    emacs_value msg = emacs_message(env, "got msg: %S", 1, arg);
    fprintf(stderr, "size-2: %td\n", size);
    // the first copy puts the string length into the variable
    env->copy_string_contents(env, msg, NULL, &size);

    // then we can allocate the string and copy into it.
    char *result = malloc(size);
    env->copy_string_contents(env, msg, result, &size);
    return result;
  }
}

void *my_run(void *startup_cmd) {
  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto(dpy);
  if (!socket)
    die("startup: display_add_socket_auto");
  setenv("WAYLAND_DISPLAY", socket, 1);

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(backend))
    die("startup: backend_start");

  /* Now that the socket exists and the backend is started, run the startup
   * command */
  if (startup_cmd) {
    int piperw[2];
    if (pipe(piperw) < 0)
      die("startup: pipe:");
    if ((child_pid = fork()) < 0)
      die("startup: fork:");
    if (child_pid == 0) {
      dup2(piperw[0], STDIN_FILENO);
      close(piperw[0]);
      close(piperw[1]);
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, NULL);
      die("startup: execl:");
    }
    /* dup2(piperw[1], STDOUT_FILENO); */
    /* close(piperw[1]); */
    /* close(piperw[0]); */
  }
  /* If nobody is reading the status output, don't terminate */
  /* signal(SIGPIPE, SIG_IGN); */
  /* printstatus(); */

  /* At this point the outputs are initialized, choose initial selmon based on
   * cursor position, and set default cursor image */
  selmon = xytomon(cursor->x, cursor->y);

  /* TODO hack to get cursor to display in its initial location (100, 100)
   * instead of (0, 0) and then jumping.  still may not be fully
   * initialized, as the image/coordinates are not transformed for the
   * monitor when displayed here */
  wlr_cursor_warp_closest(cursor, NULL, cursor->x, cursor->y);
  wlr_xcursor_manager_set_cursor_image(cursor_mgr, cursor_image, cursor);

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wl_display_run(dpy);
  return 0;
}
/* New emacs lisp function. All function exposed to Emacs must have this
 * prototype. */
static emacs_value Fmymod_test(emacs_env *env, long int nargs,
                               emacs_value args[], void *data) {
  printf("hi");
  Client *c;
  while (1) {
    sleep(1);
    wl_list_for_each(c, &fstack, flink)
        printf("%s %s", client_get_title(c), client_get_appid(c));
  }
  /* system("alacritty"); */
  return env->make_integer(env, 42);
}

static emacs_value Femdwl_init(emacs_env *env, long int nargs,
                               emacs_value args[], void *data) {
  printf("Run emdwl_init");
  if (!getenv("XDG_RUNTIME_DIR"))
    die("XDG_RUNTIME_DIR must be set");
  setup();

  /* system("alacritty"); */
  return env->make_integer(env, 1);
}

static emacs_value Femdwl_run(emacs_env *env, long int nargs,
                              emacs_value args[], void *data) {
  pthread_t ptid;
  printf("Run emdwl_run \n");
  /* run("emacs"); */
  printf("%s\n", extract_string(env, args[0]));
  /* system(extract_string(env, args[0])); */
  pthread_create(&ptid, NULL, &my_run, extract_string(env, args[0]));
  // my_run(extract_string(env, args[0]));
  return env->make_integer(env, 1);
}

static emacs_value Femdwl_list(emacs_env *env, long int nargs,
                               emacs_value args[], void *data) {
  printf("Run emdwl_list \n");
  /* printf("%s\n", extract_string(env, args[0])); */
  /* system(extract_string(env, args[0])); */
  Client *c;
  wl_list_for_each(c, &fstack, flink) {
    printf("%s %s", client_get_title(c), client_get_appid(c));
    /* message(env, client_get_title(c)); */
    emacs_message(env, "title: %S ,appid: %S", 2,
                  (env->make_string(env, client_get_title(c),
                                    strlen(client_get_title(c)))),
                  (env->make_string(env, client_get_appid(c),
                                    strlen(client_get_appid(c)))));
    // emacs_message(env, "%S : %S", 2, client_get_title(c),
    // client_get_appid(c));
  }
  return env->make_integer(env, 1);
}

int emacs_module_init(struct emacs_runtime *ert) {
  emacs_env *env = ert->get_environment(ert);

  /* create a lambda (returns an emacs_value) */
  emacs_value fun = env->make_function(
      env, 0,      /* min. number of arguments */
      0,           /* max. number of arguments */
      Fmymod_test, /* actual function pointer */
      "doc",       /* docstring */
      NULL         /* user pointer of your choice (data param in Fmymod_test) */
  );

  bind_function(env, "mymod-test", fun);

  /* create a init (returns an emacs_value) */
  emacs_value fun_init = env->make_function(
      env, 0,      /* min. number of arguments */
      0,           /* max. number of arguments */
      Femdwl_init, /* actual function pointer */
      "Init dwl",  /* docstring */
      NULL         /* user pointer of your choice (data param in Fmymod_test) */
  );

  bind_function(env, "mymod-init", fun_init);

  /* create a run (returns an emacs_value) */
  emacs_value fun_emdwl_run = env->make_function(
      env, 1,             /* min. number of arguments */
      1,                  /* max. number of arguments */
      Femdwl_run,         /* actual function pointer */
      "Run some window.", /* docstring */
      NULL /* user pointer of your choice (data param in Fmymod_test) */
  );

  bind_function(env, "mymod-run", fun_emdwl_run);

  /* create a list (returns an emacs_value) */
  emacs_value fun_list = env->make_function(
      env, 0,      /* min. number of arguments */
      0,           /* max. number of arguments */
      Femdwl_list, /* actual function pointer */
      "Init dwl",  /* docstring */
      NULL         /* user pointer of your choice (data param in Fmymod_test) */
  );

  bind_function(env, "mymod-list", fun_list);

  message(env, "Hi it my mod");

  /* if (!getenv("XDG_RUNTIME_DIR")) */
  /*       	die("XDG_RUNTIME_DIR must be set"); */
  /* setup(); */
  /* /\* system("alacritty &"); *\/ */
  /* run("xfce4-terminal"); */
  /* cleanup(); */
  message(env, "Hi my dwl");

  provide(env, "mymod");

  /* loaded successfully */
  return 0;
}
