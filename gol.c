// https://www.cairographics.org/threaded_animation_with_cairo/
// https://stackoverflow.com/questions/57699050/how-to-create-a-cairo-object-within-a-gtk-window-in-gtk3


    #include <gtk/gtk.h>
    #include <cairo.h>
    #include <omp.h>

    //#define CONSOLENPRINT 1
    #define FASTVERSION 1
    #define PARALLEL 1

    #define PIXELWIDTH 20
    #define PIXELGAP 2
    #define UPPERGRIDMARGIN 60
    #define LEFTGRIDMARGIN 10

    #define ALIVE 0x01
    #define NEXTSTATE 0x02

    #define GRID_X_SIZE 1000
    #define GRID_Y_SIZE 1000

#ifdef FASTVERSION
    char grid[GRID_X_SIZE+2][GRID_Y_SIZE+2];
#else
    char grid[GRID_X_SIZE][GRID_Y_SIZE];
#endif    
    int genCounter=0;

    void printgrid();

    int zaehleNachbarn();
    int pruefeNachbar();

    int zaehleNachbarnFast();

    void naechsteGeneration();

    void naechsteGenerationFast();

    void blinker();
    void on_btnNextGen_clicked(GtkButton *btn, gpointer data);
    void on_drawing_clicked(GtkWidget *da,GdkEventButton *event, gpointer data);


    void on_btnStart_clicked(GtkButton *btn, gpointer data);
    void aktualisiereLabel(gchar *timerString);
    


    GtkDrawingArea* drawingArea; 
    GtkWidget *lblGenCount; 
    GtkWidget *lblTimerCount;    
    GtkWidget *entryGrenze;


    // ------------------------------------------------------------

    gboolean on_draw (GtkWidget *widget,
                    GdkEventExpose *event,
                    gpointer data) 
    {

        // "convert" the G*t*kWidget to G*d*kWindow (no, it's not a GtkWindow!)
        GdkWindow* window = gtk_widget_get_window(widget);  


        cairo_region_t * cairoRegion = cairo_region_create();

        GdkDrawingContext * drawingContext;
        drawingContext = gdk_window_begin_draw_frame (window,cairoRegion);

        { 
            cairo_t * cr = gdk_drawing_context_get_cairo_context (drawingContext);

            { 
                cairo_move_to(cr, 10, 30);
                cairo_set_font_size(cr,15);
                cairo_show_text(cr, "Game of Life");
                cairo_move_to(cr, 30, 60);
                cairo_set_line_width(cr, PIXELWIDTH);
#ifdef FASTVERSION
                for (int xx=1; xx<GRID_X_SIZE+1; xx++) {
                    for (int yy=1; yy<GRID_Y_SIZE+1; yy++) {
#else
                for (int xx=0; xx<GRID_X_SIZE; xx++) {
                    for (int yy=0; yy<GRID_Y_SIZE; yy++) {
#endif
                        if (grid[xx][yy] & ALIVE)
                            cairo_set_source_rgb(cr, 1.0, 0, 0);
                        else
                            cairo_set_source_rgb(cr, 0, 0, 1.0);
                        cairo_move_to(cr, LEFTGRIDMARGIN+xx*PIXELWIDTH+xx*PIXELGAP, 
                            UPPERGRIDMARGIN+yy*PIXELWIDTH+yy*PIXELGAP);
                        cairo_line_to(cr, LEFTGRIDMARGIN+xx*PIXELWIDTH+xx*PIXELGAP+PIXELWIDTH, 
                            UPPERGRIDMARGIN+yy*PIXELWIDTH+yy*PIXELGAP);
                        cairo_stroke(cr);
                    }

                }  
                            
            }

            gdk_window_end_draw_frame(window,drawingContext);
        }

        // cleanup
        cairo_region_destroy(cairoRegion);

        return FALSE;
    }

    // ------------------------------------------------------------

    int main (int argc, char * argv[]) {
        
        gtk_init(&argc, &argv);


        // zum Testen ein paar Pixel setzen
        blinker();
                

        printgrid();

        GtkWidget *btnNextGen = gtk_button_new_with_label("Next Generation");
        GtkWidget *lblGenerations = gtk_label_new("Generation:");
        lblGenCount = gtk_label_new("0");
        GtkWidget *lblTimer = gtk_label_new("Sekunden:");
        lblTimerCount = gtk_label_new("0");
        GtkWidget *layout;

        GtkWidget *lblGrenze, *entryGrenze;
        lblGrenze = gtk_label_new("Grenze: ");
        entryGrenze = gtk_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(entryGrenze), "");

        GtkWidget *btnStart = gtk_button_new_with_label("Start");



        GtkWindow * window;         
        window = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_default_size (window, 600, 450);
        gtk_window_set_position     (window, GTK_WIN_POS_CENTER);
        gtk_window_set_title        (window, "Game of Life - Control");
        g_signal_connect(window, "destroy", gtk_main_quit, NULL);


        GtkWindow * drawWindow; 
        
        drawWindow = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
#ifdef FASTVERSION
        gtk_window_set_default_size (drawWindow, 395, 450);
#else
        gtk_window_set_default_size (drawWindow, 350, 450);
#endif
        gtk_window_set_position     (drawWindow, GTK_WIN_POS_CENTER);
        gtk_window_set_title        (drawWindow, "Game of Life - Ausgabe");
        g_signal_connect(drawWindow, "destroy", gtk_main_quit, NULL);

        // Zeichenfläche erstellen    
        drawingArea = (GtkDrawingArea*) gtk_drawing_area_new();
        gtk_container_add(GTK_CONTAINER(drawWindow), (GtkWidget*)drawingArea);

        layout = gtk_layout_new(NULL,NULL); 
        gtk_container_add(GTK_CONTAINER(window), layout);

        gtk_container_add(GTK_CONTAINER(layout), btnNextGen);
        gtk_layout_move(GTK_LAYOUT(layout), btnNextGen, 90, 200);

        gtk_container_add(GTK_CONTAINER(layout),lblGenerations);
        gtk_layout_move(GTK_LAYOUT(layout), lblGenerations, 90, 10);
        gtk_container_add(GTK_CONTAINER(layout),lblGenCount);
        gtk_layout_move(GTK_LAYOUT(layout), lblGenCount, 200, 10);

        gtk_container_add(GTK_CONTAINER(layout),lblTimer);
        gtk_layout_move(GTK_LAYOUT(layout), lblTimer, 90, 50);
        gtk_container_add(GTK_CONTAINER(layout),lblTimerCount);
        gtk_layout_move(GTK_LAYOUT(layout), lblTimerCount, 200, 50);

        gtk_container_add(GTK_CONTAINER(layout),lblGrenze);
        gtk_layout_move(GTK_LAYOUT(layout), lblGrenze, 90, 130);
        gtk_container_add(GTK_CONTAINER(layout),entryGrenze);
        gtk_layout_move(GTK_LAYOUT(layout), entryGrenze, 200, 130);

        gtk_container_add(GTK_CONTAINER(layout), btnStart);
        gtk_layout_move(GTK_LAYOUT(layout), btnStart, 90, 170);
    
        g_signal_connect((GtkWidget*)drawingArea, "draw", G_CALLBACK(on_draw), NULL); 

        g_signal_connect((GtkWidget*)btnNextGen, "clicked", G_CALLBACK(on_btnNextGen_clicked), NULL);
        g_signal_connect((GtkWidget*)drawWindow, "button_press_event",G_CALLBACK(on_drawing_clicked),NULL);

        g_signal_connect((GtkWidget*)btnStart, "clicked", G_CALLBACK(on_btnStart_clicked), entryGrenze);

        gtk_widget_show_all ((GtkWidget*)window);
        gtk_widget_show_all ((GtkWidget*)drawWindow);
        gtk_main();

        return 0;
    }

    // -------------------REGELN---------------------------------
    //Ausgabe Konsole für Testzwecke
    void printgrid() {
#ifdef CONSOLENPRINT              
        printf("    0 1 2 3 4 5 6 7 8 9 0 1 2 3\n");
#ifdef FASTVERSION
        for (int yy=0; yy<GRID_Y_SIZE+2; yy++) {
#else        
        for (int yy=0; yy<GRID_Y_SIZE; yy++) {
#endif            
            printf("[%2d]",yy);
#ifdef FASTVERSION
            for (int xx=0; xx<GRID_X_SIZE+2; xx++) {
#else
            for (int xx=0; xx<GRID_X_SIZE; xx++) {
#endif            
                printf("%d ",grid[xx][yy]);
            }
            printf("\n");            
        }
        printf("-------------------------\n");
#endif        
        return;
    }

    void naechsteGeneration() {        
        int zaehler=0;
        gchar *counterString;
        genCounter++;
        counterString = g_strdup_printf("%d",genCounter);
        gtk_label_set_text(GTK_LABEL(lblGenCount), counterString);
        #pragma omp parallel for
        for (int x=0; x<GRID_X_SIZE; x++) {
            for (int y=0; y<GRID_Y_SIZE; y++) {
                zaehler = zaehleNachbarn(x,y);

                //tote Zellen mit 3 lebenden Nachbarn werden neu geboren
                if ((zaehler == 3) && !(grid[x][y] & ALIVE)) {
                    grid[x][y] |= NEXTSTATE;
                }  

                //stirbt, wenn weniger als 2 lebende Nachbarn
                if (zaehler <2) {
                    grid[x][y] &= ~NEXTSTATE;
                }           

                //lebende mit 2 oder 3 lebenden Nachbarn bleiben am Leben
                if ((grid[x][y] & ALIVE) && (zaehler==2 || zaehler==3)) {
                    grid[x][y] |= NEXTSTATE;
                } 

                //mehr als 3 lebende Nachbarn -> stirbt an Überbevölkerung
                if (zaehler > 3) {
                    grid[x][y] &= ~NEXTSTATE;
                }
            }
        }

        // NEXTSTATE auf ALIVE shiften, damit es gezeichnet werden kann
        #pragma omp parallel for
        for (int x=0; x<GRID_X_SIZE; x++) {
            for (int y=0; y<GRID_Y_SIZE; y++) {
               grid[x][y] >>= 1;
            }
        }
        printgrid();

    }
    

    int zaehleNachbarn (int x, int y) {
        int zaehler = 0;

        zaehler += pruefeNachbar (x-1, y);
        zaehler += pruefeNachbar (x, y-1);
        zaehler += pruefeNachbar (x+1, y);
        zaehler += pruefeNachbar (x, y+1);

        zaehler += pruefeNachbar (x-1, y-1);
        zaehler += pruefeNachbar (x+1, y-1);
        zaehler += pruefeNachbar (x-1, y+1);
        zaehler += pruefeNachbar (x+1, y+1);

        return(zaehler);

    }

    int pruefeNachbar (int nachbarX, int nachbarY) {
        if ((nachbarX < 0) || (nachbarX > GRID_X_SIZE-1)) 
            return (0);
        if ((nachbarY < 0) || (nachbarY > GRID_Y_SIZE-1))
            return (0);
        if (grid[nachbarX][nachbarY] & ALIVE) 
            return(1);        
        return (0);
    }

    //---------------------------VERBESSERTE ROUTINE-----------------------------------------
    //Zähle Nachbarn fast
    void naechsteGenerationFast() {        
        int zaehler=0;
        gchar *counterString;
        genCounter++;
        counterString = g_strdup_printf("%d",genCounter);
        gtk_label_set_text(GTK_LABEL(lblGenCount), counterString);
#ifdef PARALLEL
        #pragma omp parallel for simd
#endif
        for (int x=1; x<GRID_X_SIZE+1; x++) {
            for (int y=1; y<GRID_Y_SIZE+1; y++) {
                zaehler = zaehleNachbarnFast(x,y);

                //tote Zellen mit 3 lebenden Nachbarn werden neu geboren
                if ((zaehler == 3) && !(grid[x][y] & ALIVE)) {
                    grid[x][y] |= NEXTSTATE;
                }  

                //stirbt, wenn weniger als 2 lebende Nachbarn
                if (zaehler <2) {
                    grid[x][y] &= ~NEXTSTATE;
                }           

                //lebende mit 2 oder 3 lebenden Nachbarn bleiben am Leben
                if ((grid[x][y] & ALIVE) && (zaehler==2 || zaehler==3)) {
                    grid[x][y] |= NEXTSTATE;
                } 

                //mehr als 3 lebende Nachbarn -> stirbt an Überbevölkerung
                if (zaehler > 3) {
                    grid[x][y] &= ~NEXTSTATE;
                }
            }
        }

        // NEXTSTATE auf ALIVE shiften, damit es gezeichnet werden kann
#ifdef PARALLEL
        #pragma omp parallel for simd
#endif
        for (int x=1; x<GRID_X_SIZE+1; x++) {
            for (int y=1; y<GRID_Y_SIZE+1; y++) {
               grid[x][y] >>= 1;
            }
        }
        printgrid();

    }
    

    int zaehleNachbarnFast (int x, int y) {
        return(
            (grid[x-1][y] & ALIVE)  +
            (grid[x][y-1] & ALIVE)  +
            (grid[x+1][y] & ALIVE)  +
            (grid[x][y+1] & ALIVE)  +
            (grid[x-1][y-1] & ALIVE)  +
            (grid[x+1][y-1] & ALIVE)  +
            (grid[x-1][y+1] & ALIVE)  +
            (grid[x+1][y+1] & ALIVE));        
    }
  

    // -------------------OBJEKTE----------------------------------

    void blinker() {
        grid[7][3] |= ALIVE;
        grid[7][4] |= ALIVE;
        grid[7][5] |= ALIVE;
    }
 
    // -------------------EVENTS----------------------------------

    void on_btnNextGen_clicked(GtkButton *btn, gpointer data){
#ifdef FASTVERSION
        naechsteGenerationFast();
#else
        naechsteGeneration();
#endif
        gtk_widget_queue_draw((GtkWidget*)drawingArea);
    }

    void on_btnStart_clicked(GtkButton *btn, gpointer data){
        double start;
        double end;
        const gchar *entryData;
        gchar *timerString;
        genCounter=0;
        entryData = gtk_entry_get_text (GTK_ENTRY(data));
        
        int grenze = atoi(entryData);
        printf("%d\n",grenze);
        start = omp_get_wtime();
        for (int i=0; i<grenze; i++) {
#ifdef FASTVERSION
            naechsteGenerationFast();
#else            
            naechsteGeneration();
#endif            
        }

        end = omp_get_wtime();  
        timerString = g_strdup_printf("%f",end-start);  
        gtk_label_set_text(GTK_LABEL(lblTimerCount), timerString);           
        printf("%f Sekunde\n ", end-start);
        gtk_widget_queue_draw((GtkWidget*)drawingArea);
    }

    //Funktion, um Zellen zu per Mausklick zu aktivieren
    void on_drawing_clicked(GtkWidget *da,GdkEventButton *event, gpointer data) 
    {

        int halfLineWide = PIXELWIDTH / 2;
        int recX = 0, recY = 0;
        int eventX = (int)event->x;
        int eventY = (int)event->y;

#ifdef FASTVERSION
        for (int xx = 0; xx < GRID_X_SIZE+2; xx++)
#else
        for (int xx = 0; xx < GRID_X_SIZE; xx++)
#endif        
        {
#ifdef FASTVERSION
            for (int yy = 0; yy < GRID_Y_SIZE+2; yy++)
#else            
            for (int yy = 0; yy < GRID_Y_SIZE; yy++)
#endif            
            {
                recX = LEFTGRIDMARGIN + xx * PIXELWIDTH + xx * PIXELGAP;
                recY = UPPERGRIDMARGIN + yy * PIXELWIDTH + yy * PIXELGAP - halfLineWide;
                if ((eventX >= recX) && (eventX <= (recX + PIXELWIDTH)) &&
                    (eventY >= recY) && (eventY <= (recY + PIXELWIDTH)))
                {
                    grid[xx][yy] ^= ALIVE;
                    gtk_widget_queue_draw((GtkWidget *)drawingArea);
                }
            }
        }
    }



