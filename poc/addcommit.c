#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h>
static void add_and_commit(git_repository *repo);

static void add_and_commit(git_repository *repo)
{
	git_signature *sig;
	git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;
	const git_commit* parents  = NULL;
	git_strarray paths = { NULL, 0 };
	char *strs[1];
	
	strs[0] = "*";
	paths.strings = strs;
	paths.count   = 1;

	
	if (git_signature_now(&sig, "zhangliao", "zhangliao1212@hotmail.com") < 0)
		printf("Unable to signature\n");
	
	if (git_repository_index(&index, repo) < 0)
		printf("Could not open repository index\n");
	
	//if (git_index_add_bypath(index, "bbb.c") < 0)
	//	printf("Could not add file to index\n");
	
	if (git_index_add_all(index, &paths, 0, NULL, NULL) < 0)
		printf("Could not add all file to index\n");
	
	if (git_index_write_tree(&tree_id, index) < 0)
		printf("Unable to write initial tree from index\n");
	
	git_index_free(index);
	
	if (git_tree_lookup(&tree, repo, &tree_id) < 0)
		printf("Could not look up initial tree\n");	

    if (git_revparse_single( (git_object**) &parents, repo, "HEAD" ) <0)
		printf("Could not find the parents\n");
	
	if (git_commit_create(&commit_id, repo, "HEAD", sig, sig,NULL, "NEW COMMIT", tree, 1 ,&parents) < 0)
		printf("Could not do a new commit\n");
	
	git_tree_free(tree);
	git_signature_free(sig);
}

int main (int argc, char** argv)
{   
	git_repository *repo = NULL;
	
	if (git_repository_open(&repo,"/home/liaoz/testrep") < 0 )
		printf("Unable to open the repository\n");
	
	add_and_commit(repo);
}
	
	