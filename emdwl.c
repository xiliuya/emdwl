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
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wayland-server-core.h>

void
msleep (int tms)
{
  struct timeval tv;
  tv.tv_sec = tms / 1000;
  tv.tv_usec = (tms % 1000) * 1000;
  select (0, NULL, NULL, NULL, &tv);
}

/* Declare mandatory GPL symbol.  */
int plugin_is_GPL_compatible;

/* thread id */
pthread_t ptid;
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
  emacs_value Qname = env->make_string (env, name, (ptrdiff_t)name_len);
  emacs_value Qmessage = env->intern (env, "message");
  emacs_value args[] = { Qname };

  env->funcall (env, Qmessage, 1, args);
}

emacs_value
emacs_message (emacs_env *env, const char *msg, int nargs, ...)
{
  emacs_value Smessage = env->intern (env, "message");

  int i;

  emacs_value args[nargs + 1];

  va_list ap;

  args[0] = env->make_string (env, msg, strlen (msg));

  va_start (ap, nargs);

  for (i = 0; i < nargs; i++)
    args[i + 1] = va_arg (ap, emacs_value); /* Get the next argument value. */

  va_end (ap); /* Clean up. */

  return env->funcall (env, Smessage, nargs + 1, args);
}
// Extract a string from arg. if it is a string we get it.
// Otherwise we format it with %S.
char *
extract_string (emacs_env *env, emacs_value arg)
{
  emacs_value type = env->type_of (env, arg);
  ptrdiff_t size = 0;
  char *result;
  if (env->eq (env, type, env->intern (env, "string")))
    {
      // the first copy puts the string length into the variable
      env->copy_string_contents (env, arg, NULL, &size);

      // then we can allocate the string and copy into it.
      result = malloc (size);
      env->copy_string_contents (env, arg, result, &size);
      return result;
    }

  else
    {
      emacs_value msg = emacs_message (env, "got msg: %S", 1, arg);
      fprintf (stderr, "size-2: %td\n", size);
      // the first copy puts the string length into the variable
      env->copy_string_contents (env, msg, NULL, &size);

      // then we can allocate the string and copy into it.
      result = malloc (size);
      env->copy_string_contents (env, msg, result, &size);
      return result;
    }
}
// Extract a number as an integer from arg. floats are cast as ints.
int
extract_integer (emacs_env *env, emacs_value arg)
{
  emacs_value type = env->type_of (env, arg);
  emacs_value Sint = env->intern (env, "integer");
  emacs_value Sfloat = env->intern (env, "float");

  int result = 0;
  if (env->eq (env, type, Sint))
    {
      result = env->extract_integer (env, arg);
    }
  else if (env->eq (env, type, Sfloat))
    {
      result = (int)env->extract_float (env, arg);
    }
  else
    {
      emacs_value signal = env->intern (env, "type-error");
      const char *error = "A non-number arg was passed.";
      emacs_value message = env->make_string (env, error, strlen (error));
      env->non_local_exit_signal (env, signal, message);
    }

  return result;
}

/* Call emdwl-tool-new-buffer */
char *
emdwl_newbuffer (emacs_env *env, const char *name)
{

  size_t name_len = strlen (name);
  emacs_value Qname = env->make_string (env, name, (ptrdiff_t)name_len);
  emacs_value Qmessage = env->intern (env, "emdwl-tool-new-buffer");
  emacs_value args[] = { Qname };

  emacs_value result = env->funcall (env, Qmessage, 1, args);

  char *buffer_name = extract_string (env, result);
  return buffer_name;
}

/* Find client with buffer_name */

/* Client * */
/* emdwl_find_client (const char *buffer_name) */
/* { */

/*   Client *c; */
/*   wl_list_for_each (c, &fstack, flink) */
/*   { */
/*     if (c->buffer_name != NULL && strcmp (c->buffer_name, buffer_name) == 0) */
/*       { */
/*         printf ("buffer_name: %s\n", c->buffer_name); */
/*         fflush (stdout); */
/*         return c; */
/*       } */
/*   } */
/*   return NULL; */
/* } */

