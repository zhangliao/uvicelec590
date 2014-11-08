/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2014 Liao Zhang, Yuejiao Hui, Lan xu <zhangliao1212@hotmail.com>
 * 
 * ELEC590_OFS is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ELEC590_OFS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include </home/liaoz/elec590_ofs/src/config.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/sendfile.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <sys/stat.h>  
#include <fcntl.h>  
#include <errno.h>  
#include <openssl/ssl.h>
#include <openssl/err.h>  
#define oops(msg) { perror(msg); exit(errno); }  
#include </usr/include/libssh/libssh.h>
#include <stdlib.h>
#include <stdio.h> 
#include <errno.h>
#include <string.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
typedef struct {
  GtkWidget *widget;
  gint index;
  const gchar *title;
  GtkAssistantPageType type;
  gboolean complete;
} PageInfo;

typedef struct {
	char username[100];
	char password[100];
	char directory[200];
	char query_object[200];
	char commit_sha[100];
	int operation;
} reginfo;



typedef struct {
  char  commit_sha[50];
  char  author[50];
  char  date[50];
  char  message[50];
} version_info;



typedef struct _Private Private;

struct _Private{
	/* ANJUTA: Widgets declaration for elec590_ofs.ui - DO NOT REMOVE */
};

/* For testing purpose, define TEST to use the local (not installed) ui file */
#define TEST
#ifdef TEST
#define UI_FILE "src/elec590_ofs.ui"
#else
#define UI_FILE PACKAGE_DATA_DIR"/ui/elec590_ofs.ui"
#endif
#define TOP_WINDOW "window"

GtkWidget *window;
reginfo userinfo;
reginfo loginfo;
version_info revsion[10];
SSL *ssl;
static GtkWidget *file_window = NULL;
static GtkTreeModel *model = NULL;
static guint timeout = 0;
char *select_object;
char homepath[200] ;
GtkWidget* directory_entry; 
GtkWidget *sw;
GtkWidget *treeview;
GtkListStore *store;
GtkWidget *combo;
static Private* priv = NULL;
const gchar *list_item_data_key_string="list_item_data_label_string";
const gchar *list_item_data_key_type="list_item_data_type";
static void entry_changed    (GtkEditable*, GtkAssistant*);
static void entry_login_username_changed    (GtkEditable*);
static void entry_login_password_changed    (GtkEditable*);
static void button_clicked   (GtkButton*, GtkAssistant*);
void back_btn_clicked(GtkButton* back_btn, gpointer data);
static void assistant_cancel (GtkAssistant*, gpointer);
static void assistant_close  (GtkAssistant*, gpointer);
static GtkWidget *do_list_store (char *dirpath);
void btn_getcommit_clicked(GtkButton* btn_getcommit, gpointer data);
void btn_checkout_clicked(GtkButton* btn_checkout, gpointer data);
void  on_selection_changed(GtkWidget *widget, gpointer data);
int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr);
gint  file_list_click_handle (GtkTreeView *treeview,GtkTreePath *path, GtkTreeViewColumn  *col,gpointer userdata);

enum{ 
  COLUMN_ICON,
  COLUMN_NAME,
  NUM_COLUMNS
};

void ShowCerts(SSL * ssl){
	X509 *cert;
	char *line;
	cert = SSL_get_peer_certificate(ssl);
	if (cert != NULL) {
		printf("Digital Certification Information:\n");
		line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
		printf("Certification: %s\n", line);
		free(line);
		line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
		printf("Issuer: %s\n", line);
		free(line);
		X509_free(cert);
	} 
	else
		printf("No Certification Information！\n");
}

void quick_message (gchar *message) {
	/* Function to open a dialog box displaying the message provided. */    
	GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL |  GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,  "%s", message);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

void destroy (GtkWidget *widget, gpointer data){
	/* Called when the window is closed */
	gtk_main_quit ();
}

