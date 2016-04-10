#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <libgen.h>

typedef struct {
	GtkWidget *menu_label;
   	GtkWidget *menu;
   	GtkWidget *new;
   	GtkWidget *open;
   	GtkWidget *save;
   	GtkWidget *close;
   	GtkWidget *separator;
   	GtkWidget *quit;
} FileMenu;

typedef struct {
	GtkWidget *menu_label;
	GtkWidget *menu;
	GtkWidget *cut;
	GtkWidget *copy;
 	GtkWidget *paste;
} EditMenu;

typedef struct {
   	GtkWidget *menu_label;
   	GtkWidget *menu;
   	GtkWidget *font;
} OptionsMenu;

typedef struct {
   	GtkWidget *menu_label;
   	GtkWidget *menu;
   	GtkWidget *about;
} HelpMenu;

typedef struct {
   	GtkWidget *scrolled_window;
   	GtkWidget *textview;
   	GtkWidget *tab_label;
} FileObject;

typedef struct {
   	GtkWidget *toplevel;
   	GtkWidget *vbox;
   	GtkWidget *hbox;
   	GtkWidget *notebook;
   	GtkWidget *menubar;
   	GtkWidget *toolbar;
   	GtkWidget *sidebar;
   	GtkWidget *reg_bank;
   	GtkWidget *inst_mem;
   	GtkWidget *data_mem;
   	GtkWidget *reg_table;
   	GtkWidget *inst_table;
   	GtkWidget *data_table;
   	FileMenu *filemenu;
	EditMenu *editmenu;
   	OptionsMenu *options_menu;
   	HelpMenu *helpmenu;
} TextEditUI;

typedef struct {
   	gchar *filename;
   	gint tab_number;
} FileData;

static int files_open = 0;
static char str_buffer[16];
static GtkWidget *reg_val[16];
static GtkWidget *inst_val[256];
static GtkWidget *data_val[256];
static GList *filename_data = NULL;   
static PangoFontDescription *desc = NULL;  

static void quit_application(GtkWidget *, gpointer);
static void text_edit_init_GUI(TextEditUI *);
static void text_edit_create_menus(TextEditUI *);
static void create_reg_bank(TextEditUI *);
static void create_inst_mem(TextEditUI *);
static void create_data_mem(TextEditUI *);
static void init_reg_bank();
static void init_inst_mem();
static void init_data_mem();
static void clear_reg_bank();
static void clear_data_mem();
static void clear_inst_mem();
static void fill_regs_mem();
static void clear_regs_mem();
static void quick_message(gchar *);
static void assemble(GtkWidget *, TextEditUI *);
static void execute(GtkWidget *, TextEditUI *);
static void text_edit_create_toolbar_items(TextEditUI *);
FileObject *text_edit_file_new(void);
static void text_edit_tab_new_with_file(GtkMenuItem *, GtkNotebook *);
static void text_edit_select_font(GtkMenuItem *, gpointer);
static void text_edit_apply_font_selection(GtkNotebook *);
static void text_edit_open_file(GtkMenuItem *, GtkNotebook *);
static void text_edit_cut_to_clipboard(GtkMenuItem *, GtkNotebook *);
static void text_edit_copy_to_clipboard(GtkMenuItem *, GtkNotebook *);
static void text_edit_paste_from_clipboard(GtkMenuItem *, GtkNotebook *);
static void text_edit_show_about_dialog(GtkMenuItem *, GtkWindow *);
static void text_edit_close_file(GtkMenuItem *, GtkNotebook *);
static void text_edit_save_file(GtkMenuItem *, GtkNotebook *);
static void text_edit_register_filename(gchar *fname, gint tab_num);
static gchar *text_edit_get_filename(gint tab_num);

int main(int argc, char *argv[])
{
   	TextEditUI app;
	gtk_init(&argc, &argv);
	app.toplevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);  
   	gtk_window_set_title(GTK_WINDOW(app.toplevel), "EZ-ARM");
   	gtk_window_set_default_size(GTK_WINDOW(app.toplevel), 800, 600);
   	g_signal_connect(G_OBJECT(app.toplevel), "destroy", G_CALLBACK(quit_application), NULL);
   	text_edit_init_GUI(&app);   
   	gtk_widget_show_all(app.toplevel);
   	gtk_main();

   	return 0;
}

