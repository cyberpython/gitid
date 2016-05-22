/*
 ===============================================================================
 Name        : git_helper.c
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

 Description : Utility functions to access libgit2.
 ===============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <git2.h>

void git_helper_initialize(void)
{
	git_libgit2_init();
}

void git_helper_finalize(void)
{
	git_libgit2_shutdown();
}

int get_git_tag_name(char** tag_name) {
	git_repository *repo = NULL;

	git_describe_options describe_options;
	git_describe_format_options format_options;

	git_object *commit = NULL;
	git_describe_result *describe_result = NULL;
	git_buf buf = { 0 };

	int error = git_repository_open_ext(&repo, ".", 0, NULL);

	int result = 0;

	*tag_name = NULL;

	if (!error) {

		git_describe_init_options(&describe_options,
				GIT_DESCRIBE_OPTIONS_VERSION);
		git_describe_init_format_options(&format_options,
				GIT_DESCRIBE_FORMAT_OPTIONS_VERSION);
		describe_options.max_candidates_tags = 0;
		format_options.abbreviated_size = 0;

		error = git_revparse_single(&commit, repo, "HEAD");

		if (!error) {

			error = git_describe_commit(&describe_result, commit,
					&describe_options);

			if (!error) {

				error = git_describe_format(&buf, describe_result,
						&format_options);

				if (!error) {
					size_t len = strlen(buf.ptr)+1;
					*tag_name = malloc(len);
					memset(*tag_name, 0, len);
					strcpy(*tag_name, buf.ptr);
					result = 1;
				}

			}

		}
		if (commit) {
			git_object_free(commit);
		}

	}

	if (repo) {
		git_repository_free(repo);
	}

	return result;
}

int get_git_commit_id(char** commit_id) {
	git_repository *repo = NULL;

	git_object *commit = NULL;

	git_oid *oid = NULL;

	int error = git_repository_open_ext(&repo, ".", 0, NULL);

	int result = 0;

	*commit_id = NULL;

	if (!error) {

		error = git_revparse_single(&commit, repo, "HEAD");

		if (!error) {

			oid = git_object_id(commit);

			if (oid) {

				char* commit_id_str = git_oid_tostr_s(oid);

				if (commit_id_str) {
					size_t len = strlen(commit_id_str)+1;
					*commit_id = malloc(len);
					memset(*commit_id, 0, len);
					strcpy(*commit_id, commit_id_str);
					result = 1;
				}

			}

		}

		if (commit) {
			git_object_free(commit);
		}

	}

	if (repo) {
		git_repository_free(repo);
	}

	return result;
}