static void	entry_changed (GtkEditable *entry,GtkAssistant *assistant){
/* If there is text in the GtkEntry, set the page as complete. Otherwise, stop the user from progressing the next page. */
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
	gint num = gtk_assistant_get_current_page (assistant);
	switch( num ) 
	{
		case 1:
			strcpy(userinfo.username,text);
			break;
		case 2:
			strcpy(userinfo.password,text);
			break;
		case 3:
			strcpy(userinfo.directory,text);
			break;
	}
	GtkWidget *page = gtk_assistant_get_nth_page (assistant, num);
	gtk_assistant_set_page_complete (assistant, page, (strlen (text) > 0));
}

static void	entry_login_username_changed (GtkEditable *entry){
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
	strcpy(loginfo.username,text);
}

static void	entry_login_password_changed (GtkEditable *entry){
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
	strcpy(loginfo.password,text);
}

static void	button_clicked (GtkButton *button, GtkAssistant *assistant){
	GtkProgressBar *progress;
	GtkWidget *page;
	gdouble percent = 0.0;
	page = gtk_assistant_get_nth_page (assistant, 4);
	if ( userinfo.username != NULL && userinfo.password != NULL && opendir(userinfo.directory) != NULL )
	{	
		userinfo.operation=2;//Register
		printf("user operation is:%d\n",userinfo.operation);
		SSL_write(ssl,&userinfo,sizeof(userinfo));
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
		progress = GTK_PROGRESS_BAR (g_object_get_data (G_OBJECT (page), "pbar"));	
		while (percent <= 100.0)
		{
			gchar *message = g_strdup_printf ("%.0f%% Complete", percent);
			gtk_progress_bar_set_fraction (progress, percent / 100.0);
			gtk_progress_bar_set_text (progress, message);
			
			while (gtk_events_pending ())
			gtk_main_iteration ();
			
			g_usleep (500000);
			percent += 20.0;
		}
		gtk_assistant_set_page_complete (assistant, page, TRUE);
	}	
	else
	{
		quick_message("Directory not existed!");
		gtk_assistant_set_page_complete (assistant, page, TRUE);
	}
}