static void quit_application(GtkWidget *window, gpointer data)
{ 
	gtk_main_quit(); 
	remove("out.txt");
}

static void text_edit_init_GUI(TextEditUI *app)
{
   	desc = pango_font_description_from_string("Progsole normal 12");

   	FileObject *file = text_edit_file_new();

   	text_edit_register_filename("Sin título", 0);

   	app->vbox = gtk_vbox_new(FALSE, 0);
   	app->hbox = gtk_hbox_new(FALSE, 0);
   	app->notebook = gtk_notebook_new();
   	app->menubar = gtk_menu_bar_new();
   	app->sidebar = gtk_notebook_new();

   	create_reg_bank(app);
   	create_inst_mem(app);
   	create_data_mem(app);

   	text_edit_create_menus(app);
   	text_edit_create_toolbar_items(app);   

   	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(app->notebook), TRUE);
   	gtk_notebook_append_page(GTK_NOTEBOOK(app->notebook), file->scrolled_window, file->tab_label);
   	gtk_box_pack_start(GTK_BOX(app->vbox), app->hbox, TRUE, TRUE, 0);
   	gtk_box_pack_start(GTK_BOX(app->hbox), app->notebook, TRUE, TRUE, 0);
   	gtk_box_pack_start(GTK_BOX(app->hbox), app->sidebar, FALSE, TRUE, 0);
   
   	gtk_container_add(GTK_CONTAINER(app->toplevel), app->vbox);
}

static void init_reg_bank()
{
   	for (int i = 0; i < 16; i++)
	{
		reg_val[i] = gtk_label_new("0x00000000");
   	}
}

static void init_inst_mem()
{
   	for (int i = 0; i < 256; i++){
		inst_val[i] = gtk_label_new("0x00000000");
   	}
}

static void init_data_mem()
{
   	for (int i = 0; i < 256; i++){
		data_val[i] = gtk_label_new("0x00000000");
   	}
}

static void clear_reg_bank()
{
   	for (int i = 0; i < 16; i++){
		gtk_label_set_text(GTK_LABEL(reg_val[i]), "0x00000000");
   	}
}

static void clear_inst_mem()
{
   	for (int i = 0; i < 256; i++){
		gtk_label_set_text(GTK_LABEL(inst_val[i]), "0x00000000");
   	}
}

static void clear_data_mem()
{
   	for (int i = 0; i < 256; i++){
		gtk_label_set_text(GTK_LABEL(data_val[i]), "0x00000000");
   	}
}

static void create_reg_bank(TextEditUI *app)
{
   	app->reg_table = gtk_table_new(16, 3,FALSE);
   	app->reg_bank = gtk_scrolled_window_new(NULL, NULL);
   	gtk_notebook_append_page(GTK_NOTEBOOK(app->sidebar), app->reg_bank, gtk_label_new("Banco de Registros"));
   	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(app->reg_bank), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);   
   	gtk_container_set_border_width(GTK_CONTAINER(app->reg_table), 60);
   	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(app->reg_bank),app->reg_table);
   	gtk_table_attach_defaults(GTK_TABLE(app->reg_table), gtk_vseparator_new(),1,2,0,16);

   	init_reg_bank();

   	for (int i = 0; i < 16; i++)
	{
  		sprintf(str_buffer, "R%d", i);
		gtk_table_attach_defaults(GTK_TABLE(app->reg_table), gtk_label_new(str_buffer), 0, 1, i, i + 1);
		gtk_table_attach_defaults(GTK_TABLE(app->reg_table), reg_val[i], 2, 3, i, i + 1);
   	}
}

