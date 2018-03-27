#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>

int blocking_constant; //0: non-blocking, 1: blocking
int channels_linked;	//0: left and right channels arent linked, 1: linked
int output_descriptor;
int childid;
int num_files = 0;
char **loaded_files;
char *hevexecutablepath = "~/hev/idea/src/hev-playAudioFiles/hev-playAudioFiles";

GtkWidget *blockingTBTN;
GtkWidget *faderlRBTN, *fadelrRBTN, *fadeoffRBTN;
GtkWidget *lchannelSBTN, *rchannelSBTN;
GtkWidget *fselectCOMBO;
GList *file_list; //the list of loaded files

void execute_hevPAF();
void print_loaded_files();
void play_selected_file(int);

//--- Handler Methods ---

/* Kills the program */
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data){

	kill(childid,SIGKILL);

	gtk_main_quit();
	return FALSE;
}

/* prints the selected file from the combo box */
static void printselection(GtkWidget *widget, gpointer data){

	if(num_files > 0){
		char *selectedwav;
	
		selectedwav = gtk_entry_get_text(GTK_ENTRY (GTK_COMBO(data)->entry));
				
		//determine the index of the selected file
		int filenum = -1;
		int i = 0;
		while(filenum == -1){
			if(strcmp(selectedwav, loaded_files[i]) == 0)
				filenum = i;
		
			i++;
		}
		
		play_selected_file(filenum);
	}
}

/* checks whether blocking is enabled or disabled */
static void check_blocking_state(GtkWidget *widget, gpointer data){

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
		blocking_constant = 1;
	}
	else{
		blocking_constant = 0;
	}	
	
	//toggle the blocking
	FILE* out = fdopen(output_descriptor,"w");
	fprintf(out,"-b\n");
	fflush(out);

}

/* updates the state of the left and right channels link */
static void toggle_inverse_link(GtkWidget *widget, gpointer data){	

	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))){
		channels_linked = 1;
	}
	else{
		channels_linked = 0;
	}	
	
}
 /* sets the value of the other channel to the inverse of the value of the calling object */
static void set_inverse_channel_value(GtkWidget *widget, GtkSpinButton *otherSBTN){

	if(channels_linked == 1){
		
		gdouble value = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
		value = 1 - value;
		
		channels_linked = 0;
		gtk_spin_button_set_value(otherSBTN, value);
		channels_linked = 1;
	}

}

/* adds a file name from the input area to the combo box */
static void add_input_file(GtkEntry *widget, gpointer data){
	
	int len = strlen(gtk_entry_get_text(widget));
	
	
	//create a copy of the entry
	char *temp;
	temp = (char *)malloc(sizeof(char)*len);
	
	strcpy(temp, gtk_entry_get_text(widget));
	
	gtk_entry_set_text(widget, "");
	
	//store the copy in the array and g_list
	num_files++;
	
	loaded_files = (char **)realloc(loaded_files, num_files*sizeof(char *));
	loaded_files[num_files-1] = temp;

	file_list = g_list_append(file_list, temp);
	
	//update the combo box
	gtk_combo_set_popdown_strings (GTK_COMBO(fselectCOMBO), file_list);

	
	if(childid == 0){
		execute_hevPAF();
	}
	else{
		FILE* out = fdopen(output_descriptor,"w");
		fprintf(out,"-a %s\n", temp);
		fflush(out);
	}
}

