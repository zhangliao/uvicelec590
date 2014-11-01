#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h>
static void create_initial_commit(git_repository *repo);

static void create_initial_commit(git_repository *repo)
{
	git_signature *sig;
	git_index *index;
	git_oid tree_id, commit_id;
	git_tree *tree;
	
	if (git_signature_now(&sig, "zhangliao", "zhangliao1212@hotmail.com") < 0)
		printf("Unable to signature\n");
	if (git_repository_index(&index, repo) < 0)
		printf("Could not open repository index\n");
	if (git_index_write_tree(&tree_id, index) < 0)
		printf("Unable to write initial tree from index\n");
	
	git_index_free(index);
	
	if (git_tree_lookup(&tree, repo, &tree_id) < 0)
		printf("Could not look up initial tree\n");
	if (git_commit_create_v(&commit_id, repo, "HEAD", sig, sig,NULL, "Initial commit", tree, 0) < 0)
		printf("Could not create the initial commit\n");
	
	git_tree_free(tree);
	git_signature_free(sig);
}

int main (int argc, char** argv)
{   
	git_repository *repo = NULL;
	git_config *cfg = NULL;
		if (git_repository_init(&repo, "/home/liaoz/testrep", 0) < 0)
		printf("Repository initialize failed\n");
	
	git_repository_free(repo);
	
	if (git_repository_open(&repo,"/home/liaoz/testrep") < 0 )
		printf("Unable to open the repository\n");
	
	if (git_repository_config(&cfg, repo) < 0)
		printf("Unable to open the config\n");
	
	git_config_set_string(cfg, "user.name", "zhangliao");
	git_config_set_string(cfg, "user.email", "zhangliao1212@hotmail.com");
	git_config_free(cfg);
	
	create_initial_commit(repo);
    printf("Created empty initial commit\n");
}
	
	