static void create_inst_mem(TextEditUI *app)
{
   	app->inst_table = gtk_table_new(256, 3, FALSE);
   	app->inst_mem = gtk_scrolled_window_new(NULL, NULL);
   	gtk_notebook_append_page(GTK_NOTEBOOK(app->sidebar), app->inst_mem, gtk_label_new("Memoria de Programa"));
   	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(app->inst_mem), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
   	gtk_container_set_border_width(GTK_CONTAINER(app->inst_table), 60);
   	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(app->inst_mem), app->inst_table);
   	gtk_table_attach_defaults(GTK_TABLE(app->inst_table), gtk_vseparator_new(), 1, 2, 0, 256);

   	init_inst_mem();

   	for (int i = 0; i < 256; i++)
	{
  		sprintf(str_buffer, "0x%03X", i*4);
		gtk_table_attach_defaults(GTK_TABLE(app->inst_table), gtk_label_new(str_buffer), 0, 1, i, i + 1);
		gtk_table_attach_defaults(GTK_TABLE(app->inst_table), inst_val[i], 2, 3, i, i + 1);
   	}
}

static void create_data_mem(TextEditUI *app)
{
   	app->data_table = gtk_table_new(256, 3, FALSE);
   	app->data_mem = gtk_scrolled_window_new(NULL, NULL);
   	gtk_notebook_append_page(GTK_NOTEBOOK(app->sidebar), app->data_mem, gtk_label_new("Memoria de Datos"));
   	gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(app->data_mem), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);  
   	gtk_container_set_border_width(GTK_CONTAINER(app->data_table), 60);
   	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(app->data_mem), app->data_table);
   	gtk_table_attach_defaults(GTK_TABLE(app->data_table), gtk_vseparator_new(), 1, 2, 0, 256);

   	init_data_mem();

   	for (int i = 0; i < 256; i++)
	{
  		sprintf(str_buffer, "0x%03X", 1024 + (i * 4));
		gtk_table_attach_defaults(GTK_TABLE(app->data_table), gtk_label_new(str_buffer), 0, 1, i, i + 1);
		gtk_table_attach_defaults(GTK_TABLE(app->data_table), data_val[i], 2, 3, i, i + 1);
   	}
}

static void fill_regs_mem(TextEditUI *app)
{
	char line[32];
	FILE *regs = fopen(".regs.txt", "r");
	FILE *mem = fopen(".inst_mem.txt", "r");
	FILE *inst_mem = fopen("out.txt", "r");

	for (int i = 0; i < 16; i++)
	{
		fgets(line, 32, regs);
		sprintf(str_buffer, "0x%08X", atoi(line));
		gtk_label_set_text( GTK_LABEL(reg_val[i]), str_buffer);		
   	}

	for (int i = 0; i < 256; i++)
	{
		fgets(line, 32, mem);
		sprintf(str_buffer, "0x%08X", atoi(line));
		gtk_label_set_text(GTK_LABEL(data_val[i]), str_buffer);
   	}

	if (inst_mem != NULL)
	{
		for (int i = 0; i < 256; i++)
		{
			char buffer[19];
			char *p;
			int ival;
			if (fgets(buffer, 19, inst_mem) != NULL )
			{
				strtol(buffer, &p, 16);
				ival = (int)strtol(p, NULL, 16);
			      	sprintf(str_buffer, "0x%08X", ival);
				gtk_label_set_text(GTK_LABEL(inst_val[i]), str_buffer);
			}	
		}	
	} 
	else
	{
		quick_message("No se puede generar la memoria de programa sin ensamblar primero.");
	}
}

static void clear_regs_mem()
{
	clear_reg_bank();
	clear_data_mem();
	clear_inst_mem();
}

