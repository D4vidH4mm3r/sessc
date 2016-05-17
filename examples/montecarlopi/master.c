#include <stdio.h>
#include <stdlib.h>
#include <sc.h>

int main(int argc, char *argv[]) {
  session *s;
  session_init(&argc, &argv, &s, "master.spr");
  role *W0 = s->r(s, "Worker0");
  role *W1 = s->r(s, "Worker1");

  int count = 5;
  send_int(count, W0, NULL);
  send_int(count, W1, NULL);
  int inside;
  long num_in = 0;
  long num_total = 0;
  for (int i=0; i<count; ++i) {
    recv_int(&inside, W0);
    num_total++;
    if (inside) {
      num_in++;
    }
    recv_int(&inside, W1);
    printf("Master received %d\n", inside);
    num_total++;
    if (inside) {
      num_in++;
    }
  }

  session_dump(s); // debugging
  session_end(s);

  return EXIT_SUCCESS;
}
