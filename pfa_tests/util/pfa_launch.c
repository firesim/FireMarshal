#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  int pid = getpid();

  /* Register with cgroup */
  FILE *cgroup_ctl = fopen("/sys/fs/cgroup/pfa_cg/cgroup.procs", "w");
  fprintf(cgroup_ctl, "%d\n", pid);
  fclose(cgroup_ctl);

  /* Register with pfa */
  FILE *pfa_ctl = fopen("/sys/kernel/pfa/pfa_tsk", "w");
  fprintf(pfa_ctl, "%d\n", pid);
  fclose(pfa_ctl);

  /* int res = syscall(SYS_pfa); */
  /* if(!res) { */
  /*   printf("Failed to register task with PFA\n"); */
  /*   return EXIT_FAILURE; */
  /* } */

  printf("Launching \"%s\" as pfa task (pid=%d)\n", argv[1], pid);
  execvp(argv[1], &argv[1]);
}