static void quick_message(gchar *message) 
{
   	GtkWidget *dialog, *label, *content_area;
   	dialog = gtk_dialog_new_with_buttons("Atención", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_NONE, NULL);
   	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   	label = gtk_label_new(message);
	g_signal_connect_swapped (dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
   	gtk_container_add (GTK_CONTAINER (content_area), label);
   	gtk_widget_show_all (dialog);
}

static void assemble(GtkWidget *widget, TextEditUI *app)
{
	GtkWidget *scrolled_win;
   	gint current_page;
   	GtkWidget *tab_label;
	GtkWidget *dialog;
   	gchar *filename;
   	gchar *contents;
	current_page = gtk_notebook_get_current_page(app->notebook);
	filename = text_edit_get_filename(current_page);
	if (strcmp(filename, "Sin título") == 0)
	{
		quick_message("Debe guardar el archivo antes de ensamblarlo.");
	} 
	else 
	{
		char call[64];
		strcpy(call, "./ezasm ");
		strcat(call, filename);
		int status = system(call);
		if (status == 0)
		{
			quick_message("¡Archivo ensamblado exitosamente! Ver out.txt.");
		} 
		else
		{
			quick_message("Error durante el ensamblado. Ver Módulo de Errores Ensamblado.txt.");
		}
	}
}

static void execute(GtkWidget *widget, TextEditUI *app)
{
	GtkWidget *scrolled_win;
   	gint current_page;
   	GtkWidget *tab_label;
	GtkWidget *dialog;
   	gchar *filename;
   	gchar *contents;
	current_page = gtk_notebook_get_current_page(app->notebook);
	filename = text_edit_get_filename(current_page);
	if (strcmp(filename, "Sin título") == 0)
	{
		quick_message("Debe guardar el archivo antes de simularlo.");
	} 
	else 
	{
		clear_regs_mem();
		char call[64];
		strcpy(call, "./ezsim ");
		strcat(call, filename);
		int status = system(call);
		if (status == 0)
		{
			fill_regs_mem(app);
			char t_est[128];
			char line[64];
			FILE *est_time = fopen(".exec_time.txt", "r");
			fgets(line, 64, est_time);
			sprintf(t_est, "¡Archivo simulado exitosamente!\n");
			strcat(t_est, line);
			strcat(t_est, "\n");
			FILE *flags = fopen(".flags.txt", "r");
			fgets(line, 64, flags);
			strcat(t_est, line);
			quick_message(t_est);
		} 
		else
		{
			quick_message("Error durante la simulación. Ver Módulo de Errores Simulador.txt.");
		}
	}
}

static void text_edit_create_menus(TextEditUI *app)
{
	FileMenu *file;
   	EditMenu *edit;
   	OptionsMenu *options;
   	HelpMenu *help;
   	GtkAccelGroup *group = gtk_accel_group_new();

   	app->filemenu = g_new(FileMenu, 1);
   	app->editmenu = g_new(EditMenu, 1);
   	app->options_menu = g_new(OptionsMenu, 1);
   	app->helpmenu = g_new(HelpMenu, 1);

   	file = app->filemenu;
   	edit = app->editmenu;
   	options = app->options_menu;
   	help = app->helpmenu;

   	gtk_window_add_accel_group(GTK_WINDOW(app->toplevel), group);

   	file->menu_label = gtk_menu_item_new_with_label("Archivo");
   	file->menu = gtk_menu_new();
   	file->new = gtk_image_menu_item_new_from_stock(GTK_STOCK_NEW, group);
   	file->open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, group);
   	file->save = gtk_image_menu_item_new_from_stock(GTK_STOCK_SAVE, group);
   	file->close = gtk_image_menu_item_new_from_stock(GTK_STOCK_CLOSE, group);
   	file->separator = gtk_separator_menu_item_new();
   	file->quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, group);
   	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file->menu_label), file->menu);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->new);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->open);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->save);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->close);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->separator);
   	gtk_menu_shell_append(GTK_MENU_SHELL(file->menu), file->quit);
   	gtk_menu_set_accel_group(GTK_MENU(file->menu), group);
   	gtk_widget_add_accelerator(file->new, "activate", group, GDK_N, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
   	gtk_widget_add_accelerator(file->open, "activate", group, GDK_O, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
   	gtk_widget_add_accelerator(file->save, "activate", group, GDK_S, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator(file->close, "activate", group, GDK_W, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
   	gtk_widget_add_accelerator(file->quit, "activate", group, GDK_Q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

   	edit->menu_label = gtk_menu_item_new_with_label("Editar");
   	edit->menu = gtk_menu_new();
   	edit->cut = gtk_image_menu_item_new_from_stock(GTK_STOCK_CUT, group);
   	edit->copy = gtk_image_menu_item_new_from_stock(GTK_STOCK_COPY, group);
   	edit->paste = gtk_image_menu_item_new_from_stock(GTK_STOCK_PASTE, group);
   	gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit->menu_label), edit->menu);
   	gtk_menu_shell_append(GTK_MENU_SHELL(edit->menu), edit->cut);
   	gtk_menu_shell_append(GTK_MENU_SHELL(edit->menu), edit->copy);
   	gtk_menu_shell_append(GTK_MENU_SHELL(edit->menu), edit->paste);
   	gtk_menu_set_accel_group(GTK_MENU(edit->menu), group);
   	gtk_widget_add_accelerator(edit->cut, "activate", group, GDK_X, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
   	gtk_widget_add_accelerator(edit->copy, "activate", group, GDK_C, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
   	gtk_widget_add_accelerator(edit->paste, "activate", group, GDK_V, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

   	options->menu_label = gtk_menu_item_new_with_label("Opciones");
   	options->menu = gtk_menu_new();
   	options->font = gtk_image_menu_item_new_from_stock(GTK_STOCK_SELECT_FONT, NULL);
   	gtk_menu_item_set_submenu(GTK_MENU_ITEM(options->menu_label), options->menu);
   	gtk_menu_shell_append(GTK_MENU_SHELL(options->menu), options->font);

   	help->menu_label = gtk_menu_item_new_with_label("Ayuda");
   	help->menu = gtk_menu_new();
   	help->about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
   	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help->menu_label), help->menu);
   	gtk_menu_shell_append(GTK_MENU_SHELL(help->menu), help->about);

   	gtk_menu_shell_append(GTK_MENU_SHELL(app->menubar), file->menu_label);
   	gtk_menu_shell_append(GTK_MENU_SHELL(app->menubar), edit->menu_label);
   	gtk_menu_shell_append(GTK_MENU_SHELL(app->menubar), options->menu_label);
   	gtk_menu_shell_append(GTK_MENU_SHELL(app->menubar), help->menu_label);

   	g_signal_connect(G_OBJECT(file->new), "activate", G_CALLBACK(text_edit_tab_new_with_file), (gpointer) app->notebook);
	g_signal_connect(G_OBJECT(file->open), "activate", G_CALLBACK(text_edit_open_file), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(file->save), "activate", G_CALLBACK(text_edit_save_file), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(file->close), "activate", G_CALLBACK(text_edit_close_file), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(file->quit), "activate", G_CALLBACK(quit_application), NULL);
   	g_signal_connect(G_OBJECT(edit->cut), "activate", G_CALLBACK(text_edit_cut_to_clipboard), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(edit->copy), "activate", G_CALLBACK(text_edit_copy_to_clipboard), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(edit->paste), "activate", G_CALLBACK(text_edit_paste_from_clipboard), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(options->font), "activate", G_CALLBACK(text_edit_select_font), (gpointer) app->notebook);
   	g_signal_connect(G_OBJECT(help->about), "activate", G_CALLBACK(text_edit_show_about_dialog), (gpointer) app->toplevel);

   	gtk_box_pack_start(GTK_BOX(app->vbox), app->menubar, FALSE, FALSE, 0);
}

static void text_edit_create_toolbar_items(TextEditUI *app)
{
   	GtkWidget *toolbar;
   	GtkToolItem *new, *open, *save, *assm, *run;

   	toolbar = app->toolbar = gtk_toolbar_new();
   	gtk_toolbar_set_show_arrow(GTK_TOOLBAR(toolbar), TRUE);
   	new = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
   	open = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
   	save = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
   	assm = gtk_tool_button_new_from_stock(GTK_STOCK_CONVERT);
   	run = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY);

   	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), new, 0);
   	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), open, 1);
   	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), save, 2);
   	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), assm, 3);
   	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), run, 4);
   
   	g_signal_connect_swapped(G_OBJECT(new), "clicked", G_CALLBACK(gtk_menu_item_activate), (gpointer) app->filemenu->new);
   	g_signal_connect_swapped(G_OBJECT(open), "clicked", G_CALLBACK(gtk_menu_item_activate), (gpointer) app->filemenu->open);
   	g_signal_connect_swapped(G_OBJECT(save), "clicked", G_CALLBACK(gtk_menu_item_activate), (gpointer) app->filemenu->save);
   	g_signal_connect(G_OBJECT(assm), "clicked", G_CALLBACK(assemble), (gpointer) app);
   	g_signal_connect(G_OBJECT(run), "clicked", G_CALLBACK(execute), (gpointer) app);

   	gtk_box_pack_start(GTK_BOX(app->vbox), toolbar, FALSE, FALSE, 0);
}