void Click_Register(GtkWidget *widget){  
	GtkWidget *assistant, *entry_username, *label_username, *button, *progress,*entry_password, *label_password, *entry_directory, *label_directory,*hbox;
	guint i;
	PageInfo page[6] = {
		{ NULL, -1, "Introduction",    GTK_ASSISTANT_PAGE_INTRO,    TRUE},//page[0]
		{ NULL, -1, "Username", 	   GTK_ASSISTANT_PAGE_CONTENT,  FALSE},//page[1]
		{ NULL, -1, "Password",        GTK_ASSISTANT_PAGE_CONTENT,  FALSE},//page[2]
		{ NULL, -1, "Directory",	   GTK_ASSISTANT_PAGE_CONTENT, FALSE},//page[3]
		{ NULL, -1, "Processing",      GTK_ASSISTANT_PAGE_PROGRESS, FALSE},//page[4]
		{ NULL, -1, "Confirmation",    GTK_ASSISTANT_PAGE_CONFIRM,  TRUE},//page[5]
	};
	/* Create a new assistant widget with no pages. */
	assistant = gtk_assistant_new ();
	gtk_widget_set_size_request (assistant, 450, 300);
	gtk_widget_hide (window);
	gtk_window_set_title (GTK_WINDOW (assistant), "ELEC590 OFS New User Register");
	gtk_window_set_position(GTK_WINDOW(assistant), GTK_WIN_POS_CENTER);
	page[0].widget = gtk_label_new ("ELEC590 OFS new user register.\n"\
									"Please input your username and password.\n"\
									"OFS will generate acreate a clould storage for you.\n"
									"Storage Capacity will be allocated automatically for you.\n");
	page[1].widget = gtk_box_new (FALSE, 5);
	page[2].widget = gtk_box_new (FALSE, 5);
	page[3].widget = gtk_box_new (FALSE, 5);
	page[4].widget = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	page[5].widget = gtk_label_new ("Register successfully!\n");	
	/* Create the necessary widgets for the page[1]. */
	label_username = gtk_label_new ("Your Username: ");
	entry_username = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (page[1].widget), label_username, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX (page[1].widget), entry_username, FALSE, FALSE, 5);		
	/* Create the necessary widgets for the page[2]. */
	label_password = gtk_label_new ("Your Passowrd: ");
	entry_password = gtk_entry_new ();
	gtk_entry_set_visibility(GTK_ENTRY(entry_password),FALSE);
	gtk_entry_set_invisible_char (GTK_ENTRY(entry_password),'*');
	gtk_box_pack_start (GTK_BOX (page[2].widget), label_password, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX (page[2].widget), entry_password, FALSE, FALSE, 5);	
	/* Create the necessary widgets for the page[3]. The, Attach the progress bar to the GtkAlignment widget for later access.*/
	label_directory = gtk_label_new ("Your Directory: ");
	entry_directory = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (page[3].widget), label_directory, FALSE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX (page[3].widget), entry_directory, FALSE, FALSE, 5);		
	/* Create the necessary widgets for the page[4]. */
	button = gtk_button_new_with_label ("Server Processing");
	progress = gtk_progress_bar_new ();
	hbox = gtk_box_new (FALSE, 5);
	gtk_box_pack_start (GTK_BOX (hbox), progress, TRUE, FALSE, 5);
	gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 5);
	gtk_container_add (GTK_CONTAINER (page[4].widget), hbox);
	g_object_set_data (G_OBJECT (page[4].widget), "pbar", (gpointer) progress);		
	/* Add five pages to the GtkAssistant dialog. */
	for (i = 0; i < 6; i++)
	{
		page[i].index = gtk_assistant_append_page (GTK_ASSISTANT (assistant),page[i].widget);
		gtk_assistant_set_page_title (GTK_ASSISTANT (assistant),page[i].widget, page[i].title);
		gtk_assistant_set_page_type (GTK_ASSISTANT (assistant),page[i].widget, page[i].type);			
		gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant),page[i].widget, page[i].complete);
	}	
	/* Update whether pages 2 through 4 are complete based upon whether there is text in the GtkEntry, the check button is active, or the progress bar is completely filled. */
	g_signal_connect (G_OBJECT (entry_username), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
	g_signal_connect (G_OBJECT (entry_password), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
	g_signal_connect (G_OBJECT (entry_directory), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
	g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (button_clicked), (gpointer) assistant);
	g_signal_connect (G_OBJECT (assistant), "cancel", G_CALLBACK (assistant_cancel), NULL);
	g_signal_connect (G_OBJECT (assistant), "close", G_CALLBACK (assistant_close), NULL);
	g_signal_connect (G_OBJECT (assistant), "destroy",G_CALLBACK (assistant_close), NULL);	
	gtk_widget_show_all (assistant);
}

int Click_Login(GtkWidget *widget){

	ssh_session my_ssh_session;
	int rc;	
	char command[50];
	char dirpath[100];
	// Open session and set options
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "localhost");
	// Connect to server
	rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK)
	{
		fprintf(stderr, "Error connecting to server: %s\n",
		ssh_get_error(my_ssh_session));
		ssh_free(my_ssh_session);	
	}
	// Verify the server's identity
	// For the source code of verify_knowhost(), check previous example
	if (verify_knownhost(my_ssh_session) < 0)
	{
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		quick_message("Connection Failed!");
	}
	// Authenticate ourselves
	rc = ssh_userauth_password(my_ssh_session, loginfo.username, loginfo.password);
	if (rc != SSH_AUTH_SUCCESS)
	{	
		fprintf(stderr, "Error authenticating with password: %s\n",ssh_get_error(my_ssh_session));
		quick_message("Wrong Password!");
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);		
	}
	else
	{
		printf("Session connected successfully!\n");
		memset(dirpath, 0, sizeof(dirpath) );
		ssh_disconnect(my_ssh_session);
		ssh_free(my_ssh_session);
		//memset(&loginfo, 0, sizeof(loginfo));
		loginfo.operation=1;//Login
		printf("user operation is:%d\n",loginfo.operation);
		SSL_write(ssl,&loginfo,sizeof(loginfo));
		SSL_read(ssl,dirpath,sizeof(dirpath)-1);
		printf("Local directory is:%s\n",dirpath);

		if ( opendir(dirpath) )
		{
			do_list_store(dirpath);
			strcpy(homepath,dirpath);
		}
		else
		{
			quick_message("Wrong local directory! Please make it first!");
		}
	}
}

