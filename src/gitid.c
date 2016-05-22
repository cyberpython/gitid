/*
 ===============================================================================
 Name        : gitid.c
 Author      : Georgios Migdos
 License     : The MIT License (MIT)

               Copyright (c) 2016 Georgios Migdos

               Permission is hereby granted, free of charge, to any person
               obtaining a copy of this software and associated documentation
               files (the "Software"), to deal in the Software without
               restriction, including without limitation the rights to use,
               copy, modify, merge, publish, distribute, sublicense, and/or sell
               copies of the Software, and to permit persons to whom the
               Software is furnished to do so, subject to the following
               conditions:

               The above copyright notice and this permission notice shall be
               included in all copies or substantial portions of the Software.

               THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
               EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
               OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
               NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
               HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
               WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
               FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
               OTHER DEALINGS IN THE SOFTWARE.

 Description : This program will recursively search the current directory
               (which should be part of a Git repository), for files with one of
               the following extensions:

               .adb, .ads, .c, .h, .cpp, .hpp, .cc, .java, .py, .js, .cs, .rc,
               .xml, .html, .htm, .xhtml

               If a file's extension matches one of the above, then it is opened
               and all occurences of the following keywords are are replaced:

               $Id$

               $Name$

               The search is performed CVS-style (i.e. the whole string until
               the second '$' character is replaced).

               - $Id$ is replaced with $Id: <GIT_COMMIT_ID> $

               - $Name$ is replaced with $Name: <GIT_TAG_NAME> $

               - If no tag exists for the current commit, then $Name$ is
                 replaced with $Name: WORKING $

              Note that the each file is read into memory and then rewritten to
              replace the markers. This is performed for each marker (=2) thus
              it may be very slow if you have many large files.
 ===============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#include "git_helper.h"

#define PATH_MAX_LEN 4096

#define REPLACE_RESULT_OK 0
#define REPLACE_RESULT_OPEN_FILE_ERROR 1
#define REPLACE_RESULT_STAT_FILE_ERROR 2
#define REPLACE_RESULT_ALLOC_MEM_ERROR 3
#define REPLACE_RESULT_READ_FILE_ERROR 4

#define NO_TAG_TEXT "WORKING"
#define UNKNOWN_COMMIT_ID_TEXT "<unknown>"

typedef int bool;
#define true 1
#define false 0

#ifdef _WIN32

#define PATH_SEPARATOR_CHAR "\\"

#else

#define PATH_SEPARATOR_CHAR "/"

#endif

static const char * source_file_exts[] = { ".adb", ".ads", ".c", ".h", ".cpp",
		".hpp", ".cc", ".java", ".py", ".js", ".cs", ".rc", ".xml", ".html",
		".htm", ".xhtml" };


char * read_file(char *filepath){
	char *buf = NULL;
	size_t size = 0;
	FILE *f = fopen(filepath, "r");
	if(f == NULL){
		return NULL;
	}

#ifdef __WIN32
	size = _filelength(f->_file);
#else
	fseeko(f, 0 , SEEK_END);
	size = ftello(f);
	fseeko(f, 0, SEEK_SET);
#endif

	if(!(buf = malloc(size + 1))){
		return NULL;
	}
	memset(buf, 0, size + 1);
	fread(buf, 1, size, f);
	fclose(f);
	return buf;
}

int find_pattern(const char *str, size_t len, int start_at, const char *pattern,
		size_t pattern_length) {
	int i = start_at;
	int j = 0;
	size_t max = len - pattern_length;
	while (i < max) {
		for (j = 0; j < pattern_length; j++)
		{
			if (str[i+j] != pattern[j])
				break;
		}

		if (j == pattern_length)
		{
			return i;
		}

		i++;
	}
	return -1;
}

int find_char(const char *str, size_t len, int start_at, char c) {
	int i = start_at;
	while (i < len) {
		if (str[i] == c) {
			return i;
		}
		i++;
	}
	return -1;
}

void cleanup_after_replace(FILE *file, char *buf, char *pattern) {
	if (file) {
		fflush(file);
		fclose(file);
	}
	if (buf) {
		free(buf);
	}
	if (pattern) {
		free(pattern);
	}
}

int replace_keyword_in_file(const char *keyword, const char *replacement,
		const char *filepath, char delimeter) {

	char *buf = NULL;
	char *pattern = NULL;
	char end_str[3] = { ' ', delimeter, 0 };
	int read_from = 0;
	int stop_at = -1;
	int next_delimeter = -1;
	size_t size = 0;
	size_t keyword_len = strlen(keyword);
	size_t replacement_len = strlen(replacement);
	size_t pattern_len = keyword_len + 2;
	FILE *f = NULL;

	pattern = malloc(pattern_len);
	pattern[pattern_len-1] = 0;
	snprintf(pattern, pattern_len, "%c%s", delimeter, keyword);
	pattern_len--;

	buf = read_file((char*)filepath);
	if(!buf){
		cleanup_after_replace(f, buf, pattern);
		return REPLACE_RESULT_READ_FILE_ERROR;
	}
	size = strlen(buf);
	if(size == 0){
		cleanup_after_replace(f, buf, pattern);
		return REPLACE_RESULT_READ_FILE_ERROR;
	}

	f = fopen(filepath, "wb+");

	if(!f){
		cleanup_after_replace(f, buf, pattern);
		return REPLACE_RESULT_OPEN_FILE_ERROR;
	}

	stop_at = find_pattern(buf, size, 0, pattern, pattern_len);

	if (stop_at != -1) {

		do {

			fwrite(buf + read_from, 1, stop_at - read_from, f);
			fwrite(pattern, 1, pattern_len, f);
			fwrite(": ", 1, 2, f);
			fwrite(replacement, 1, replacement_len, f);
			fwrite(end_str, 1, 2, f);
			fflush(f);

			if(stop_at == size){
				printf("SHOULD STOP.\n");
				next_delimeter = -1;
			}else{
				next_delimeter = find_char(buf, size, stop_at + 1, delimeter);
			}

			if (next_delimeter != -1) {
				read_from = next_delimeter + 1;
				stop_at = find_pattern(buf, size, read_from, pattern, pattern_len);
			} else {
				read_from = stop_at + pattern_len;
				stop_at = -1;
			}

		} while (stop_at != -1);
	}

	if (read_from < size)
		fwrite(buf+read_from, 1, size - read_from, f);

	cleanup_after_replace(f, buf, pattern);
	return REPLACE_RESULT_OK;
}

bool is_candidate_file(const char *filepath) {
	char *dot = strrchr(filepath, '.');
	int i;
	for (i = 0; i < sizeof(source_file_exts) / sizeof(char*); i++) {
		if (dot && !strcmp(dot, source_file_exts[i])) {
			return true;
		}
	}

	return false;
}

void process_files_recursively(const char *dirpath, const char* pattern,
		const char* replace_with) {
	DIR *dir;
	struct dirent *entry;

	if (!(dir = opendir(dirpath)))
		return;

	if (!(entry = readdir(dir)))
		return;

	do {
		char path[PATH_MAX_LEN];
		int len = snprintf(path, sizeof(path) - 1, "%s%s%s", dirpath,
		PATH_SEPARATOR_CHAR, entry->d_name);
		path[len] = 0;
		if (entry->d_type == DT_DIR) {
			if (strcmp(entry->d_name, ".") == 0
					|| strcmp(entry->d_name, "..") == 0)
				continue;
			process_files_recursively(path, pattern, replace_with);
		} else {
			if (is_candidate_file(path)) {
				replace_keyword_in_file(pattern, replace_with, path, '$');
			}
		}
	} while ((entry = readdir(dir)));
	closedir(dir);
}

int main(void) {

	char *relpath = ".";
	char abspath[PATH_MAX_LEN];
	char *tag_name = NULL;
	char *commit_id = NULL;

	git_helper_initialize();

#ifdef _WIN32

	_fullpath(abspath, relpath, PATH_MAX_LEN);

#else

	realpath(relpath, abspath);

#endif


	get_git_tag_name(&tag_name);
	get_git_commit_id(&commit_id);

	if(tag_name == NULL){
		tag_name = malloc(strlen(NO_TAG_TEXT));
		strcpy(tag_name, NO_TAG_TEXT);
	}

	if(commit_id == NULL){
		commit_id = malloc(strlen(UNKNOWN_COMMIT_ID_TEXT));
		strcpy(commit_id, UNKNOWN_COMMIT_ID_TEXT);
	}

	fflush(stdout);

	process_files_recursively(abspath, "Name", tag_name);
	process_files_recursively(abspath, "Id", commit_id);

	if(commit_id)
	{
		free(commit_id);
	}

	if(tag_name)
	{
		free(tag_name);
	}

	git_helper_finalize();




	return 0;
}