FileObject *text_edit_file_new(void)
{
   	FileObject *new_file = g_new(FileObject, 1);
   	new_file->scrolled_window = gtk_scrolled_window_new(NULL, NULL);
   	new_file->textview = gtk_text_view_new();
      	new_file->tab_label = gtk_label_new("Sin título");

   	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(new_file->scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
   	gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(new_file->scrolled_window), GTK_SHADOW_IN);
   	gtk_container_set_border_width(GTK_CONTAINER(new_file->scrolled_window), 3);
   	gtk_text_view_set_left_margin(GTK_TEXT_VIEW(new_file->textview), 3);
   	gtk_text_view_set_right_margin(GTK_TEXT_VIEW(new_file->textview), 3);
   	gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(new_file->textview), 1);
   	gtk_widget_modify_font(new_file->textview, desc);  

   	gtk_container_add(GTK_CONTAINER(new_file->scrolled_window), new_file->textview);

   	return new_file;
}

static void text_edit_tab_new_with_file(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	FileObject *f = text_edit_file_new();
   	gint current_tab;
   	current_tab = gtk_notebook_append_page(notebook, f->scrolled_window, f->tab_label);
   	text_edit_register_filename("Sin título", current_tab);
   	gtk_widget_show_all(GTK_WIDGET(notebook));
}

