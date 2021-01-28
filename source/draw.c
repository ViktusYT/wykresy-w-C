#include "draw.h"

static void put_pixel (GdkPixbuf* pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha);
static void draw_rasterizaton(eqData* data, gdouble wyniki[], gint column, gdouble l, gdouble delta);

void draw_chart (GtkWidget *widget, eqData* data) 
{
    const gchar* wejscie, *interval, *scale_;
    gchar* eptr;
    gdouble l, p, delta, scale;
    gint r;

    wejscie = gtk_entry_get_text(GTK_ENTRY(data->equation));
    interval = gtk_entry_get_text(GTK_ENTRY(data->interval));
    scale_ = gtk_entry_get_text(GTK_ENTRY(data->res));

    if (strcmp(interval, "") == 0) l = 10.0;
    else l = strtod(interval, &eptr);

    p = l;
    l *= -1;

    gint lorg = l;

    r = (gint)800;
    delta = (p - l) / (gdouble)r;

    gdouble wyniki[r];
    gdouble max = -800.0;

    data->chartData = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, 800, 600);
    put_lines_to_chart(data->chartData, l, p);

    for (gint i = 0; i < r; i++)
    {
        bool stop = false;
        double wynik = calc(wejscie, strlen(wejscie), l, &stop);
        if (stop) return;

        wyniki[i] = wynik;

        if (!isnan(wynik) && wynik > max) max = wynik;
        l+=delta;
    }

    max += max * 0.4;
    if (max > 300) max = 300;

    if (strcmp(scale_, "") == 0) scale = 300 / max;
    else scale = strtod(scale_, &eptr);

    draw_make_legend(data->chartLegendLeft, (300/scale) / 5);
    draw_make_legend(data->chartLegendBottom, p / 5);

    for(gint i = 0; i < 10; i++)
    {
        //Problem w tym miejscu ->
        //GtkRequisition r;

        /*gint w;
        gtk_widget_get_preferred_width(data->chartLegendBottom[i], NULL, &w);

        gint width = 80 - w;
        gtk_widget_set_margin_end(GTK_WIDGET(data->chartLegendBottom[i]), width);*/
    }

    //draw and rasterization
    for (gint i = 0; i < r; i++) 
    {
        wyniki[i] = (gint)(300 - (wyniki[i]) * scale);

        if (!isnan(wyniki[i]) && wyniki[i] >=0 && wyniki[i] <= 600) put_pixel(data->chartData, i, wyniki[i], 0, 0, 255, 255); 

        if (i && !isnan(wyniki[i]) && data->rasterization)
            draw_rasterizaton(data, wyniki, i, lorg, delta);
    }
    
    gtk_image_set_from_pixbuf(GTK_IMAGE(data->chart), data->chartData);
}

static void put_pixel (GdkPixbuf* pixbuf, int x, int y, guchar red, guchar green, guchar blue, guchar alpha)
{
    int width, height, rowstride, n_channels;
    guchar *pixels, *p; 

    n_channels = gdk_pixbuf_get_n_channels (pixbuf);

    g_assert (gdk_pixbuf_get_colorspace(pixbuf) == GDK_COLORSPACE_RGB);
    g_assert (gdk_pixbuf_get_bits_per_sample(pixbuf) == 8);
    g_assert (gdk_pixbuf_get_has_alpha (pixbuf));
    g_assert (n_channels == 4);

    width = gdk_pixbuf_get_width (pixbuf);
    height = gdk_pixbuf_get_height (pixbuf);

    if (x < 0 || x >= width) return;
    if (y < 0 || y >= height) return;

    rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    pixels = gdk_pixbuf_get_pixels(pixbuf);

    p = pixels + y * rowstride + x * n_channels;
    p[0] = red;
    p[1] = green;
    p[2] = blue;
    p[3] = alpha;
}

static void draw_rasterizaton(eqData* data, gdouble wyniki[], gint column, gdouble l, gdouble delta)
{
    gint start = wyniki[column - 1] < wyniki[column] ? (gint)wyniki[column - 1] : (gint)wyniki[column];
    gint end = wyniki[column - 1] < wyniki[column] ? (gint)wyniki[column] : (gint)wyniki[column - 1];

    bool stopRast = false;
    gdouble sampling[2341];

    if (end - start >= 2 && end - start < 400 && data->microSampling)
    {
        const gchar* equation = gtk_entry_get_text(GTK_ENTRY(data->equation));

        for (gint i = 0; i <= 2340; i++)
        {
            bool stop = false;
            sampling[i] = calc(equation, strlen(equation), l + (delta*(gdouble)(column - 1)) + ((gdouble)(i+30)/2340 * delta), &stop);
        }

        for (gint i = 1; i <= 2340 && !stopRast; i++)
        {
            if (fabs(sampling[i - 1] - sampling[i]) > 0.5) 
                stopRast = true;
        }
    }

    if (end - start >= 2 && end - start < 400 && !stopRast)
    {
        for (gint i = start; i < end; i++)
        {
            put_pixel(data->chartData, column, i, 0, 0, 255, 255);
        }
    }
}

void put_lines_to_chart(GdkPixbuf* pixbuf, gdouble l, gdouble p)
{
    for (gint i = 0; i < 800; i++)
    {
        for (gint j = 0; j < 600; j++)
        {
            if (j % 60 == 0 && j) put_pixel(pixbuf, i, j, 0, 0, 0, 127);
            else if (j % 12 == 0 && j) put_pixel(pixbuf, i, j, 0, 0, 0, 63);
        }
    }

    for (gint i = 0; i < 600; i++)
    {
        for (gint j = 0; j < 800; j++)
        {
            if (j % 80 == 0 && j) put_pixel(pixbuf, j, i, 0, 0, 0, 127);
            else if (j % 16 == 0 && j) put_pixel(pixbuf, j, i, 0, 0, 0, 63);
        }
    }

    for (gint i = 0; i < 800; i++)
    {
        put_pixel(pixbuf, i, 299, 0, 0, 0, 255);
        put_pixel(pixbuf, i, 300, 0, 0, 0, 255);
    }

    for (gint i = 0; i < 600; i++)
    {
        put_pixel(pixbuf, 399, i, 0, 0, 0, 255);
        put_pixel(pixbuf, 400, i, 0, 0, 0, 255);
    }
}

void draw_make_legend(GtkWidget* chartLeft[], gdouble delta)
{
    for (gint i = 1; i <= 11; i++)
    {
        if (i < 6)
        {
            gchar str[20]; double_to_char((gdouble)((6 - i) * delta), 2, str);
            gtk_label_set_text(GTK_LABEL(chartLeft[i - 1]), str);
        }
        else if (i == 6)
        {
            gtk_label_set_text(GTK_LABEL(chartLeft[i - 1]), "0");
        }
        else
        {   
            gchar str[20]; double_to_char((gdouble)((i - 6) * delta), 2, str);
            gtk_label_set_text(GTK_LABEL(chartLeft[i - 1]), str);
        }
    }
}