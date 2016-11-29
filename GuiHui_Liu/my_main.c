/*========== my_main.c ==========

  This is the only file you need to modify in order
  to get a working mdl project (for now).

  my_main.c will serve as the interpreter for mdl.
  When an mdl script goes through a lexer and parser, 
  the resulting operations will be in the array op[].

  Your job is to go through each entry in op and perform
  the required action from the list below:

  frames: set num_frames (in misc_headers.h) for animation

  basename: set name (in misc_headers.h) for animation

  vary: manipluate knob values between two given frames
        over a specified interval

  set: set a knob to a given value
  
  setknobs: set all knobs to a given value

  push: push a new origin matrix onto the origin stack
  
  pop: remove the top matrix on the origin stack

  move/scale/rotate: create a transformation matrix 
                     based on the provided values, then 
		     multiply the current top of the
		     origins stack by it.

  box/sphere/torus: create a solid object based on the
                    provided values. Store that in a 
		    temporary matrix, multiply it by the
		    current top of the origins stack, then
		    call draw_polygons.

  line: create a line based on the provided values. Store 
        that in a temporary matrix, multiply it by the
	current top of the origins stack, then call draw_lines.

  save: call save_extension with the provided filename

  display: view the image live
  
  jdyrlandweaver
  =========================*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "parser.h"
#include "symtab.h"
#include "y.tab.h"

#include "misc_headers.h"
#include "matrix.h"
#include "ml6.h"
#include "display.h"
#include "draw.h"
#include "stack.h"

/*======== void first_pass()) ==========
  Inputs:   
  Returns: 

  Checks the op array for any animation commands
  (frames, basename, vary)
  
  Should set num_frames and basename if the frames 
  or basename commands are present

  If vary is found, but frames is not, the entire
  program should exit.

  If frames is found, but basename is not, set name
  to some default value, and print out a message
  with the name being used.
  
  05/17/12 09:27:02
  jdyrlandweaver
  ====================*/
void first_pass() {
  int i,f,b,v;
  f = 0;
  b = 0;
  v = 0;
  for( i = 0; i <lastop; i++){
    switch(op[i].opcode){
      case FRAMES:
	num_frames = op[i].op.frames.num_frames;
	f = 1;
	break;
      case BASENAME:
	strcpy(name,op[i].op.basename.p->name);
	b = 1;
      case VARY:
	v = 1;
	break;
    }
  }
  if( v && !f){
    printf("can't vary without frame\n");
    exit(0);
  }
  else if(v && f && !b){
    printf("base set to default\n");
    strcpy(name,"default");
  }
}
  
	

/*======== struct vary_node ** second_pass()) ==========
  Inputs:   
  Returns: An array of vary_node linked lists

  In order to set the knobs for animation, we need to keep
  a seaprate value for each knob for each frame. We can do
  this by using an array of linked lists. Each array index
  will correspond to a frame (eg. knobs[0] would be the first
  frame, knobs[2] would be the 3rd frame and so on).

  Each index should contain a linked list of vary_nodes, each
  node contains a knob name, a value, and a pointer to the
  next node.

  Go through the opcode array, and when you find vary, go 
  from knobs[0] to knobs[frames-1] and add (or modify) the
  vary_node corresponding to the given knob with the
  appropirate value. 

  05/17/12 09:29:31
  jdyrlandweaver
  ====================*/
struct vary_node ** second_pass() {
  int i,j,beg,end,v;
  double sval, eval, dif;
  struct vary_node* tmp;
  struct vary_node** knobs;
  knobs = (struct vary_node**)malloc(num_frames * (sizeof(struct vary_node*)));