static void text_edit_select_font(GtkMenuItem *menu_item, gpointer notebook)
{
   	GtkWidget *font_dialog = gtk_font_selection_dialog_new("Seleccionar Fuente");
   	gchar *fontname;
   	gint id;

   	gtk_font_selection_dialog_set_preview_text(GTK_FONT_SELECTION_DIALOG(font_dialog), "abcdefghijk ABCDEFHIJK");

   	id = gtk_dialog_run(GTK_DIALOG(font_dialog));

   	switch (id)
    	{
      		case GTK_RESPONSE_OK:
      		case GTK_RESPONSE_APPLY:
         		fontname = gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(font_dialog));
         		desc = pango_font_description_from_string(fontname);
         		break;
      		case GTK_RESPONSE_CANCEL:
         		break;
    	}
   	gtk_widget_destroy(font_dialog);

   	text_edit_apply_font_selection(notebook);
}

static void text_edit_apply_font_selection(GtkNotebook *notebook)
{
   	GList *child_list;
   	gint pages;
   	gint i;
   	GtkWidget *swin;
   	pages = gtk_notebook_get_n_pages(notebook);

   	for (i = 0; i < pages; i++)
    	{
      		swin = gtk_notebook_get_nth_page(notebook, i);
      		child_list = gtk_container_get_children(GTK_CONTAINER(swin));
      		if (GTK_IS_TEXT_VIEW(child_list->data))
         		gtk_widget_modify_font(child_list->data, desc);
    	}
}

