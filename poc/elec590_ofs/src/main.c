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
#include<stdio.h>  
#include<stdlib.h>  
#include<string.h>  
#include<unistd.h>  
  
#include<sys/types.h>  
#include<sys/socket.h>  
#include<sys/sendfile.h>  
#include<netinet/in.h>  
#include<arpa/inet.h>  
  
#include<sys/stat.h>  
#include<fcntl.h>  
  
#include<errno.h>  
#include <openssl/ssl.h>
#include <openssl/err.h>  
#define oops(msg) { perror(msg); exit(errno); }  
void ShowCerts(SSL * ssl)
{
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
		printf("无证书信息！\n");
}

static void entry_changed    (GtkEditable*, GtkAssistant*);
//static void button_toggled   (GtkCheckButton*, GtkAssistant*);
static void button_clicked   (GtkButton*, GtkAssistant*);
static void assistant_cancel (GtkAssistant*, gpointer);
static void assistant_close  (GtkAssistant*, gpointer);
GtkWidget *window;

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
} reginfo;

reginfo userinfo;
SSL *ssl;
typedef struct _Private Private;
struct _Private
{
	/* ANJUTA: Widgets declaration for elec590_ofs.ui - DO NOT REMOVE */
};

static Private* priv = NULL;

/* For testing purpose, define TEST to use the local (not installed) ui file */
#define TEST
#ifdef TEST
#define UI_FILE "src/elec590_ofs.ui"
#else
#define UI_FILE PACKAGE_DATA_DIR"/ui/elec590_ofs.ui"
#endif
#define TOP_WINDOW "window"

/* Signal handlers */
/* Note: These may not be declared static because signal autoconnection
 * only works with non-static methods
 */

/* Called when the window is closed */
void
destroy (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

/* If there is text in the GtkEntry, set the page as complete. Otherwise, stop the user from progressing the next page. */
static void	entry_changed (GtkEditable *entry,GtkAssistant *assistant)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
  gint num = gtk_assistant_get_current_page (assistant);
  //printf("Page Num:%d\n",num);
  //printf("Entry:%s\n",text);
  switch( num ) 
  {
    case 1:
          strcpy(userinfo.username,text);
    case 2:
          strcpy(userinfo.password,text);
	case 3:
          strcpy(userinfo.directory,text);
  }
  GtkWidget *page = gtk_assistant_get_nth_page (assistant, num);
  gtk_assistant_set_page_complete (assistant, page, (strlen (text) > 0));
}

/* If the check button is toggled, set the page as complete. Otherwise, stop the user from progressing the next page. 
static void	button_toggled (GtkCheckButton *toggle, GtkAssistant *assistant)
{
  gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));
  gtk_assistant_set_page_complete (assistant, GTK_WIDGET (toggle), active);
}*/

/* Fill up the progress bar, 10% every second when the button is clicked. Then, set the page as complete when the progress bar is filled. */
static void	button_clicked (GtkButton *button, GtkAssistant *assistant)
{
  GtkProgressBar *progress;
  GtkWidget *page;
  gdouble percent = 0.0;
  //SSL_write(ssl,userinfo.username,sizeof(userinfo.username)-1);
  SSL_write(ssl,&userinfo,sizeof(userinfo));
  gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
  page = gtk_assistant_get_nth_page (assistant, 4);
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

void Click_Register(GtkWidget *widget, gpointer label1)
{  
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
  g_signal_connect (G_OBJECT (assistant), "destroy",G_CALLBACK (gtk_main_quit), NULL);
  
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

  /* Set the introduction and conclusion pages as complete so they can be incremented or closed. */
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant),page[i].widget, page[i].complete);
  }
  
  /* Update whether pages 2 through 4 are complete based upon whether there is text in the GtkEntry, the check button is active, or the progress bar is completely filled. */
  g_signal_connect (G_OBJECT (entry_username), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
  g_signal_connect (G_OBJECT (entry_password), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
  g_signal_connect (G_OBJECT (entry_directory), "changed", G_CALLBACK (entry_changed), (gpointer) assistant);
  g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (button_clicked), (gpointer) assistant);
  g_signal_connect (G_OBJECT (assistant), "cancel", G_CALLBACK (assistant_cancel), NULL);
  g_signal_connect (G_OBJECT (assistant), "close", G_CALLBACK (assistant_close), NULL);
  gtk_widget_show_all (assistant);
  
}

static GtkWidget*
create_window (void)
{
	
	GtkBuilder *builder;
	GtkWidget *button_font; 
	GtkWidget *button_reg; 
	GtkWidget *button_login; 
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

	entry_password	 = GTK_WIDGET (gtk_builder_get_object (builder, "entry_password"));
	gtk_entry_set_visibility(GTK_ENTRY(entry_password),FALSE);
	gtk_entry_set_invisible_char (GTK_ENTRY(entry_password),'*');
	g_signal_connect(button_reg, "clicked", G_CALLBACK(Click_Register), NULL);
	priv = g_malloc (sizeof (struct _Private));
	
	/* ANJUTA: Widgets initialization for elec590_ofs.ui - DO NOT REMOVE */

	g_object_unref (builder);

	
	return window;
}

int
main (int argc, char *argv[])
{
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
		gtk_main ();
  
        close(sd);  
        return EXIT_SUCCESS;  
		


	g_free (priv);


	return 0;
}


/* If the dialog is cancelled, delete it from memory and then clean up after the Assistant structure. */
static void	assistant_cancel (GtkAssistant *assistant, gpointer data)
{
  gtk_widget_destroy (GTK_WIDGET (assistant));
}

/* This function is where you would apply the changes and destroy the assistant. */
static void	assistant_close (GtkAssistant *assistant, gpointer data)
{
  g_print ("You would apply your changes now!\n");
  printf("Username:%s\n",userinfo.username);
  printf("Passowrd:%s\n",userinfo.password);
  printf("Directory:%s\n",userinfo.directory);
  gtk_widget_destroy (GTK_WIDGET (assistant));
}
