/*
 * Copyright 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License version 3, as published 
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranties of 
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along 
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    char *const envp[7] = {
        "PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 
        "SHELL=/bin/bash", 
        "HOME=/root",
        "USER=root",
        "USERNAME=root",
        "LOGNAME=root", 
        NULL
    };

    /* Exec shell */
    printf("Calling UBUNTU init...\n");
    execle("/sbin/init", "/sbin/init", "--verbose", NULL, envp);

    return 0;
}

int main() {
    pid_t pid;
    int ret;
    long stack_size = sysconf(_SC_PAGESIZE);
    void *stack = alloca(stack_size) + stack_size;

    printf("About to call clone()\n");
    /* New process with its own PID/IPC/NS namespace */
    pid = clone(ubuntum, stack, SIGCHLD | CLONE_NEWPID | CLONE_NEWIPC |
        CLONE_NEWNS, NULL);

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
