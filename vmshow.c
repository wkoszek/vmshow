/*-
 * Copyright (c) 2007 Wojciech A. Koszek <wkoszek@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/user.h>
#include <sys/sbuf.h>

#include <vm/vm.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>

#include <machine/vmparam.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <kvm.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sysexits.h>
#include <unistd.h>

/*
 * This assert used to work on old 32-bit FreeBSD
 * but fails on FreeBSD 10.1/64-bit.
 */
#define ASSERT_ADDR_ISKERN(a)	do {				\
	break;							\
	assert((((u_long) a) >= VM_MIN_KERNEL_ADDRESS)	&&	\
	 (((u_long) a) <= VM_MAX_KERNEL_ADDRESS));		\
} while (0)

static kvm_t *kmem;

static void
usage(void)
{

	fprintf(stderr, "usage: %s <pid>\n", getprogname());
	exit(EX_USAGE);
}

static void
kmem_read(void *kaddr, void *uaddr, size_t size)
{
	int ret;

	ret = kvm_read(kmem, (u_long) kaddr, uaddr, size);
	assert(ret == size && ("kvm_read() problem"));
}


static int
vm_process(struct kinfo_proc *proc)
{
	struct vm_map_entry entry;
	struct vm_object object;
	struct vmspace vm;
	vm_map_entry_t entryp;
	vm_map_t map;
	vm_object_t objp;
	unsigned long addr;
	struct sbuf *sb;
	int ret;

	assert(proc != NULL && kmem != NULL);
	addr = (unsigned long)proc->ki_vmspace;
	printf("vmspace: 0x%lx\n", addr);
	ASSERT_ADDR_ISKERN(addr);
	ret = kvm_read(kmem, addr, &vm, sizeof(vm));
	assert(ret == sizeof(vm));
        printf("swrss: %lu\n", (u_long)vm.vm_swrss);
        printf("tsize: %lu\n", (u_long)vm.vm_tsize);
        printf("dsize: %lu\n", (u_long)vm.vm_dsize);
	printf("taddr: %lx\n", (u_long)vm.vm_taddr);
	printf("daddr: %lx\n", (u_long)vm.vm_daddr);
	map = &(vm.vm_map);
	printf("nentries: %d\n", map->nentries);

	sb = sbuf_new(NULL, NULL, 1024 * 8, SBUF_AUTOEXTEND);
	if (sb == NULL)
		return (ENOMEM);
	sbuf_clear(sb);
	for (entryp = map->header.next;
	    entryp != &(proc->ki_vmspace->vm_map.header);
	    entryp = entry.next) {
		kmem_read(entryp, &entry, sizeof(entry));
		sbuf_printf(sb, "[0x%lx -> 0x%lx]\t%lx\t", (u_long)entry.start,
		    (u_long)entry.end, (u_long) entryp);	
		if (entry.eflags & MAP_ENTRY_IS_SUB_MAP) {
			sbuf_putc(sb, '\n');
			continue;
		}
		if ((objp = entry.object.vm_object) == NULL) {
			sbuf_putc(sb, '\n');
			continue;
		}
		for (; objp; objp = object.backing_object)
			kmem_read(objp, &object, sizeof(object));
		switch (object.type) {
		case OBJT_VNODE:
			sbuf_printf(sb, "vnode");
			break;
		case OBJT_DEFAULT:
			sbuf_printf(sb, "default");
			break;
		default:
			sbuf_printf(sb, "<unknown>");
			break;
		}
		sbuf_printf(sb, "\n");
	}
	sbuf_finish(sb);
	printf("%s", sbuf_data(sb));
	sbuf_delete(sb);

	return (0);
}

int
main (int argc, char **argv)
{
	char errstr[_POSIX2_LINE_MAX];
	int error;
	struct kinfo_proc *kp;
	int pid, cnt;

	pid = cnt = -1;
	bzero(errstr, sizeof(errstr));
	if (argc < 2)
		usage();
	pid = atoi(argv[1]);
	kmem = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errstr);
	if (kmem == NULL)
		err(EXIT_FAILURE, "kvm_openfiles()");
	printf("trying to obtain %d\n", pid);
	kp = kvm_getprocs(kmem, KERN_PROC_PID, pid, &cnt);
	if (kp == NULL || cnt != 1)
		err(EXIT_FAILURE, "kvm_getprocs()");
	printf("kmin   : 0x%lx, kmax: 0x%lx\n", VM_MIN_KERNEL_ADDRESS,
	    VM_MAX_KERNEL_ADDRESS);
	error = vm_process(kp);
	if (error != 0)
		err(EXIT_FAILURE, "vm_process()");
	error = kvm_close(kmem);
	if (error != 0)
		err(EXIT_FAILURE, "kvm_close()");
	exit(EXIT_SUCCESS);
}
