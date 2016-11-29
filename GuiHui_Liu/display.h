#ifndef DISPLAY_H
#define DISPLAY_H

void plot( screen s, color c, int x, int y,double z, z_buff z_buf);
void clear_screen( screen s, z_buff z_buf);
void save_ppm( screen s, char *file);
void save_extension( screen s, char *file);
void display( screen s);

color change_color( int i );
#endif
