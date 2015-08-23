VMshow - Virtual memory statistics tool based on FreeBSD's KVM API
======

`vmshow` is a simple program which shows how to use KVM API for obtaining
system statistics.

It should build out of the box on the FreeBSD system.

# How to build

Just run:

	make

# How to run

Run the program, you must be root, and you must pass the PID to the program
as the 1st argument:

	sudo ./vmshow <pid>

For example:

	./vmshow $$

To be able to observe the output (FreeBSD 10 run inside of
[Vagrant](http://www.vagrantup.com):

	# ./vmshow  $$
	trying to obtain 833
	kmin   : 0xfffffe0000000000, kmax: 0xfffffffffffff000
	vmspace: 0xfffff800021408c0
	swrss: 0
	tsize: 205
	dsize: 13
	taddr: 400000
	daddr: 6cc000
	nentries: 19
	[0x400000 -> 0x4cd000]	fffff800027fa300	vnode
	[0x6cc000 -> 0x6d4000]	fffff800028b7700	vnode
	[0x6d4000 -> 0x6d9000]	fffff80002911780	default
	[0x8006cc000 -> 0x8006e7000]	fffff800028b7680	vnode
	[0x8006e7000 -> 0x80070d000]	fffff800027f9080	default
	[0x8008e7000 -> 0x8008e9000]	fffff80002911500	default
	[0x8008e9000 -> 0x800931000]	fffff80002911380	vnode
	[0x800931000 -> 0x800b31000]	fffff80002911680	default
	[0x800b31000 -> 0x800b36000]	fffff80002911400	vnode
	[0x800b36000 -> 0x800b3f000]	fffff80002911480	vnode
	[0x800b3f000 -> 0x800d3f000]	fffff80002912100	default
	[0x800d3f000 -> 0x800d40000]	fffff80002912500	vnode
	[0x800d40000 -> 0x800eb4000]	fffff800028b7400	vnode
	[0x800eb4000 -> 0x8010b3000]	fffff800028b8000	default
	[0x8010b3000 -> 0x8010c0000]	fffff800027fae00	vnode
	[0x8010c0000 -> 0x8010e9000]	fffff800027f8780	default
	[0x801400000 -> 0x801c00000]	fffff800028b7600	default
	[0x7ffffffdf000 -> 0x7ffffffff000]	fffff800028b7500	default
	[0x7ffffffff000 -> 0x800000000000]	fffff8000255fc80	<unknown>

(`$$` means "current PID" and gets replaced with the numeric value of the
PID by the shell).

# Last tested on:

	FreeBSD vagrant-freebsd-101-amd64 10.1-RELEASE FreeBSD 10.1-RELEASE #0
	r274401: Tue Nov 11 21:02:49 UTC 2014
	root@releng1.nyi.freebsd.org:/usr/obj/usr/src/sys/GENERIC  amd64
