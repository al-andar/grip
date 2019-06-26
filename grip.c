// grip.c
// Search files for lines containing IP addresses from specified networks.
// 2019-06 Bernhard Griener
//
#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


int ip_in_netlist (unsigned long ip, unsigned long* nets, unsigned long* masks, int nnets);
void usage();

int main (int nargs, char** arg) {
	

	int opt_reverse=0;
	int maxnets=100;
	int nnets=0;
	unsigned long* nets;
	unsigned long* masks;

	int maxfiles=1024;
	int nfiles=0;
	char** files;

	int linebufferlen=8192;
	char* line=malloc(linebufferlen);

	nets=calloc(maxnets,sizeof(unsigned long));
	masks=calloc(maxnets,sizeof(unsigned long));
	files=calloc(maxfiles, sizeof(char*));

	if (nets==NULL || masks==NULL|| line==NULL||files==NULL) {
		fprintf(stderr,"Out of memory\n");
		exit(1);
	}

	//----------------------------------------------------------------------
	// parse args
	//----------------------------------------------------------------------
	int endof_options=0;
	for (int i=1; i<nargs; i++) {
		int b1,b2,b3,b4,pfl;

		// known option?
		if (strncmp("-h", arg[i],2)==0) {
			usage();
			exit (1);
		}
		if (strncmp("-v", arg[i],2)==0) {
			opt_reverse=1;
			continue;
		}
		// unknown option?
		if (arg[i][0]=='-' && arg[i][1]) {
			fprintf(stderr, "Unknown option %s\n",arg[i]);
			exit (1);
		}
		// pattern?
		if (sscanf(arg[i], "%3d.%3d.%3d.%3d/%2d", &b1,&b2,&b3,&b4,&pfl)==5) {
			if (nnets+1>=maxnets) {
				fprintf(stderr, "Error: Supports only %d network patterns\n", maxnets);
				exit (1);
			} 
			nets[nnets]=(b1<<24)|(b2<<16)|(b3<<8)|b4;
			masks[nnets]=  (0xffffffffL<<(32-pfl))&0xffffffff;

			if ((nets[nnets] & masks[nnets]) != nets[nnets]) {
				fprintf(stderr, "Network does not match prefix length: %s\n", arg[i]);
				exit(1);
			}
			nnets++;
			continue;
		}
		// file name!
		if (nfiles+1>=maxfiles) {
			fprintf(stderr, "Error: Supports only %d files\n", maxfiles);
			exit(1);
		}
		files[nfiles]=arg[i];
		nfiles++;
	}

	if (nnets==0) {
		usage();
		exit(1);
	}


	if (nfiles==0) {
		// dirty hack. signal to read from stdin
		nfiles=1;
		files[0]=NULL;
	}
	// fprintf(stderr, "%lx %lx\n", nets[0], masks[0]);

	//----------------------------------------------------------------------
	// grip files
	//----------------------------------------------------------------------
	
	for (int fn=0; fn<nfiles; fn++) {
		FILE* file;
		if (files[fn]==NULL) {
			file=stdin;
		} else {
			file=fopen(files[fn], "r");
			if (file==NULL) {
				fprintf(stderr, "Error opening file '%s': %s\n", files[fn], strerror(errno));
			
				continue;
			}
		}
		// fprintf(stderr, "%s:\n", files[fn]);
		while(!feof(file)) {
			int line_matches=0;	// does this line match one of the networks?

			getline(&line, &linebufferlen, file);
			// find IP addresses in line
			char* pos=line;
			for (pos=line; *pos; pos++) {
				int nfields=0;
				int nbytesread=0;
				int b1, b2, b3, b4;
				unsigned long thisip;
				if (*pos<'0' || *pos>'9') continue;
				nfields=sscanf(pos, "%3d.%3d.%3d.%3d%n", &b1, &b2, &b3, &b4, &nbytesread);
				if (nfields<4) continue;
				thisip= (b1<<24)|(b2<<16)|(b3<<8)|b4;
				if (ip_in_netlist(thisip, nets, masks, nnets)) {
					line_matches=1;	
					break;
				}
				// advance scan position past current number
				pos++;
				while(*pos>='0'&&*pos<='9') pos++;
			}
			// print line if appropriate
			if ((opt_reverse && !line_matches ) || (!opt_reverse && line_matches)) {
					fputs(line, stdout);
			}
		}
		if (file != stdin) fclose(file);
	}
}

			


int ip_in_netlist (unsigned long ip, unsigned long* nets, unsigned long* masks, int nnets) {
	// fprintf(stderr, "DEBUG: ip_In_netlist:  ip=%lx  nnets=%d  nets[0]= %lx %lx\n", ip, nnets, nets[0], masks[0]);
	for (int i=0; i<nnets; i++) {
		if ((ip & masks[i])==(nets[i] & masks[i])) return 1;
	}
	return 0;
}

			


void usage () {
	fprintf(stderr, "Usage: grip [-v] a1.b1.c1.d1/p1  [ a2.b2.c2.d2/p2 ...]  [ file1  ... ]\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "GRep IP addresses - \n");
	fprintf(stderr, "       Prints lines in files that contain an IP address inside\n");
	fprintf(stderr, "       at least one of the networks specified.\n");
	fprintf(stderr, "       When no files are given, STDIN is used instead.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "        -v: inverse search: print lines not containing such IP addresses\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Example:\n");
	fprintf(stderr, "	grip 10.20.0.0/16 172.25.11.32/27 logfile\n");
	fprintf(stderr, "\n");
}

	