static GtkWidget* create_window (void){
	GtkBuilder *builder;
	GtkWidget *button_font; 
	GtkWidget *button_reg; 
	GtkWidget *button_login; 
	GtkWidget *entry_username; 
	GtkWidget *entry_password; 	
	GError* error = NULL;
	/* Load UI from file */
	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, UI_FILE, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}
	/* Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, NULL);
	/* Get the window object from the ui file */
	window = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
    if (!window)
    {
        g_critical ("Widget \"%s\" is missing in file %s.",
		TOP_WINDOW,
		UI_FILE);
    }	
	button_reg	 = GTK_WIDGET (gtk_builder_get_object (builder, "button_reg"));
	button_font = gtk_bin_get_child(GTK_BIN (button_reg));
    gtk_widget_override_font(GTK_WIDGET(button_font), pango_font_description_from_string("DejaVu Sans Bold 15"));
	button_login	 = GTK_WIDGET (gtk_builder_get_object (builder, "button_login"));
	button_font = gtk_bin_get_child(GTK_BIN (button_login));
    gtk_widget_override_font(GTK_WIDGET(button_font), pango_font_description_from_string("DejaVu Sans Bold 15"));
	entry_username	 = GTK_WIDGET (gtk_builder_get_object (builder, "entry_username"));
	entry_password	 = GTK_WIDGET (gtk_builder_get_object (builder, "entry_password"));
	gtk_entry_set_visibility(GTK_ENTRY(entry_password),FALSE);
	gtk_entry_set_invisible_char (GTK_ENTRY(entry_password),'*');
	g_signal_connect(button_reg, "clicked", G_CALLBACK(Click_Register), NULL);
	priv = g_malloc (sizeof (struct _Private));	
	g_signal_connect (G_OBJECT (entry_username), "changed", G_CALLBACK (entry_login_username_changed), NULL);
	g_signal_connect (G_OBJECT (entry_password), "changed", G_CALLBACK (entry_login_password_changed), NULL);
	g_signal_connect(button_login, "clicked", G_CALLBACK(Click_Login), NULL);
	/* ANJUTA: Widgets initialization for elec590_ofs.ui - DO NOT REMOVE */
	g_object_unref (builder);	
	return window;
}

static void	assistant_cancel (GtkAssistant *assistant, gpointer data){
/* If the dialog is cancelled, delete it from memory and then clean up after the Assistant structure. */
	gtk_widget_show(window);
	gtk_widget_destroy (GTK_WIDGET (assistant));
}

static void	assistant_close (GtkAssistant *assistant, gpointer data){
/* This function is where you would apply the changes and destroy the assistant. */
	gtk_widget_show(window);
	g_print ("You would apply your changes now!\n");
	printf("Username:%s\n",userinfo.username);
	printf("Passowrd:%s\n",userinfo.password);
	printf("Directory:%s\n",userinfo.directory);
	gtk_widget_destroy (GTK_WIDGET (assistant));
}

