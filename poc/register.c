#include <gtk/gtk.h>
#include <string.h>

static void entry_changed    (GtkEditable*, GtkAssistant*);
static void button_toggled   (GtkCheckButton*, GtkAssistant*);
static void button_clicked   (GtkButton*, GtkAssistant*);
static void assistant_cancel (GtkAssistant*, gpointer);
static void assistant_close  (GtkAssistant*, gpointer);

typedef struct {
  GtkWidget *widget;
  gint index;
  const gchar *title;
  GtkAssistantPageType type;
  gboolean complete;
} PageInfo;

int main (int argc, char *argv[])
{
  GtkWidget *assistant, *entry_username, *label_username, *button, *progress,*entry_password, *label_password, *hbox;
  guint i;
  PageInfo page[5] = {
    { NULL, -1, "Introduction",    GTK_ASSISTANT_PAGE_INTRO,    TRUE},//page[0]
    { NULL, -1, "Username", 	   GTK_ASSISTANT_PAGE_CONTENT,  FALSE},//page[1]
	{ NULL, -1, "Password",        GTK_ASSISTANT_PAGE_CONTENT,  FALSE},//page[2]
    { NULL, -1, "Click the Button",GTK_ASSISTANT_PAGE_PROGRESS, FALSE},//page[3]
    { NULL, -1, "Confirmation",    GTK_ASSISTANT_PAGE_CONFIRM,  TRUE},//page[4]
  };
  
  gtk_init (&argc, &argv);
   
  /* Create a new assistant widget with no pages. */
  assistant = gtk_assistant_new ();
  gtk_widget_set_size_request (assistant, 450, 300);
  gtk_window_set_title (GTK_WINDOW (assistant), "ELEC590 OFS New User Register");
  gtk_window_set_position(GTK_WINDOW(assistant), GTK_WIN_POS_CENTER);
  g_signal_connect (G_OBJECT (assistant), "destroy",
                    G_CALLBACK (gtk_main_quit), NULL);
  
  page[0].widget = gtk_label_new ("ELEC590 OFS new user register.\n"\
                                  "Please input your username and password.\n"\
                                  "OFS will generate acreate a clould storage for you.\n"
								  "Storage Capacity will be allocated automatically for you.\n");
  page[1].widget = gtk_box_new (FALSE, 5);
  page[2].widget = gtk_box_new (FALSE, 5);
  page[3].widget = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  page[4].widget = gtk_label_new ("Register successfully!\n");
  
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
  
  /* Create the necessary widgets for the fourth page. The, Attach the progress bar
   * to the GtkAlignment widget for later access.*/
  button = gtk_button_new_with_label ("Click me to get an SSH certification!");
  progress = gtk_progress_bar_new ();
  hbox = gtk_box_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), progress, TRUE, FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 5);
  gtk_container_add (GTK_CONTAINER (page[3].widget), hbox);
  g_object_set_data (G_OBJECT (page[3].widget), "pbar", (gpointer) progress);
  
  /* Add five pages to the GtkAssistant dialog. */
  for (i = 0; i < 5; i++)
  {
    page[i].index = gtk_assistant_append_page (GTK_ASSISTANT (assistant),
                                               page[i].widget);
    gtk_assistant_set_page_title (GTK_ASSISTANT (assistant),
                                  page[i].widget, page[i].title);
    gtk_assistant_set_page_type (GTK_ASSISTANT (assistant),
                                  page[i].widget, page[i].type);

    /* Set the introduction and conclusion pages as complete so they can be
     * incremented or closed. */
    gtk_assistant_set_page_complete (GTK_ASSISTANT (assistant),
                                     page[i].widget, page[i].complete);
  }
  
  /* Update whether pages 2 through 4 are complete based upon whether there is
   * text in the GtkEntry, the check button is active, or the progress bar
   * is completely filled. */
  g_signal_connect (G_OBJECT (entry_username), "changed",
                    G_CALLBACK (entry_changed), (gpointer) assistant);
  g_signal_connect (G_OBJECT (entry_password), "changed",
                    G_CALLBACK (entry_changed), (gpointer) assistant);
  g_signal_connect (G_OBJECT (button), "clicked",
                    G_CALLBACK (button_clicked), (gpointer) assistant);
  g_signal_connect (G_OBJECT (assistant), "cancel",
                    G_CALLBACK (assistant_cancel), NULL);
  g_signal_connect (G_OBJECT (assistant), "close",
                    G_CALLBACK (assistant_close), NULL);
  gtk_widget_show_all (assistant);
  gtk_main ();
  return 0;
}

/* If there is text in the GtkEntry, set the page as complete. Otherwise,
 * stop the user from progressing the next page. */
static void
entry_changed (GtkEditable *entry,
               GtkAssistant *assistant)
{
  const gchar *text = gtk_entry_get_text (GTK_ENTRY (entry));
  gint num = gtk_assistant_get_current_page (assistant);
  GtkWidget *page = gtk_assistant_get_nth_page (assistant, num);
  gtk_assistant_set_page_complete (assistant, page, (strlen (text) > 0));
}

/* If the check button is toggled, set the page as complete. Otherwise,
 * stop the user from progressing the next page. */
static void
button_toggled (GtkCheckButton *toggle,
                GtkAssistant *assistant)
{
  gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle));
  gtk_assistant_set_page_complete (assistant, GTK_WIDGET (toggle), active);
}

/* Fill up the progress bar, 10% every second when the button is clicked. Then,
 * set the page as complete when the progress bar is filled. */
static void
button_clicked (GtkButton *button,
                GtkAssistant *assistant)
{
  GtkProgressBar *progress;
  GtkWidget *page;
  gdouble percent = 0.0;
  
  gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
  page = gtk_assistant_get_nth_page (assistant, 3);
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

/* If the dialog is cancelled, delete it from memory and then clean up after
 * the Assistant structure. */
static void
assistant_cancel (GtkAssistant *assistant,
                  gpointer data)
{
  gtk_widget_destroy (GTK_WIDGET (assistant));
}

/* This function is where you would apply the changes and destroy the assistant. */
static void
assistant_close (GtkAssistant *assistant,
                 gpointer data)
{
  g_print ("You would apply your changes now!\n");
  gtk_widget_destroy (GTK_WIDGET (assistant));
}