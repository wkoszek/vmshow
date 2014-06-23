PROG=		vmshow
LDADD+=		-lkvm -lsbuf
CFLAGS+=	-g -ggdb -Wall -pedantic -gdwarf-2
STRIP	=	
NO_MAN=

.include <bsd.prog.mk>