int verify_knownhost(ssh_session session){
	int state;
	size_t hlen;
	unsigned char *hash = NULL;
	char *hexa;
	char buf[10];
	ssh_key srv_pubkey;
	int rc;	
	state = ssh_is_server_known(session);
	rc = ssh_get_publickey(session, &srv_pubkey);
	if (rc < 0) {
		return -1;
	}
	rc = ssh_get_publickey_hash(srv_pubkey,SSH_PUBLICKEY_HASH_SHA1,&hash,&hlen);
	ssh_key_free(srv_pubkey);
	switch (state)
	{
		case SSH_SERVER_KNOWN_OK:
			break; /* ok */
		case SSH_SERVER_KNOWN_CHANGED:
			fprintf(stderr, "Host key for server changed: it is now:\n");
			ssh_print_hexa("Public key hash", hash, hlen);
			fprintf(stderr, "For security reasons, connection will be stopped\n");
			free(hash);
			return -1;
		case SSH_SERVER_FOUND_OTHER:
			fprintf(stderr, "The host key for this server was not found but an other"
				"type of key exists.\n");
			fprintf(stderr, "An attacker might change the default server key to"
				"confuse your client into thinking the key does not exist\n");
			free(hash);
			return -1;
		case SSH_SERVER_FILE_NOT_FOUND:
			fprintf(stderr, "Could not find known host file.\n");
			fprintf(stderr, "If you accept the host key here, the file will be"
			"automatically created.\n");
		/* fallback to SSH_SERVER_NOT_KNOWN behavior */
		case SSH_SERVER_NOT_KNOWN:
			hexa = ssh_get_hexa(hash, hlen);
			fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
			fprintf(stderr, "Public key hash: %s\n", hexa);
			free(hexa);
			if (fgets(buf, sizeof(buf), stdin) == NULL)
			{
				free(hash);
				return -1;
			}
			if (strncasecmp(buf, "yes", 3) != 0)
			{
				free(hash);
				return -1;
			}
			if (ssh_write_knownhost(session) < 0)
			{
				fprintf(stderr, "Error %s\n", strerror(errno));
				free(hash);
				return -1;
			}
			break;
		case SSH_SERVER_ERROR:
			fprintf(stderr, "Error %s", ssh_get_error(session));
			free(hash);
			return -1;
	}
}

static GtkTreeModel *create_model (GtkEntry* directory_entry){
  gint i = 0;

  GtkTreeIter iter;

  /* create list store */
  store = gtk_list_store_new (NUM_COLUMNS,
                              G_TYPE_STRING,
                              G_TYPE_STRING);

  /* add data to the list store */
	const char* dirpath = (const char*)gtk_entry_get_text(directory_entry);
    DIR* dir = opendir(dirpath);
    struct dirent* enump = NULL;
    size_t name_len;
	gchar *icon_name;

    if(NULL == dir)
    {
        g_print("Open directory failed:%s.\n", dirpath);
    }

    while(enump = readdir(dir))
    {
        name_len = strlen(enump->d_name);

        if( enump->d_name[0] == '.')     
			continue;

        if(DT_DIR == enump->d_type)
        {
			icon_name="folder";
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
								COLUMN_ICON, icon_name,
								COLUMN_NAME, enump->d_name,
								-1);
        }

        if(DT_REG == enump->d_type)
        {	icon_name="text-html";
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
								COLUMN_ICON, icon_name,
								COLUMN_NAME, enump->d_name,
								-1);
        }
    }


  return GTK_TREE_MODEL (store);
}

static void add_columns (GtkTreeView *treeview){
  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkTreeModel *model = gtk_tree_view_get_model (treeview);
  
  /* column for file icon */
  renderer = gtk_cell_renderer_pixbuf_new ();
  g_object_set (G_OBJECT (renderer), "follow-state", TRUE, NULL);
  column = gtk_tree_view_column_new_with_attributes ("Type",
                                                     renderer,
                                                     "icon-name",
                                                     COLUMN_ICON,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_ICON);
  gtk_tree_view_append_column (treeview, column);

  /* column for filename */
  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes ("Name",
                                                     renderer,
                                                     "text",
                                                     COLUMN_NAME,
                                                     NULL);
  gtk_tree_view_column_set_sort_column_id (column, COLUMN_NAME);
  gtk_tree_view_append_column (treeview, column);
}

static gboolean window_closed (GtkWidget *widget, GdkEvent  *event, gpointer   user_data){
  model = NULL;
  window = NULL;
  if (timeout != 0)
    {
      g_source_remove (timeout);
      timeout = 0;
    }
  return FALSE;
}

