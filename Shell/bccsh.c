#include "bccsh.h"

int main()
{
  char * c;
  using_history();

  while((c = readline(">> "))) {
    /* printf("Eu recebi %s\n", c); */

    if (!strcmp(c, "/usr/bin/du -hs .")) {
      printf("execuanto du...");
    }
    else if (!strcmp(c, "/usr/bin/traceroute www.google.com.br")) {
      printf("execuanto traceroute...\n");
    }

    else if (!strncmp(c, "mkdir", 5)) {
      printf("executando mkdir\n");
    }
    else if (!strncmp(c, "kill -9", 7)) {
      printf("executando kill...\n");
    }
    else if (!strncmp(c, "ln -s", 5)) {
      printf("executando ln...\n");
    }
    else {
      printf("Comando desconhecido\n");
    }

    add_history(c);
    free(c);
  }
  printf("\n");

  return EXIT_SUCCESS;
}