static void text_edit_open_file(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GList *child_list;
   	GtkWidget *dialog;
   	GtkWidget *save_dialog, *error_dialog;
   	gint current_page;
   	gint id;
   	gint offset;
   	gchar *filename;
   	gchar *contents;
   	GtkWidget *scrolled_win;
   	GtkWidget *view;
   	GtkTextBuffer *buffer;
   	GtkWidget *tab_name;
   	GtkTextIter start, end;
   	GtkTextMark *mark;
   	GtkWidget *prompt_label;
   	GtkWidget *content_area;

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);
   	child_list = gtk_container_get_children(GTK_CONTAINER(scrolled_win));
   	view = (GTK_IS_TEXT_VIEW(child_list->data) ? child_list->data : NULL);

   	if (view != NULL)
    	{
      		dialog = gtk_file_chooser_dialog_new("Abrir Archivo", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
      		id = gtk_dialog_run(GTK_DIALOG(dialog));
      		tab_name = gtk_notebook_get_tab_label(notebook, scrolled_win);

      		switch (id)
       		{
         		case GTK_RESPONSE_ACCEPT:
            			filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
                       		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
            			gtk_text_buffer_get_end_iter(buffer, &end);
            			offset = gtk_text_iter_get_offset(&end);
            			if (offset > 0)
             			{
               				save_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_NONE, NULL);
               				gtk_dialog_add_buttons(GTK_DIALOG(save_dialog), GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_DELETE, GTK_RESPONSE_CLOSE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
               				prompt_label = gtk_label_new("¿Guardar el contenido del búfer?");
               				content_area = gtk_dialog_get_content_area(GTK_DIALOG(save_dialog));
               				gtk_box_pack_start(GTK_BOX(content_area), prompt_label, FALSE, FALSE, 0);
               				gtk_widget_show_all(save_dialog);
               				gtk_widget_hide(dialog);
               				id = gtk_dialog_run(GTK_DIALOG(save_dialog));

               				switch (id)
                			{
                  				case GTK_RESPONSE_ACCEPT:
                     					text_edit_save_file(NULL, notebook);
                     					text_edit_register_filename(filename, current_page);
                     					break;
                  				case GTK_RESPONSE_CLOSE:
                     					gtk_text_buffer_get_bounds(buffer, &start, &end);
                     					gtk_text_buffer_delete(buffer, &start, &end);
                     					break;
                  				case GTK_RESPONSE_CANCEL:
                     					gtk_widget_destroy(save_dialog);
                     					return;
                			}
               				gtk_widget_destroy(save_dialog);
             			}

            			if (g_file_test(filename, G_FILE_TEST_EXISTS))
             			{
               				g_file_get_contents(filename, &contents, NULL, NULL);
               				mark = gtk_text_buffer_get_insert(buffer);
               				gtk_text_buffer_get_iter_at_mark(buffer, &start, mark);
               				gtk_text_buffer_set_text(buffer, contents, -1);
               				text_edit_register_filename(filename, current_page);
               				gtk_label_set_text(GTK_LABEL(tab_name), basename(filename));
             			}
            			else
             			{
                              		error_dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, NULL);
               				gtk_dialog_run(GTK_DIALOG(error_dialog));
               				gtk_widget_destroy(error_dialog);
             			}
            			break;
         		case GTK_RESPONSE_REJECT:
            			break;
       		}
      		gtk_widget_destroy(dialog);
    	}
}

static void text_edit_cut_to_clipboard(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GList *child_list;
   	GtkWidget *textview;
   	GtkTextBuffer *buffer;
   	GtkWidget *scrolled_win;
   	gint current_page;
   	GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);
   	child_list = gtk_container_get_children(GTK_CONTAINER(scrolled_win));
   	textview = child_list->data;

   	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
   	gtk_text_buffer_cut_clipboard(buffer, clipboard, TRUE);
}

static void text_edit_copy_to_clipboard(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GList *child_list;
   	GtkWidget *textview;
   	GtkTextBuffer *buffer;
   	GtkWidget *scrolled_win;
   	gint current_page;
   	GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);
   	child_list = gtk_container_get_children(GTK_CONTAINER(scrolled_win));
   	textview = child_list->data;

   	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
   	gtk_text_buffer_copy_clipboard(buffer, clipboard);
}

