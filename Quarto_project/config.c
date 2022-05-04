#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "config.h"


struct configuration conf;


struct configuration read_conf_file(char *filename) {

	int chr, i = 0;
	char **lines;

	FILE *config_file = fopen(filename, "r");
	if (config_file == NULL) {
		perror("file open");
		exit(EXIT_FAILURE);
	} else printf("Configuration file was opened successfully\n\n");

	if ((fseek(config_file, 0L, SEEK_END)) < 0) {
		perror("config.c fseek error");
		exit(EXIT_FAILURE);
	}

	long int len;
	if ((len = ftell(config_file)) < 0) {
		perror("config.c ftell error");
		exit(EXIT_FAILURE);
	}
	rewind(config_file);

	char str_file[len];
	int nb_lines = 0;

	while ((chr = fgetc(config_file)) != EOF) {
		if (isblank(chr) == 0) {
			str_file[i] = chr;
			i++;
		}
		if (chr == '\n') nb_lines++;
	}
	str_file[i] = '\0';

	if ((lines = malloc(sizeof(char *) * nb_lines)) == NULL) {
		perror("line 47 config.c:: malloc error");
		exit(EXIT_FAILURE);
	}
	char *token = strtok(str_file, "\n");
	i = 0;

	while (token) {
		*(lines + i) = token;
		token = strtok(NULL, "\n");
		i++;
	}

	char *value;

	for (i = 0; i < nb_lines; i++) {
		token = strtok(*(lines + i), "=");
		value = strtok(NULL, "=");
		if (strcmp(token, "hostname") == 0) {
			strcpy(conf.hostname, value);
		}
		else if (strcmp(token, "portnumber") == 0) {
			strcpy(conf.portnumber, value);
		}
		else if (strcmp(token, "gamekind") == 0) {
			strcpy(conf.gamekind, value);
		}
		else {
			printf("Error trying to read the configuration file:\n");
			printf("***More parameters than expected or parameters misspelled\n");
			exit(EXIT_FAILURE);
		}
	}

	free(lines);

	if (fclose(config_file) == EOF) {
		perror("line 83 config.c:: error closing the file");
		exit(EXIT_FAILURE);
	}

	return conf;
}