static void save_current_state(GtkWidget *widget, gpointer data){
	
	FILE *output_pointer = fopen("config","w");
	
	//wrote the loaded files
	fprintf(output_pointer, "%d, ", num_files);

	int i;
	for(i = 0; i < num_files; i++){
		fprintf(output_pointer,"%s, ",loaded_files[i]);
	}
	fprintf(output_pointer,"\n");
	
	//write the blocking mode
	fprintf(output_pointer, "%d\n", blocking_constant);
	
	//write out the channel controls
	double left_percent = gtk_spin_button_get_value(GTK_SPIN_BUTTON(lchannelSBTN));
	double right_percent = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rchannelSBTN));
	
	fprintf(output_pointer, "%f %f", left_percent, right_percent);
	
	//write the fade controls
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(faderlRBTN))){
		fprintf(output_pointer, " -f");
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fadelrRBTN))){
		fprintf(output_pointer, " +f");
	}
	
	fprintf(output_pointer,"\n");
	
	fclose(output_pointer);
}

static void load_state_from_file(GtkWidget *widget, gpointer data){
	
	FILE *input_pointer = fopen("config","r");
	
	char* line = (char*)malloc(sizeof(char*));
	size_t len = 0;
	
	//read in the line of file names
	getline(&line, &len, input_pointer);
	
	char* word = strtok(line, ", ");
	num_files = atoi(word);
	
	loaded_files = (char **)malloc(sizeof(char *)*num_files);
	int i;
	file_list = NULL;
	for(i = 0; i < num_files; i++){
		word = strtok(NULL, ", ");
		loaded_files[i] = (char *)malloc(sizeof(char)*strlen(word));
		strcpy(loaded_files[i], word);
		file_list = g_list_append(file_list, loaded_files[i]);
	}
	gtk_combo_set_popdown_strings (GTK_COMBO(fselectCOMBO), file_list);

	//blocking	
	getline(&line, &len, input_pointer);
	blocking_constant = atoi(line);
	if(blocking_constant == 1){
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blockingTBTN), TRUE);
	}
	else{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blockingTBTN), FALSE);
	}
	
	//channel control
	getline(&line, &len, input_pointer);
	word = strtok(line, " ");
	
	int temp = channels_linked;
	channels_linked = 0;
	
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(lchannelSBTN), atof(word));
	word = strtok(NULL, " ");
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(rchannelSBTN), atof(word));

	channels_linked = temp;

	//fading
	word = strtok(NULL, " ");
	if(word != NULL){
		if(strcmp(word, "-f") == 0){
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(faderlRBTN), TRUE);
		}
		else{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fadelrRBTN), TRUE);
		}
	}
	

	
	execute_hevPAF();
}