static void text_edit_paste_from_clipboard(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GList *child_list;
   	GtkWidget *textview;
   	GtkTextBuffer *buffer;
   	GtkWidget *scrolled_win;
   	gint current_page;
   	GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);
   	child_list = gtk_container_get_children(GTK_CONTAINER(scrolled_win));
   	textview = child_list->data;

   	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
   	gtk_text_buffer_paste_clipboard(buffer, clipboard, NULL, TRUE);
}

static void text_edit_show_about_dialog(GtkMenuItem *menu_item, GtkWindow *parent_window)
{
   	const gchar *authors[] = { "Glenn Schemenauer", "Héctor Porras", "Sergio Vargas", NULL};
   	gtk_show_about_dialog(parent_window, "program-name", "EZ-ARM", "authors", authors, "license", "GNU General Public License", "comments", "A simple lightweight GTK+ text editor/ARMv4 Assembler & Simulator", NULL);
}

static void text_edit_close_file(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GtkWidget *scrolled_win;
   	gint current_page;

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);

   	gtk_widget_destroy(scrolled_win); 
}

static void text_edit_save_file(GtkMenuItem *menu_item, GtkNotebook *notebook)
{
   	GList *child_list;
   	GtkWidget *dialog;
   	GtkWidget *textview;
   	GtkTextBuffer *buffer;
   	GtkWidget *scrolled_win;
   	gint current_page;
   	gint response;
   	GtkWidget *tab_label;
   	GtkTextIter start, end;
   	gchar *filename;
   	gchar *contents;

   	current_page = gtk_notebook_get_current_page(notebook);
   	scrolled_win = gtk_notebook_get_nth_page(notebook, current_page);
   	child_list = gtk_container_get_children(GTK_CONTAINER(scrolled_win));
   	textview = child_list->data;
   	tab_label = gtk_notebook_get_tab_label(notebook, scrolled_win);

   	if (strcmp(gtk_label_get_text(GTK_LABEL(tab_label)), "Sin título") == 0)
    	{
      		dialog = gtk_file_chooser_dialog_new("Guardar Archivo", NULL, GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_APPLY, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
      		response = gtk_dialog_run(GTK_DIALOG(dialog));

      		if (response == GTK_RESPONSE_APPLY)
       		{
         		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
         		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
         		gtk_text_buffer_get_bounds(buffer, &start, &end);
         		contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
         		g_file_set_contents(filename, contents, -1, NULL);
         		text_edit_register_filename(filename, current_page); 
         		gtk_label_set_text(GTK_LABEL(tab_label), basename(filename));
       		}
      		else if (response == GTK_RESPONSE_CANCEL)
       		{
         		gtk_widget_destroy(dialog);
         		return;
       		}
      		gtk_widget_destroy(dialog);
    	}
   	else
    	{
            	filename = text_edit_get_filename(current_page);
      		buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
      		gtk_text_buffer_get_bounds(buffer, &start, &end);
      		contents = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);
      		g_file_set_contents(filename, contents, -1, NULL);
    	}
}

static void text_edit_register_filename(gchar *fname, gint tab_num)
{
   	gint found = FALSE;
   	FileData *f = g_new(FileData, 1);
   	GList *node = g_list_alloc();
   	f->filename = fname;
   	f->tab_number = tab_num;
   	node->data = f;

   	if (filename_data == NULL) 
		filename_data = node;   
   	else
    	{
            	GList *list = filename_data;
      		while (list != NULL)
       		{
         		if (((FileData *) list->data)->tab_number == tab_num)
          		{
            			found = TRUE;
            			((FileData *) list->data)->filename = fname;
            			break;
          		}
         		else 
				list = g_list_next(list);
       		}
      		if (!found) 
			g_list_append(filename_data, node);
    	}
}

static gchar *text_edit_get_filename(gint tab_num)
{
   	GList *list = filename_data;
   	while (list != NULL)
    	{
      		if (((FileData *) list->data)->tab_number == tab_num)
         		return ((FileData *) list->data)->filename;
      		else
         		list = g_list_next(filename_data);
    	}

   	return NULL;
}
