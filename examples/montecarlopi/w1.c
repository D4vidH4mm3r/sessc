#include <stdio.h>
#include <stdlib.h>

#include <sc.h>

int main(int argc, char *argv[]) {
  session *s;
  session_init(&argc, &argv, &s, "w0.spr");
  role *M = s->r(s, "Master");
  session_dump(s);

  int count;
  recv_int(&count, M);
  for (int i=0; i<count; ++i) {
    send_int(1, M, NULL);
  }

  session_end(s);

  return EXIT_SUCCESS;
}
