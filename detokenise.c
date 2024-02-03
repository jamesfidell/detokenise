#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

const char*			keywords[] = {
	"AND","DIV","EOR","MOD","OR","ERROR","LINE","OFF","STEP","SPC","TAB(",
	"ELSE","THEN","","OPENIN","PTR","PAGE","TIME","LOMEM","HIMEM","ABS","ACS",
	"ADVAL","ASC","ASN","ATN","BGET","COS","COUNT","DEG","ERL","ERR","EVAL",
	"EXP","EXT","FALSE","FN","GET","INKEY","INSTR(","INT","LEN","LN","LOG",
	"NOT","OPENUP","OPENOUT","PI","POINT(","POS","RAD","RND","SGN","SIN","SQR",
	"TAN","TO","TRUE","USR","VAL","VPOS","CHR$","GET$","INKEY$","LEFT$(",
	"MID$(","RIGHT$(","STR$","STRING$(","EOF","SUM","WHILE","CASE","WHEN","OF",
	"ENDCASE","OTHERWISE","ENDIF","ENDWHILE","PTR","PAGE","TIME","LOMEM",
	"HIMEM","SOUND","BPUT","CALL","CHAIN","CLEAR","CLOSE","CLG","CLS","DATA",
	"DEF","DIM","DRAW","END","ENDPROC","ENVELOPE","FOR","GOSUB","GOTO","GCOL",
	"IF","INPUT","LET","LOCAL","MODE","MOVE","NEXT","ON","VDU","PLOT","PRINT",
	"PROC","READ","REM","REPEAT","REPORT","RESTORE","RETURN","RUN","STOP",
	"COLOUR","TRACE","UNTIL","WIDTH","OSCLI","<undef>","CIRCLE","ELLIPSE",
	"FILL","MOUSE","ORIGIN","QUIT","RECTANGLE","SWAP","SYS","TINT","WAIT",
	"INSTALL","","PRIVATE","BY","EXIT"
};

void		detokenise ( const unsigned char* );
void		usage ( void );
int			processLine ( const unsigned char*, int );

int
main ( int argc, const char** argv )
{
	const char*			file;
	struct stat			sb;
	unsigned char*	buffer;
	int							fd;

	if ( argc != 2 ) {
		usage();
		return -1;
	}

	file = argv[1];
	if ( stat ( file, &sb ) < 0 ) {
		fprintf ( stderr, "Unable to stat %s, errno = %d (%s)\n", file, errno,
				strerror ( errno ));
		return -1;
	}

	if (( buffer = malloc (( size_t ) sb.st_size )) == 0 ) {
		fprintf ( stderr, "malloc failed for %ld bytes\n", sb.st_size );
	}

	if (( fd = open ( file, O_RDONLY )) < 0 ) {
		fprintf ( stderr, "Unable to open %s, errno = %d (%s)\n", file, errno,
				strerror ( errno ));
		return -1;
	}

	if ( read ( fd, buffer, ( size_t ) sb.st_size ) != ( size_t ) sb.st_size ) {
		fprintf ( stderr, "Unable to read file %s, errno = %d (%s)\n", file, errno,
				strerror ( errno ));
		return -1;
	}

	detokenise ( buffer );

	return 0;
}


void
usage ( void )
{
	fprintf ( stderr, "Usage\n" );
}


void
detokenise ( const unsigned char* code )
{
	const unsigned char*	p = code;
	int										linenum;
	int										linelen;
	int										eop = 0;

	if ( 13 == *p++ ) {
		do {
			linenum = ( *p++ << 8 ) + *p++;
			linelen = *p++;
			printf ( "%5d ", linenum );
			if ( processLine ( p, linelen - 4 ) < 0 ) {
				return;
			}
			p = p + linelen - 3;
			if ( 13 == *p && 0xff == 0xff ) {
				eop = 1;
			}
		} while ( !eop );
	}
}


int
processLine ( const unsigned char* p, int len )
{
	int							i, lo, hi;
	unsigned char		c, n;

	for ( i = 0; i < len; i++ ) {
		c = *p++;
		if ( c < 32 || 127 == c ) {
			printf ( "0x%02x ", c );
		} else if ( c < 127 ) {
			printf ( "%c", c );
		} else if ( c == 0x8d ) { // GOTO + encoded line number
			n = *p++;
			lo = (( n << 2 ) & 0xc0 ) ^ ( *p++ );
			hi = (( n << 4 ) & 0xc0 ) ^ ( *p++ );
			printf ( "%d", ( hi << 8 ) + lo );
			i += 3;
		} else {
			printf ( "%s ", keywords[ c ^ 0x80 ]);
		}
	}
	printf ( "\n" );
	if ( *p != 13 ) {
		fprintf ( stderr, "EOL not found ( %d )\n", *p );
		return -1;
	}
	return 0;
}
