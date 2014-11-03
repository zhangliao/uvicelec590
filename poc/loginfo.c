#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include </usr/include/git2/odb.h>
#include <git2.h>
#include <stdio.h>
void visit(git_commit *c);

void visit(git_commit *c)
{
	size_t i, num_parents = git_commit_parentcount(c);

	/* Print some stuff about this commit */
	char oidstr[10] = {0};
	git_oid_tostr(oidstr, 9, git_commit_id(c));
	printf("%s\n%s\n\n", oidstr,git_commit_message(c));

	for (i=0; i<num_parents; i++) {
	/* Do the same for the parents */
	git_commit *p;
	if (!git_commit_parent(&p, c, i))
	  visit(p);
	git_commit_free(p);
	}
}

int main (int argc, char** argv)
{   
	git_repository *repo = NULL;
	git_tree_entry *entry = NULL;
	git_tree *tree = NULL;
	static git_tree *g_root_tree;
	git_oid oid1;
	int error;

	if (git_repository_open(&repo,"/home/liaoz/testrep") < 0 )
		printf("Unable to open the repository\n");
	
	git_revwalk *walk;
	if (git_revwalk_new(&walk, repo) < 0)
		printf("git_revwalk_new error\n");
	git_revwalk_sorting(walk,GIT_SORT_TOPOLOGICAL |GIT_SORT_TIME);
    git_revwalk_push_head(walk);
    git_revwalk_hide_glob(walk, "tags/*");
    
    git_object *obj;
    git_revparse_single(&obj, repo, "HEAD~10");
    git_revwalk_hide(walk, git_object_id(obj));
	
	//if (git_tree_entry_bypath(&entry, g_root_tree, ".") < 0)
		//printf("git_tree_entry_bypath error\n");
	
	//if (git_tree_lookup(&tree, repo, git_tree_entry_id(entry)) < 0)
		//printf("git_tree_lookup error\n");
	
	//if (git_object_lookup_bypath(&obj, (git_object*)tree,"testfile", GIT_OBJ_BLOB) < 0)
		//printf("git_object_lookup_bypath error\n");
	
	//git_revparse_single(&obj, repo, "testfile");
	//if (git_odb_hashfile(&oid1, "/home/liaoz/testrep/testfile",GIT_OBJ_BLOB) < 0)
	//	printf("git_odb_hashfile error:%d\n");
	//if (git_object_lookup(&obj, repo, &oid1, GIT_OBJ_BLOB) < 0)
	//	printf("git_object_lookup error\n");
	//if (git_revwalk_push(walk, git_object_id(obj)) < 0)
	//	printf("git_revwalk_push error\n");
    git_object_free(obj);
	
	git_oid oid;
	while (git_revwalk_next(&oid, walk) == 0) {
        git_commit *c;
        char oidstr[40] = {0};
        
        git_commit_lookup(&c, repo, &oid);
        git_oid_tostr(oidstr, 41, &oid);
        printf("%s\n%s\n\n", oidstr,git_commit_message(c));
        
        git_commit_free(c);
	}
	git_repository_free(repo);
}
	
	