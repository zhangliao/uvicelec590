#include </usr/include/git2/common.h>
#include </usr/include/git2/errors.h>
#include <git2.h>
#include <stdio.h>



int main (int argc, char** argv)
{   
	git_repository *repo = NULL;
	git_config *cfg = NULL;
	git_index *index = NULL;
	git_signature *me = NULL;
	int error;
	git_oid oid;
	git_oid parent_id;
	git_oid oid_tree;  
	git_tree *tree = NULL;
	git_treebuilder *tree_bld;
	git_commit *commit;
	git_commit *parent;

	//git_repository_init(&repo, "/home/liaoz/testrep", 0);//0:bare false
	//git_repository_free(repo);
	
	
	error = git_repository_open(&repo,"/home/liaoz/testrep");
	if (error < 0) {
		repo = NULL;
        if (giterr_last() == NULL)
            fprintf(stderr, "Error without message.\n");
        else
            fprintf(stderr, "Bad news:\n %s\n", giterr_last()->message);
	}
	else {
		printf("Repository open sucessfully!\n");
	}
	
	error = git_repository_config(&cfg, repo);
	printf("config error is:%d\n",error);
	git_config_set_string(cfg, "user.name", "zhangliao");
	git_config_set_string(cfg, "user.email", "zhangliao1212@hotmail.com");
	git_config_free(cfg);
	
	error = git_repository_index(&index, repo);
	printf("index error is:%d\n",error);
	error = git_index_add_bypath(index, "filestore/aaa.c");
	printf("add error is:%d\n",error);
	
	git_index_write(index);
	//git_index_free(index);
	

	error = git_signature_now(&me, "zhangliao", "zhangliao1212@hotmail.com");
	
	
	

	//error = git_treebuilder_create( &tree_bld, NULL );
	//printf("treebuilder_create error is:%d\n",error);
	//error = git_treebuilder_write(&oid_tree, repo, tree_bld );
	//error = git_index_write_tree_to(&oid_tree, index, repo);
	//printf("git_index_write_tree_to error is:%d\n",error);
	//printf("treebuilder_write error is:%d\n",error);
	//error = git_tree_lookup(&tree, repo, &oid_tree );
	//printf("git_tree_lookup error is:%d\n",error);

	//git_oid_fromstr(&oid, "master");
	//error = git_commit_lookup(&commit, repo, &oid);
	//printf("commit error is:%d\n",error);
	//error = git_tree_lookup(&tree, repo, &oid);
	//printf("tree error is:%d\n",error);
	//const git_commit *parents[] = {parent1, parent2};
	git_oid_fromstr(&oid_tree, "11993bd6502a40705fc3b585b9ad4a6a695b0646");
	git_tree_lookup(&tree, repo, &oid_tree);
	git_oid_fromstr(&parent_id, "b24fb28995f849fb43479dd947ed1fff533e3844");
	git_commit_lookup(&parent, repo, &parent_id);
	git_oid new_commit_id;
	error = git_commit_create(
	    &new_commit_id,
	    repo,
	    "HEAD",                      /* name of ref to update */
	    me,                          /* author */
	    me,                          /* committer */
	    "UTF-8",                     /* message encoding */
	    "TEST COMMIT MSG!",  /* message */
	    tree,                        /* root tree */
	    1,                           /* parent count */
	    &parent);                    /* parents */
	printf("git_commit_create error is:%d\n",error);
	if(repo)
        git_repository_free(repo);
}