/* Find client with client_id */
Client *
emdwl_find_client_with_id (int id)
{

  Client *c;

  if (id == 0)
    {
      return NULL;
    }

  wl_list_for_each (c, &fstack, flink)
  {
    if (c->client_id != 0 && c->client_id == id)
      {
        printf ("client_id: %u\n", c->client_id);
        fflush (stdout);
        return c;
      }
  }
  return NULL;
}
/* Close client */
int
emdwl_close_client (int id)
{
  Client *c;
  /* c = emdwl_find_client (buffer_name); */
  c = emdwl_find_client_with_id (id);
  if (c == NULL)
    {
      return 0;
    }
  client_send_close (c);
  return 1;
}

int
emdwl_open_client (const char *run_cmd)
{
  /* Client *c; */
  int read_id = 0;
  int pid;
  int piperw[2];
  int old_client_id;
  int i = 100;
  old_client_id = client_id;
  if (child_pid < 0)
    {
      if (pipe (piperw) < 0)
        die ("startup: pipe:");
      if ((child_pid = fork ()) < 0)
        die ("startup: fork:");
      if (child_pid == 0)
        {
          dup2 (piperw[0], STDIN_FILENO);
          close (piperw[0]);
          close (piperw[1]);
          execl ("/bin/sh", "/bin/sh", "-c", run_cmd, NULL);
          read_id = 0;
          die ("startup: execl:");
        }
      else
        {
          /* Read client_id in pipe */
          printf ("client_id is:%d\n", read_id);
          fflush (stdout);
        }
      /* dup2 (piperw[1], STDOUT_FILENO); */
      /* close (piperw[1]); */
      /* close (piperw[0]); */
    }
  else
    {
      /* const char *buffer_name = extract_string (env, args[1]); */
      pid = fork ();
      if (pid < 0)
        {
          die ("open: fork:");
        }
      else if (pid == 0)
        {
          printf ("Run emdwl_open \n");
          execl ("/bin/sh", "/bin/sh", "-c", run_cmd, NULL);
          read_id = 0;
          exit (0);
        }
      else
        {
          /* Read client_id in pipe */
          read_id = 0;
          while (i--)
            {
              if (old_client_id == client_id)
                read_id = old_client_id;
              msleep (10);
            }
          printf ("client_id is:%d\n", read_id);
          fflush (stdout);
        }
    }
  signal (SIGPIPE, SIG_IGN);
  printf ("emdwl_open read pipe\n");
  /* read_id = 10000; */

  /* /\* Read client_id in pipe *\/ */
  /* close (dwl_pipe[1]); */
  /* read (dwl_pipe[0], msg, 2); */
  /* read_id = msg[1]; */
  /* read_id = read_id << 8; */
  /* read_id += msg[0]; */
  /* printf ("client_id is:%d\n", read_id); */
  /* fflush (stdout); */
  /* close (dwl_pipe[0]); */
  return read_id;
}

void
print_pid (Client *c)
{
  int pid;
  wl_client_get_credentials (c->surface.xdg->client->client, &pid, NULL, NULL);
  printf ("pid id: %d c->client: %d\n", pid, c->client_id);
}
void *
my_run (void *startup_cmd)
{
  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto (dpy);
  if (!socket)
    die ("startup: display_add_socket_auto");
  setenv ("WAYLAND_DISPLAY", socket, 1);

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start (backend))
    die ("startup: backend_start");

  /* Now that the socket exists and the backend is started, run the startup
   * command */
  if (startup_cmd)
    {
      /* int piperw[2]; */
      /* if (pipe (piperw) < 0) */
      /*   die ("startup: pipe:"); */
      /* if ((child_pid = fork ()) < 0) */
      /*   die ("startup: fork:"); */
      /* if (child_pid == 0) */
      /*   { */
      /*     dup2 (piperw[0], STDIN_FILENO); */
      /*     close (piperw[0]); */
      /*     close (piperw[1]); */
      /*     execl ("/bin/sh", "/bin/sh", "-c", startup_cmd, NULL); */
      /*     die ("startup: execl:"); */
      /*   } */
      /* /\* dup2(piperw[1], STDOUT_FILENO); *\/ */
      /* /\* close(piperw[1]); *\/ */
      /* /\* close(piperw[0]); *\/ */
      emdwl_open_client ("alacritty");
    }
  /* If nobody is reading the status output, don't terminate */
  /* signal(SIGPIPE, SIG_IGN); */
  /* printstatus(); */

  /* At this point the outputs are initialized, choose initial selmon based on
   * cursor position, and set default cursor image */
  selmon = xytomon (cursor->x, cursor->y);

  /* TODO hack to get cursor to display in its initial location (100, 100)
   * instead of (0, 0) and then jumping.  still may not be fully
   * initialized, as the image/coordinates are not transformed for the
   * monitor when displayed here */
  wlr_cursor_warp_closest (cursor, NULL, cursor->x, cursor->y);
  wlr_xcursor_manager_set_cursor_image (cursor_mgr, cursor_image, cursor);

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wl_display_run (dpy);
  /* pthread_exit (0); */
  return 0;
}
/* New emacs lisp function. All function exposed to Emacs must have this
 * prototype. */