  for(i = 0; i<lastop; i++){
    switch(op[i].opcode){
      case VARY:
	beg = op[i].op.vary.start_frame;
	end = op[i].op.vary.end_frame;
	sval = op[i].op.vary.start_val;
	eval = op[i].op.vary.end_val;
	dif = (eval - sval)/ (end - beg);
	for(j = beg; j<= end;j++){
	  if(!knobs[j]){
	    knobs[j] = (struct vary_node *)malloc(sizeof(struct vary_node));
	    strcpy(knobs[j] ->name, op[i].op.vary.p->name);
	    knobs[j] -> value = sval + (j - beg) * dif;
	  }
	  else {
	    tmp = knobs[j];
	    while(tmp->next)
	      tmp = tmp->next;
	    tmp->next = (struct vary_node *)malloc(sizeof(struct vary_node));
	    tmp = tmp->next;
	    strcpy(tmp->name,op[i].op.vary.p->name);
	    tmp->value = sval + (j - beg) * dif;
	  }
	}
	if(!v){
	  for(j =0; j<beg; j++){
	    if(!knobs[j])
	      knobs[j] = (struct vary_node *)malloc(sizeof(struct vary_node));
	    strcpy(knobs[j] ->name, op[i].op.vary.p->name);
	    knobs[j] -> value = sval;
	  }
	  for(j = end; j < num_frames; j++){
	    if(!knobs[j])
	      knobs[j] = (struct vary_node *)malloc(sizeof(struct vary_node));
	    strcpy(knobs[j]->name,op[i].op.vary.p->name);
	    knobs[j]->value = eval;
	  }
	  v++;
	}
	break;
    }
  }
  return knobs;
}

/*======== void print_knobs()) ==========
  Inputs:   
  Returns: 

  Goes through the symbol table and prints
  the values of all the knobs.

  05/15/13 09:29:17
  jdyrlandweaver
====================*/
void print_knobs() {
  
  int i;

  printf( "ID\tNAME\t\tTYPE\t\tVALUE\n" );
  for ( i=0; i < lastsym; i++ ) {

    if ( symtab[i].type == SYM_VALUE ) {
      printf( "%d\t%s\t\t", i, symtab[i].name );

      printf( "SYM_VALUE\t");
      printf( "%6.2f\n", symtab[i].s.value);
    }
  }
}

/*======== void my_main() ==========
  Inputs:   int polygons  
  Returns: 

  This is the main engine of the interpreter, it should
  handle most of the commadns in mdl.

  If frames is not present in the source (and therefore 
  num_frames is 1, then process_knobs should be called.

  If frames is present, the enitre op array must be
  applied frames time. At the end of each frame iteration
  save the current screen to a file named the
  provided basename plus a numeric string such that the
  files will be listed in order, then clear the screen and
  reset any other data structures that need it.

  Important note: you cannot just name your files in 
  regular sequence, like pic0, pic1, pic2, pic3... if that
  is done, then pic1, pic10, pic11... will come before pic2
  and so on. In order to keep things clear, add leading 0s
  to the numeric portion of the name. If you use sprintf, 
  you can use "%0xd" for this purpose. It will add at most
  x 0s in front of a number, if needed, so if used correctly,
  and x = 4, you would get numbers like 0001, 0002, 0011,
  0487

  05/17/12 09:41:35
  jdyrlandweaver
  ====================*/
