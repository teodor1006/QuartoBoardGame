#ifndef config
#define config

struct configuration {
	char hostname[31];
	char portnumber[5];
	char gamekind[16];
};

struct configuration read_conf_file(char *filename);

#endif