static emacs_value
Fmymod_test (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  Client *c;
  printf ("hi");
  while (1)
    {
      sleep (1);
      wl_list_for_each (c, &fstack, flink)
          printf ("%s %s", client_get_title (c), client_get_appid (c));
    }
  /* system("alacritty"); */
  return env->make_integer (env, 42);
}

static emacs_value
Femdwl_init (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  printf ("Run emdwl_init");
  if (!getenv ("XDG_RUNTIME_DIR"))
    die ("XDG_RUNTIME_DIR must be set");
  setup ();

  /* system("alacritty"); */
  return env->make_integer (env, 1);
}

static emacs_value
Femdwl_run (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  printf ("Run emdwl_run \n");
  /* run("emacs"); */
  printf ("%s\n", extract_string (env, args[0]));
  /* system(extract_string(env, args[0])); */
  pthread_create (&ptid, NULL, &my_run, extract_string (env, args[0]));
  // my_run(extract_string(env, args[0]));
  return env->make_integer (env, 1);
}

static emacs_value
Femdwl_list (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  /* printf("%s\n", extract_string(env, args[0])); */
  /* system(extract_string(env, args[0])); */
  Client *c;
  printf ("Run emdwl_list \n");
  wl_list_for_each (c, &fstack, flink)
  {
    printf ("%s %s", client_get_title (c), client_get_appid (c));
    /* message(env, client_get_title(c)); */
    emacs_message (env, "title: %S ,appid: %S", 2,
                   (env->make_string (env, client_get_title (c),
                                      strlen (client_get_title (c)))),
                   (env->make_string (env, client_get_appid (c),
                                      strlen (client_get_appid (c)))));
    /* if (c->buffer_name == NULL) */
    /*   { */
    /*     c->buffer_name = emdwl_newbuffer (env, client_get_appid (c)); */
    /*   } */
  }
  return env->make_integer (env, 1);
}

static emacs_value
Femdwl_open (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  const char *run_cmd = extract_string (env, args[0]);
  return env->make_integer (env, emdwl_open_client (run_cmd));
}
static emacs_value
Femdwl_resize (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  /* printf("%s\n", extract_string(env, args[0])); */
  /* system(extract_string(env, args[0])); */
  Client *c;
  // c = focustop(selmon);
  int x = extract_integer (env, args[0]);
  int y = extract_integer (env, args[1]);
  int width = extract_integer (env, args[2]);
  int height = extract_integer (env, args[3]);
  int id = extract_integer (env, args[4]);
  /* const char *buffer_name = extract_string (env, args[4]); */

  printf ("Run emdwl_resize \n");
  printf ("id is:%d\n", id);
  /* c = emdwl_find_client (buffer_name); */
  c = emdwl_find_client_with_id (id);
  if (c == NULL)
    return env->make_integer (env, 0);
  else
    {
      setfloating (c, 1);
      resize (
          c,
          (struct wlr_box){ .x = x, .y = y, .width = width, .height = height },
          1);
      /* c->resize = client_set_size(c, width, height); */
    }
  print_pid (c);
  return env->make_integer (env, 1);
}

static emacs_value
Femdwl_close_client (emacs_env *env, long int nargs, emacs_value args[],
                     void *data)
{
  int id = extract_integer (env, args[0]);
  printf ("Run emdwl_close_client \n");
  printf ("%d\n", id);
  return env->make_integer (env, emdwl_close_client (id));
}