void my_main( int polygons ) {

  int i, f, j,k;
  double step;
  double xval, yval, zval, knob_value;
  struct matrix *transform;
  struct matrix *tmp;
  struct stack *s;
  struct vary_node ** knobs;
  struct vary_node * temp;
  screen t;
  z_buff z_buf;
  color g;
  char q;
  char dir[256];
  char p[256];

  num_frames = 1;
  step = 0.05;
 
  g.red = 0;
  g.green = 255;
  g.blue = 255;

  first_pass();

  if(num_frames >1)
    knobs = second_pass();

  for(k = 0; k <num_frames; k++){
    s = new_stack();
    tmp = new_matrix(4,4);
    clear_screen(t,z_buf);
    
    for(j = 0; j < lastsym; j++){
      if(symtab[j].type == SYM_VALUE){
	temp = knobs[k];
	while(strcmp(temp->name,symtab[j].name) )
	  temp = temp->next;
	if(tmp)
	  (&symtab[j]) ->s.value = temp->value;
      }
    }

  for (i=0;i<lastop;i++) {
  
    switch (op[i].opcode) {
      
    case SET:
      f = op[i].op.set.p->s.value;
      set_value(lookup_symbol(op[i].op.set.p->name),f);
      break;
      
    case SETKNOBS:
      f = op[i].op.setknobs.value;
      for(j = 0; j <lastsym; j++){
	if(symtab[j].type == SYM_VALUE)
	  set_value(&(symtab[j]), f);
      }
      break;
	
    case SPHERE:
      add_sphere( tmp,op[i].op.sphere.d[0], //cx
		  op[i].op.sphere.d[1],  //cy
		  op[i].op.sphere.d[2],  //cz
		  op[i].op.sphere.r,
		  step);
      //apply the current top origin
      matrix_mult( s->data[ s->top ], tmp );
      draw_polygons( tmp, t, g,z_buf );
      tmp->lastcol = 0;
      break;

    case TORUS:
      add_torus( tmp, op[i].op.torus.d[0], //cx
		 op[i].op.torus.d[1],     //cy
		 op[i].op.torus.d[2],    //cz
		 op[i].op.torus.r0,
		 op[i].op.torus.r1,
		 step);
      matrix_mult( s->data[ s->top ], tmp );
      draw_polygons( tmp, t, g,z_buf );
      tmp->lastcol = 0;
      break;

    case BOX:
      add_box( tmp, op[i].op.box.d0[0],
	       op[i].op.box.d0[1],
	       op[i].op.box.d0[2],
	       op[i].op.box.d1[0],
	       op[i].op.box.d1[1],
	       op[i].op.box.d1[2]);
      matrix_mult( s->data[ s->top ], tmp );
      draw_polygons( tmp, t, g,z_buf );
      tmp->lastcol = 0;
      break;

    case LINE:
      add_edge( tmp, op[i].op.line.p0[0],
		op[i].op.line.p0[1],
		op[i].op.line.p0[1],
		op[i].op.line.p1[0],
		op[i].op.line.p1[1],
		op[i].op.line.p1[1]);
      draw_lines( tmp, t, g,z_buf );
      tmp->lastcol = 0;
      break;

    case MOVE:
      knob_value = 1;
      if(op[i].op.move.p){
	temp = knobs[k];
	while (strcmp(temp->name, op[i].op.move.p->name))
	  temp = temp ->next;
	if(temp)
	  knob_value = temp ->value;
      }
      //get the factors
      xval = op[i].op.move.d[0] * knob_value;
      yval = op[i].op.move.d[1] * knob_value;
      zval = op[i].op.move.d[2] * knob_value;

      transform = make_translate( xval, yval, zval );
      //multiply by the existing origin
      matrix_mult( s->data[ s->top ], transform );
      //put the new matrix on the top
      copy_matrix( transform, s->data[ s->top ] );
      free_matrix( transform );
      break;

    case SCALE:
      knob_value = 1;
      if(op[i].op.scale.p){
	temp = knobs[k];
	while (strcmp(temp->name, op[i].op.scale.p->name))
	  temp = temp ->next;
	if(temp)
	  knob_value = temp ->value;
      }

      xval = op[i].op.scale.d[0] * knob_value;
      yval = op[i].op.scale.d[1] * knob_value;
      zval = op[i].op.scale.d[2] * knob_value;
      
      transform = make_scale( xval, yval, zval );
      matrix_mult( s->data[ s->top ], transform );
      //put the new matrix on the top
      copy_matrix( transform, s->data[ s->top ] );
      free_matrix( transform );
      break;

    case ROTATE:
      knob_value = 1;
      if(op[i].op.rotate.p){
	temp = knobs[k];
	while (strcmp(temp->name, op[i].op.rotate.p->name))
	  temp = temp ->next;
	if(temp)
	  knob_value = temp ->value;
      }
      
      xval = op[i].op.rotate.degrees * ( M_PI / 180 ) * knob_value;

      //get the axis
      if ( op[i].op.rotate.axis == 0 ) 
	transform = make_rotX( xval );
      else if ( op[i].op.rotate.axis == 1 ) 
	transform = make_rotY( xval );
      else if ( op[i].op.rotate.axis == 2 ) 
	transform = make_rotZ( xval );

      matrix_mult( s->data[ s->top ], transform );
      //put the new matrix on the top
      copy_matrix( transform, s->data[ s->top ] );
      free_matrix( transform );
      break;

    case PUSH:
      push( s );
      break;
    case POP:
      pop( s );
      break;
    case SAVE:
      save_extension( t, op[i].op.save.p->name );
      break;
    case DISPLAY:
      display( t );
      break;
    }
  }
  
  strcpy(dir,"anim");
  strcat(dir,name);
  sprintf(p,"%03d", k);
  strcat(dir,p);
  strcat(dir, ".png");
  save_extension(t, dir);
  }
  
  
  free_stack( s );
  free_matrix( tmp );
  //free_matrix( transform );    
}