int main(int argc, char **argv){
	
	gtk_init(&argc, &argv);
	
	GtkWidget *window;
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL); //creates the window
	g_signal_connect(window, "delete-event", G_CALLBACK(delete_event), NULL);
	GtkTooltips *tooltips;	
	tooltips = gtk_tooltips_new();
		
	GtkWidget *frame = gtk_vbox_new(FALSE, 0);
	gtk_widget_show(frame);
	
	//--- MENUBAR SETUP ---
	GtkWidget *menu_bar, *file_item, *file_menu, *save_item, *load_item;
	menu_bar = gtk_menu_bar_new();
	gtk_box_pack_start(GTK_BOX(frame), menu_bar, FALSE, FALSE, 2);
	gtk_widget_show(menu_bar);
	
	//menus
	file_menu = gtk_menu_new();
	
	//items
	file_item = gtk_menu_item_new_with_label("File");
	save_item = gtk_menu_item_new_with_label("Save");
	load_item = gtk_menu_item_new_with_label("Load");
	
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu),save_item);
	gtk_menu_shell_append(GTK_MENU_SHELL(file_menu),load_item);
	
	gtk_widget_show(file_item);
	gtk_widget_show(save_item);
	gtk_widget_show(load_item);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
	
	gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), file_item);
	
	//--- INITIALIZATION ---
	
	//blocking toggle button (TBTN)
	blockingTBTN = gtk_toggle_button_new_with_label("Blocking");

	gtk_tooltips_set_tip(tooltips, blockingTBTN, "Toggle the overlap of sound clips", NULL);
	
	gtk_widget_show(blockingTBTN);
	
	//play button (BTN)
	GtkWidget *playBTN;
	playBTN = gtk_button_new_with_label("Play");
	
	gtk_widget_show(playBTN);
	
	//left and right channel spin buttons (SBTN)
	GtkObject *ladjust = gtk_adjustment_new(1,0,1,0.1,0.01,0);
	GtkObject *radjust = gtk_adjustment_new(1,0,1,0.1,0.01,0);
	lchannelSBTN = gtk_spin_button_new(GTK_ADJUSTMENT(ladjust),0.5,2);
	rchannelSBTN = gtk_spin_button_new(GTK_ADJUSTMENT(radjust),0.5,2);
	
	GtkWidget *lchannelLABEL, *rchannelLABEL, *lchannelBOX, *rchannelBOX;
	lchannelLABEL = gtk_label_new("Left: ");
	rchannelLABEL = gtk_label_new("Right: ");
	lchannelBOX = gtk_hbox_new(FALSE, 5);
	rchannelBOX = gtk_hbox_new(FALSE, 5);
	
	gtk_box_pack_start(GTK_BOX(lchannelBOX), lchannelLABEL, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(lchannelBOX), lchannelSBTN, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(rchannelBOX), rchannelLABEL, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX(rchannelBOX), rchannelSBTN, FALSE, FALSE, 5);
	
	gtk_tooltips_set_tip(tooltips, lchannelBOX, "The volume percentage of the left channel, represented as a decimal.", NULL);
	gtk_tooltips_set_tip(tooltips, rchannelBOX, "The volume percentage of the right channel, represented as a decimal.", NULL);
	
	gtk_widget_show(lchannelSBTN);
	gtk_widget_show(rchannelSBTN);
	gtk_widget_show(lchannelLABEL);
	gtk_widget_show(rchannelLABEL);
	gtk_widget_show(lchannelBOX);
	gtk_widget_show(rchannelBOX);
	
	//inverse link check button (CBTN)
	GtkWidget *linkedCBTN;
	linkedCBTN = gtk_check_button_new_with_label("Inverse Channel Link");
	gtk_tooltips_set_tip(tooltips, linkedCBTN, "Inversely link the left and right channels, ie.: Right = .643, then Left = 1 - 0.643 = 0.357", NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(linkedCBTN), TRUE);
	channels_linked = 1;
	
	gtk_widget_show(linkedCBTN);
	
	//fade radio buttons (RBTN)
	faderlRBTN = gtk_radio_button_new_with_label(NULL, "Fade - Right to Left");
	fadelrRBTN = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(faderlRBTN), "Fade - Left to Right");
	fadeoffRBTN = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(fadelrRBTN), "Fade - None");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(fadeoffRBTN), TRUE);
	
	gtk_widget_show(faderlRBTN);
	gtk_widget_show(fadelrRBTN);
	gtk_widget_show(fadeoffRBTN);
	
	//file select combo box (COMBO)
	fselectCOMBO = gtk_combo_new();
	loaded_files = (char **)malloc(sizeof(char *));
		
	gtk_combo_set_popdown_strings(GTK_COMBO(fselectCOMBO), file_list);
	gtk_combo_set_use_arrows(GTK_COMBO(fselectCOMBO), TRUE);
	
	gtk_widget_show(fselectCOMBO);
	
	//file input text area (TXT)
	GtkWidget *finputTXT;
	finputTXT = gtk_entry_new();
	
	gtk_widget_show(finputTXT);
		
	//--- PACKING ---
	GtkWidget *table;
	table = gtk_table_new(4,4,FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table),10);
	gtk_table_set_col_spacings(GTK_TABLE(table),10);
	
	gtk_table_attach_defaults(GTK_TABLE(table), finputTXT, 0,4,0,1);
	gtk_table_attach_defaults(GTK_TABLE(table), fselectCOMBO, 0,3,1,2);
	gtk_table_attach_defaults(GTK_TABLE(table), playBTN, 3,4,1,2);
	gtk_table_attach_defaults(GTK_TABLE(table), linkedCBTN, 0,2,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table), lchannelBOX, 2,3,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table), rchannelBOX, 3,4,2,3);
	gtk_table_attach_defaults(GTK_TABLE(table), faderlRBTN, 0,1,3,4);
	gtk_table_attach_defaults(GTK_TABLE(table), fadelrRBTN, 1,2,3,4);
	gtk_table_attach_defaults(GTK_TABLE(table), fadeoffRBTN, 2,3,3,4);
	gtk_table_attach_defaults(GTK_TABLE(table), blockingTBTN, 3,4,3,4);
	
	gtk_box_pack_end(GTK_BOX(frame), table, FALSE, FALSE, 2);
	gtk_widget_show(table);
	
	//--- SIGNAL CONNECTIONS ---
	g_signal_connect(blockingTBTN, "clicked", G_CALLBACK(check_blocking_state), NULL);
	g_signal_connect(playBTN, "clicked", G_CALLBACK(printselection), fselectCOMBO);
	g_signal_connect(lchannelSBTN, "value_changed", G_CALLBACK(set_inverse_channel_value), rchannelSBTN);
	g_signal_connect(rchannelSBTN, "value_changed", G_CALLBACK(set_inverse_channel_value), lchannelSBTN);
	g_signal_connect(linkedCBTN, "clicked", G_CALLBACK(toggle_inverse_link), NULL);
	g_signal_connect(finputTXT, "activate", G_CALLBACK(add_input_file), NULL);
	g_signal_connect(save_item, "activate", G_CALLBACK(save_current_state), NULL);
	g_signal_connect(load_item, "activate", G_CALLBACK(load_state_from_file), NULL);
	
	gtk_container_add(GTK_CONTAINER(window), frame);
	gtk_widget_show(window);
	
	gtk_main();
		
	return 0;
	
}

