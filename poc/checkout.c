#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h>
static void checkout(git_repository *repo, char filename[], char commit_sha[]);

static void checkout(git_repository *repo, char filename[], char commit_sha[])
{
	git_checkout_opts opts = GIT_CHECKOUT_OPTS_INIT;
	char *paths[] = {filename};
	opts.paths.strings = paths;
	opts.paths.count = 1;
	opts.checkout_strategy = GIT_CHECKOUT_FORCE ;
	git_commit *commit;
	git_oid oid;
	
	// Get "Commit ID"
	if (git_oid_fromstr(&oid, commit_sha) < 0)
		printf("Could not find the commit object\n");
	if (git_commit_lookup(&commit, repo, &oid) < 0) 
		printf("Could not lookup the commit id\n");

	// Do the checkout
	if (git_checkout_tree(repo, (git_object*)commit, &opts) < 0)
		printf("Checkout faild\n");

	// Clean up
	git_commit_free(commit);
}

int main (int argc, char** argv)
{   
	git_repository *repo = NULL;
	
	if (git_repository_open(&repo,"/home/liaoz/testrep") < 0 )
		printf("Unable to open the repository\n");
	
	checkout(repo,argv[1],argv[2]);
}
	
	