#define _GNU_SOURCE

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <alloca.h>
#include <linux/sched.h>
#include <sched.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>

/* Locally define the new clone flags, to avoid changing bionic for now */
#define CLONE_NEWUTS         0x04000000
#define CLONE_NEWIPC         0x08000000
#define CLONE_NEWUSER        0x10000000
#define CLONE_NEWPID         0x20000000
#define CLONE_NEWNET         0x40000000
#define CLONE_IO             0x80000000


static int ubuntum(void *a) {
	/* Chroot */
	chroot("/data/ubuntu");
	chdir("/");

	/* Set basic env variables */
	setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
	setenv("SHELL", "/bin/bash", 1);
	setenv("HOME", "/root", 1);

	/* Exec shell */
	printf("Calling UBUNTU init...\n");
	execl("/sbin/init", "/sbin/init", "--verbose", NULL);
//	execl("/bin/bash", "/bin/bash", NULL);

	return 0;
}

int main() {
	pid_t pid;
	int ret;
	long stack_size = sysconf(_SC_PAGESIZE);
	void *stack = alloca(stack_size) + stack_size;

	/* Mount /dev so console can work from start */
	mount("/dev", "/data/ubuntu/dev", "", MS_BIND, "");

	/* New process with its own PID/IPC/NS namespace */
	pid = clone(ubuntum, stack, SIGCHLD | CLONE_NEWPID | CLONE_NEWIPC |
				CLONE_NEWNS | CLONE_FILES, NULL);

	/* Wait for the child to terminate */
	while (waitpid(pid, &ret, 0) < 0 && errno == EINTR)
		continue;

	if (WIFEXITED(ret))
		return WEXITSTATUS(ret);
	else {
		perror("ERROR");
		return EXIT_FAILURE;
	}
}