static GtkWidget *do_list_store (char *dirpath){
  if (!file_window)
    {
		GtkWidget *vbox;
		GtkWidget *hbox;
		GtkWidget *hbox1;
		GtkWidget *label;
		GtkWidget* directory_label;
		GtkWidget* btn_back;
		GtkWidget* btn_getcommit;
		GtkWidget* btn_checkout;
		GtkWidget* btn_resync;
		GtkTreeSelection *selection; 
		
		/* create file_window, etc */
		file_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		//gtk_window_set_screen (GTK_WINDOW (file_window), gtk_widget_get_screen (do_widget));
		gtk_window_set_title (GTK_WINDOW (file_window), "ELEC590 Online File System V0.01");
		
		g_signal_connect (file_window, "destroy",G_CALLBACK (gtk_widget_destroyed), &file_window);
		gtk_container_set_border_width (GTK_CONTAINER (file_window), 8);
	
		vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 8);
		hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
		hbox1 = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
		gtk_container_add (GTK_CONTAINER (file_window), vbox);
		gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
		
		directory_label = gtk_label_new("Path");
		gtk_box_pack_start (GTK_BOX (hbox), directory_label, FALSE, FALSE, 0);
		directory_entry = gtk_entry_new();
		gtk_editable_set_editable ((GtkEditable*)directory_entry, FALSE);
		gtk_entry_set_text((GtkEntry*)(directory_entry), dirpath);
		gtk_box_pack_start (GTK_BOX (hbox), directory_entry, TRUE, TRUE, 0);
		
		btn_back = gtk_button_new_from_icon_name ("gtk-go-back-ltr",GTK_ICON_SIZE_SMALL_TOOLBAR);
		gtk_box_pack_start (GTK_BOX (hbox), btn_back, FALSE, FALSE, 0);
		
		label = gtk_label_new ("Choose a file to view the revisions. Click the button to rollback.");
		gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
		
		sw = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (sw),GTK_SHADOW_ETCHED_IN);
		gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
		gtk_box_pack_start (GTK_BOX (vbox), sw, TRUE, TRUE, 0);
		gtk_box_pack_start (GTK_BOX (vbox), hbox1, FALSE, FALSE, 0);
		
		combo = gtk_combo_box_text_new ();
		gtk_combo_box_text_prepend (GTK_COMBO_BOX_TEXT (combo), NULL,"File/Directory Modified Timestamp");
		gtk_box_pack_start (GTK_BOX (hbox1), combo, FALSE, FALSE, 0);
		
		btn_getcommit = gtk_button_new_from_icon_name ("viewmagfit",GTK_ICON_SIZE_SMALL_TOOLBAR);
		gtk_button_set_always_show_image ((GtkButton*)btn_getcommit, TRUE);
		gtk_button_set_label ((GtkButton*)btn_getcommit,"Query Version");
        gtk_widget_set_margin_left (btn_getcommit, 15);
        gtk_widget_set_margin_right (btn_getcommit, 15);
		gtk_box_pack_start (GTK_BOX (hbox1), btn_getcommit, FALSE, FALSE, 0);
		btn_checkout = gtk_button_new_from_icon_name ("stock_calc-accept",GTK_ICON_SIZE_SMALL_TOOLBAR);
		gtk_button_set_always_show_image ((GtkButton*)btn_checkout, TRUE);
		gtk_button_set_label ((GtkButton*)btn_checkout," Rollback File ");
		gtk_widget_set_margin_left (btn_checkout, 15);
        gtk_widget_set_margin_right (btn_checkout, 15);
		gtk_box_pack_start (GTK_BOX (hbox1), btn_checkout, FALSE, FALSE, 0);
		btn_resync = gtk_button_new_from_icon_name ("view-refresh",GTK_ICON_SIZE_SMALL_TOOLBAR);
		gtk_button_set_always_show_image ((GtkButton*)btn_resync, TRUE);
		gtk_widget_set_margin_left (btn_resync, 15);
        gtk_widget_set_margin_right (btn_resync, 15);
		gtk_button_set_label ((GtkButton*)btn_resync," Resync All ");
		gtk_box_pack_start (GTK_BOX (hbox1), btn_resync, FALSE, FALSE, 0);
		/* create tree model */
		model = create_model ((GtkEntry*)directory_entry);
	
		/* create tree view */
		treeview = gtk_tree_view_new_with_model (model);
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeview), TRUE);
		gtk_tree_view_set_search_column (GTK_TREE_VIEW (treeview),COLUMN_NAME);
	
		g_object_unref (model);
	
		gtk_container_add (GTK_CONTAINER (sw), treeview);
	
		/* add columns to the tree view */
		add_columns (GTK_TREE_VIEW (treeview)); 
		
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		
		
		/* finish & show */
		gtk_window_set_default_size (GTK_WINDOW (file_window), 500, 400);
		gtk_window_set_position (GTK_WINDOW (file_window),GTK_WIN_POS_CENTER);
		gtk_widget_hide (window);
		g_signal_connect(selection, "changed", G_CALLBACK(on_selection_changed), NULL);
		g_signal_connect (file_window, "delete-event",G_CALLBACK (window_closed), NULL);
		g_signal_connect(btn_back, "clicked", G_CALLBACK(back_btn_clicked), (gpointer)(directory_entry));
		g_signal_connect(btn_getcommit, "clicked", G_CALLBACK(btn_getcommit_clicked), (gpointer)(directory_entry));
		g_signal_connect(btn_checkout, "clicked", G_CALLBACK(btn_checkout_clicked), NULL);
		g_signal_connect(treeview, "row-activated", G_CALLBACK(file_list_click_handle), (gpointer)(directory_entry));
    }

  if (!gtk_widget_get_visible (file_window))
    {
      gtk_widget_show_all (file_window);
    }
  else
    {
      gtk_widget_destroy (file_window);
      file_window = NULL;
      if (timeout != 0)
        {
          g_source_remove (timeout);
          timeout = 0;
        }
    }

  return file_window;
}