static emacs_value
Femdwl_newtags_client (emacs_env *env, long int nargs, emacs_value args[],
                       void *data)
{
  Client *c;
  unsigned int tag = 0;
  /* const char *buffer_name; */
  int id = extract_integer (env, args[0]);
  printf ("Run emdwl_newtags_client \n");
  /* buffer_name = extract_string (env, args[0]); */
  tag = extract_integer (env, args[1]);
  printf ("%d--%d\n", id, tag);
  /* c = emdwl_find_client (buffer_name); */
  c = emdwl_find_client_with_id (id);
  if (c == NULL)
    {
      return env->make_integer (env, 0);
    }

  c->tags = 1 << tag;
  focusclient (focustop (selmon), 1);
  arrange (selmon);
  return env->make_integer (env, 1);
}

static emacs_value
Femdwl_close (emacs_env *env, long int nargs, emacs_value args[], void *data)
{
  int i;
  printf ("Run emdwl_close \n");
  cleanup ();
  i = pthread_cancel (ptid);
  return env->make_integer (env, i);
}
int
emacs_module_init (struct emacs_runtime *ert)
{

  emacs_env *env = ert->get_environment (ert);

  /* create a lambda (returns an emacs_value) */
  emacs_value fun = env->make_function (
      env, 0,      /* min. number of arguments */
      0,           /* max. number of arguments */
      Fmymod_test, /* actual function pointer */
      "doc",       /* docstring */
      NULL /* user pointer of your choice (data param in Fmymod_test) */
  );

  /* create a init (returns an emacs_value) */
  emacs_value fun_init = env->make_function (
      env, 0,      /* min. number of arguments */
      0,           /* max. number of arguments */
      Femdwl_init, /* actual function pointer */
      "Init dwl",  /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* create a run (returns an emacs_value) */
  emacs_value fun_emdwl_run = env->make_function (
      env, 1,             /* min. number of arguments */
      1,                  /* max. number of arguments */
      Femdwl_run,         /* actual function pointer */
      "Run some window.", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* message client list (returns an emacs_value) */
  emacs_value fun_list = env->make_function (
      env, 0,                /* min. number of arguments */
      0,                     /* max. number of arguments */
      Femdwl_list,           /* actual function pointer */
      "Client list message", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* open a client  (returns an client_id) */
  emacs_value fun_open = env->make_function (
      env, 1,                /* min. number of arguments */
      1,                     /* max. number of arguments */
      Femdwl_open,           /* actual function pointer */
      "Client list message", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* resize a client (returns an emacs_value) */
  emacs_value fun_resize = env->make_function (
      env, 5,            /* min. number of arguments */
      5,                 /* max. number of arguments */
      Femdwl_resize,     /* actual function pointer */
      "Resize a client", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* close a client (returns an emacs_value) */
  emacs_value fun_close_client = env->make_function (
      env, 1,                          /* min. number of arguments */
      1,                               /* max. number of arguments */
      Femdwl_close_client,             /* actual function pointer */
      "Close a client with client_id", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* close a client (returns an emacs_value) */
  emacs_value fun_newtags_client = env->make_function (
      env, 2,                          /* min. number of arguments */
      2,                               /* max. number of arguments */
      Femdwl_newtags_client,           /* actual function pointer */
      "Close a client with client_id", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* create a run (returns an emacs_value) */
  emacs_value fun_emdwl_close = env->make_function (
      env, 0,             /* min. number of arguments */
      0,                  /* max. number of arguments */
      Femdwl_close,       /* actual function pointer */
      "Run some window.", /* docstring */
      NULL /* user pointer of your choice (data param in Femdwl_test) */
  );

  /* Bind function */
  bind_function (env, "mymod-test", fun);
  bind_function (env, "emdwl-init", fun_init);
  bind_function (env, "emdwl-run", fun_emdwl_run);
  bind_function (env, "emdwl-list", fun_list);
  bind_function (env, "emdwl-open", fun_open);
  bind_function (env, "emdwl-resize", fun_resize);
  bind_function (env, "emdwl-close-client", fun_close_client);
  bind_function (env, "emdwl-newtags-client", fun_newtags_client);
  bind_function (env, "emdwl-close", fun_emdwl_close);

  message (env, "Hi it my emdwl");

  /* if (!getenv("XDG_RUNTIME_DIR")) */
  /*       	die("XDG_RUNTIME_DIR must be set"); */
  /* setup(); */
  /* /\* system("alacritty &"); *\/ */
  /* run("xfce4-terminal"); */
  /* cleanup(); */
  message (env, "Hi my dwl");

  provide (env, "emdwl");

  /* loaded successfully */
  return 0;
}