void execute_hevPAF(){
	
	//kill any child process already running
	if(childid != 0){
		kill(childid,SIGKILL);
	}
	
	int fd[2];
	pipe(fd);
	
	childid = fork();
	
	if(childid == 0){
		
		//reroute standard input to the pipe
		close(0);
		close(fd[1]);
		dup(fd[0]);
		
		int len;
		
		//blocking mode enabled
		if(blocking_constant == 1){
			len = num_files + 3;
		}
		else{
			len = num_files + 2;
		}
		
		//create an array of command line args
		char** args = malloc(sizeof(char *) * len);
		
		args[0] = "hev-playAudioFiles";
		args[len-1] = NULL;

		if(blocking_constant == 1){
			args[1] = "-b";
		}
		
		//transfer the loaded files into the command args array
		int i;
		int buffer = 1+blocking_constant;
		
		for(i = buffer; i < num_files+buffer; i++){
			args[i] = loaded_files[i-buffer];
		}
		
		execvp("hev-playAudioFiles", args);
		printf("failed to execute\n");
	}

	output_descriptor = fd[1];	
}

void play_selected_file(int file_num){
	
	double left_percent = gtk_spin_button_get_value(GTK_SPIN_BUTTON(lchannelSBTN));
	double right_percent = gtk_spin_button_get_value(GTK_SPIN_BUTTON(rchannelSBTN));
	
	//write the index across the pipe to the hev-playAudioFiles process
	FILE* out = fdopen(output_descriptor,"w");
	
	fprintf(out, "%d", file_num);
	
	//write the left and right channel controls
	if(left_percent != 1 || right_percent != 1){
		
		fprintf(out, " %f %f", left_percent, right_percent);
		
	}
	else{
		fprintf(out, " 1 1");
	}
	
	//write the fade controls
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(faderlRBTN))){
		fprintf(out, " -f");
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fadelrRBTN))){
		fprintf(out, " +f");
	}
	
	fprintf(out, "\n");
	fflush(out);
}




