#include "taskparams.h"
#include "exitit.h"
#include "cdfpred.h"
#include "cdfstat.h"
#include "cfamily.h"

#include <string.h>
#include <stdlib.h>


char *synmsg=	"\n" PROGNAME
				"\nsfalic c|d infile outfile [switches]"
				"\n       c - compress"      
				"\n       d - decompress"      
				"\n       image format: PGM P5"      
				"\n       switches:"      
				"\n         -echo      -- echo processed cmdline and input file header"      
				"\n         -noheader  -- skip writting output file header (for testing only)"      
				"\n         -generic8bpp -- (de)compress 8bpp imgs using generic 16bpp routines"      
				"\n                       (slower, for testing only)"      
				"\n       compression switches:"      
				"\n         -decoronly -- decorrelate only (skip stat. compression)"      
				"\n         -pred pred -- pred number: " STR(MINpred) " to " STR(MAXpred) ", def.: " STR(DEFpred) 
				"\n                       -1: raw binary copy "
				"\n                       0-7: Loslles JPEG predictors,"
				"\n                       8: (3A+3B-2C)/4"
				"\n         -evol evol -- model structure: " STR(MINevol) " to " STR(MAXevol) ", def.: " STR(DEFevol) 
				"\n                       1: consecutive bucket sizes: 1 1 1 2 2 4 4 8 8 ..."
				"\n                       3: 1 2 4 8 16 32 64 ..."
				"\n                       5: 1 4 16 64 256 1024 4096 16384 65536"
				"\n                       0, 2, 4: obsolete"
				"\n         -trigger trigger -- bucket trigger: " STR(MINtrigger) " to " STR(MAXtrigger) ", def.: " STR(DEFtrigger) 
				"\n                       threshold for halving bucket counters"
				"\n                       must be an integer multiple of 10"
				"\n                       0: auto select trigger for evol and wait mask"
				"\n         -maxclen maxclen -- codeword length limit: img. depth+1 to " STR(MAXmaxclen) ", def.: " STR(DEFmaxclen) 
				"\n         -wm wmistart wmimax wminext -- model update frequency controll " 
				"\n                 wmistart -- starting frequency index: " STR(MINwmistart) " to wmimax, def.: " STR(DEFwmistart) 
				"\n                       0: 100%, 1: 66%, 2: 40%, 3: 22.2%, 4: 11.8%,"
				"\n                       5: 6.06%, 6: 3.08%, 7: 1.55%, 8: 0.778%, ..."
				"\n                 wmimax -- target frequency index: wmistart to " STR(MAXwmimax) ", def.: " STR(DEFwmimax) 
				"\n                 wminext -- number of pixels encoded before increasing" 
				"\n                       frequency index: " STR(MINwminext) " to " STR(MAXwminext) ", def.: " STR(DEFwminext) 
				"\n                       must be an integer power of 2" 
				;



void processcmdline(int argc, char **argv)
{
	argv++;
	if (!*argv)
		exitit(synmsg, -1);

	if ( !strcmp(*argv, "c") )
	{
		argv++;
		f_compress=1;
	}
	else if ( !strcmp(*argv, "d") )
	{
		argv++;
		f_compress=0;
	}
	else
		exitit(synmsg, -1);

	if (!*argv)
		exitit("no infile", 1);

	infilename=*argv;
		argv++;

	if (!*argv)
		exitit("no outfile", 1);

	outfilename=*argv;
		argv++;

	while (*argv)
	{
		if ( !strcmp(*argv, "-echo") ) 
			{
				echo=1;
				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-noheader") ) 
			{
				noheader=1;
				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-decoronly") ) 
			{
				decoronly=1;
				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-generic8bpp") ) 
			{
				generic8bpp=1;
				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-pred") ) 
			{
				argv++;
				if (!f_compress)
					exitit("-pred in forbidden for decompression", 1);

				if(!*argv)
					exitit("-pred requires argument", 1);

				pred=atoi(*argv);
				if ( pred<MINpred || pred>MAXpred )
					exitit("-pred arg out of range", 1);

				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-evol") ) 
			{
				argv++;
				if (!f_compress)
					exitit("-evol in forbidden for decompression", 1);

				if(!*argv || **argv=='-')
					exitit("-evol requires argument", 1);

				evol=atoi(*argv);
				if ( evol<MINevol || evol>MAXevol )
					exitit("-evol arg out of range", 1);

				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-trigger") ) 
			{
				argv++;
				if (!f_compress)
					exitit("-trigger in forbidden for decompression", 1);

				if(!*argv || **argv=='-')
					exitit("-trigger requires argument", 1);

				trigger=atoi(*argv);
				if ( trigger<MINtrigger || trigger>MAXtrigger )
					exitit("-trigger arg out of range", 1);

				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-maxclen") ) 
			{
				argv++;
				if (!f_compress)
					exitit("-maxclen in forbidden for decompression", 1);

				if(!*argv || **argv=='-')
					exitit("-maxclen requires argument", 1);

				maxclen=atoi(*argv);
				if ( maxclen<0 || maxclen>MAXmaxclen )
					exitit("-maxclen arg out of range", 1);

				argv++;
				continue;
			}

		if ( !strcmp(*argv, "-wm") ) 
			{
				argv++;
				if (!f_compress)
					exitit("-wm in forbidden for decompression", 1);

				if(!*argv || **argv=='-')
					exitit("-wm requires argument (s)", 1);
				wmistart=atoi(*argv);
				if ( wmistart<MINwmistart || wmistart>MAXwmimax )
					exitit("-wmistart arg out of range", 1);
				argv++;

				if(!*argv || **argv=='-')
					exitit("-wm requires argument (s)", 1);
				wmimax=atoi(*argv);
				if ( wmimax<wmistart || wmimax>MAXwmimax )
					exitit("-wmimax arg out of range", 1);
				argv++;

				if(!*argv || **argv=='-')
					exitit("-wm requires argument (s)", 1);
				wminext=atoi(*argv);
				if ( wminext<MINwminext || wminext>MAXwminext )
					exitit("-wminext arg out of range", 1);
				argv++;

				continue;
			}

		exitit("commandline syntax error", 1);
	}
}