gint file_list_click_handle (GtkTreeView *treeview1,GtkTreePath *path, GtkTreeViewColumn  *col,gpointer userdata){       
	GtkEntry* path_entry = (GtkEntry*)userdata;
	GtkTreeModel *model1;
	GtkTreeIter   iter;
    model1 = gtk_tree_view_get_model(treeview1); 
    if (gtk_tree_model_get_iter(model1, &iter, path))
    {
		gchar *name;
		gchar *icon;
		gtk_tree_model_get(model1, &iter, COLUMN_NAME, &name,COLUMN_ICON, &icon, -1); 
		//g_print ("Double-clicked %s\n", name);		
		const char* old_path = gtk_entry_get_text(path_entry);
            if( strcmp(icon,"folder") == 0)
            {
                //directory
                gchar* new_path = (gchar*)malloc(strlen(old_path) + strlen(name) + 2);
                strcpy(new_path, old_path);
                strcat(new_path, name);
                strcat(new_path, "/");

                if(opendir(new_path))
                {	
                    gtk_entry_set_text(path_entry, new_path);
					create_model((GtkEntry*)(path_entry));
					/* update tree view */
					gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
                }
                free(new_path);
            }
            else 
            {
                //regular file
                gchar* file_path = (gchar*)malloc(strlen(old_path) + strlen(name) + 1);
                strcpy(file_path, old_path);
                strcat(file_path, name);
                //text_view_file_content(text, file_path);
            }				
       g_free(name);
    }
}

gint calc_start_pos(const char* path){
    size_t path_len = strlen(path);
    const char* p = path;
    gint count = 0;
    const char* p_second_last_slash = path + path_len - 2;
    while(*p_second_last_slash != '/')
    {
        p_second_last_slash--;
    }

    for(; p != p_second_last_slash; ++p)
    {
        count++;
    }

    //skip the slash
    count+=1;

    return count;
}

void back_btn_clicked(GtkButton* back_btn, gpointer data){
    GtkEntry* path_entry = (GtkEntry*)data;
    gint start_pos;
    const char* old_path = gtk_entry_get_text(path_entry);
    if(!strcmp(old_path, homepath))
        return;
    start_pos = calc_start_pos(old_path);
    gtk_editable_delete_text((GtkEditable*)path_entry, start_pos, -1);
    create_model((GtkEntry*)(path_entry));
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
}

void btn_getcommit_clicked(GtkButton* btn_getcommit, gpointer data){
	int i;
	GtkEntry* path_entry = (GtkEntry*)data;
	const char* old_path = gtk_entry_get_text(path_entry);
	char whole_path[200];
	strcpy(whole_path,old_path);
	strcat(whole_path,select_object);
	ReplaceStr(whole_path,homepath,"");
	loginfo.operation=3;//get commit sha
	strcpy(loginfo.query_object,whole_path);
	//printf("user operation is checkout\n");
	printf("select target:%s\n",loginfo.query_object);
	SSL_write(ssl,&loginfo,sizeof(loginfo));
	memset((char*)revsion, 0, sizeof(version_info)*10);
	SSL_read(ssl, (char*)revsion, sizeof(version_info)*10);
	gtk_combo_box_text_remove_all ((GtkComboBoxText *)combo);
	for (i=0;i<10;i++){
		if ( strlen(revsion[i].date))
		{	
			gtk_combo_box_text_append  ((GtkComboBoxText *)combo, NULL,revsion[i].date);
		}
	}
}

void btn_checkout_clicked(GtkButton* btn_checkout, gpointer data){
	int i;
	const char *timestamp = gtk_combo_box_text_get_active_text ((GtkComboBoxText *)combo);
	printf("selct time:%s\n",timestamp);
	printf("select target:%s\n",loginfo.query_object);
	for (i=0;i<10;i++)
	{	
		if ( strcmp(revsion[i].date,timestamp) == 0 )
		{
			strcpy(loginfo.commit_sha,revsion[i].commit_sha);
		}
	}
	loginfo.operation=4;//checkout file
	SSL_write(ssl,&loginfo,sizeof(loginfo));
}

void  on_selection_changed(GtkWidget *widget, gpointer data) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	//memset(&select_object, 0, sizeof(select_object) );
	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) 
	{
		gtk_tree_model_get(model, &iter, COLUMN_NAME, &select_object,  -1);
	}
	gtk_combo_box_text_remove_all ((GtkComboBoxText *)combo);
}

int ReplaceStr(char *sSrc, char *sMatchStr, char *sReplaceStr){
        int  StringLen;
        char caNewString[1000];

        char *FindPos = strstr(sSrc, sMatchStr);
        if( (!FindPos) || (!sMatchStr) )
                return -1;
        while( FindPos )
        {
                memset(caNewString, 0, sizeof(caNewString));
                StringLen = FindPos - sSrc;
                strncpy(caNewString, sSrc, StringLen);
                strcat(caNewString, sReplaceStr);
                strcat(caNewString, FindPos + strlen(sMatchStr));
                strcpy(sSrc, caNewString);

                FindPos = strstr(sSrc, sMatchStr);
        }
        return 0;
}

int main (int argc, char *argv[]){
 	GtkWidget *window;
	#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
	#endif
	gtk_init (&argc, &argv);
	window = create_window ();
	gtk_widget_show (window);
	int sd = socket(PF_INET, SOCK_STREAM, 0);  
    if(sd == -1) oops("socket");  
    struct sockaddr_in address;  
    bzero(&address, sizeof(address));  
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = inet_addr("192.168.0.1");  
    address.sin_port =  8888;  
	SSL_CTX *ctx;
	/* SSL 库初始化，参看 ssl-server.c 代码 */
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		ERR_print_errors_fp(stdout);
		exit(1);
	}
    int connect_ret = connect(sd, (struct sockaddr*)&address, sizeof(address));  
    if(connect_ret == -1) oops("connect");  
	
	/* 基于 ctx 产生一个新的 SSL */
	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sd);
	
	/* 建立 SSL 连接 */
	if (SSL_connect(ssl) == -1)
		ERR_print_errors_fp(stderr);
	else {
		printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
		ShowCerts(ssl);
	}
	memset(&userinfo, 0, sizeof(reginfo) );
	memset(&loginfo, 0, sizeof(reginfo) );
	gtk_main ();
    close(sd);  
    return EXIT_SUCCESS;  
	g_free (priv);
	return 0;
}