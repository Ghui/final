#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "matrix.h"
#include "gmath.h"



/*======== double * calculate_normal() ==========
  Inputs:   double ax
            double ay
	    double az
	    double bx
	    double by
	    double bz  
  Returns: A double arry of size 3 representing the 
           cross product of <ax, ay, az> and <bx, by, bz>

  04/17/12 16:46:30
  jonalf
  ====================*/
double * calculate_normal( double ax, double ay, double az,	
			   double bx, double by, double bz ) {
  
  double *normal;
  normal = (double *)malloc(3 * sizeof(double));
  
  normal[0] = ay * bz - az * by;
  normal[1] = az * bx - ax * bz;
  normal[2] = ax * by - ay * bx;

  return normal;
}

/*======== double calculate_dot() ==========
  Inputs:   struct matrix *points
            int i  
  Returns: The dot product of a surface normal and
           a view vector
  
  calculates the dot product of the surface normal to
  triangle points[i], points[i+1], points[i+2] and a 
  view vector (use <0, 0, -1> to start.

  04/17/12 16:38:34
  jonalf
  ====================*/
double calculate_dot( struct matrix *points, int i ) {

  double ax, ay, az, bx, by, bz;
  double *normal;
  double vx, vy, vz;
  double dot, mag;

  //get as and bs to calculate the normal
  ax = points->m[0][i + 1] - points->m[0][ i ];
  ay = points->m[1][i + 1] - points->m[1][ i ];
  az = points->m[2][i + 1] - points->m[2][ i ];

  bx = points->m[0][ i ] - points->m[0][ i + 2 ];
  by = points->m[1][ i ] - points->m[1][ i + 2 ];
  bz = points->m[2][ i ] - points->m[2][ i + 2 ];

  /* "real" surface normal
  bx = points->m[0][ i + 2] - points->m[0][ i ];
  by = points->m[1][ i + 2] - points->m[1][ i ];
  bz = points->m[2][ i + 2] - points->m[2][ i ];
  */
  normal = calculate_normal( ax, ay, az, bx, by, bz );

  //normalize the surface normal
  mag = sqrt( (normal[0] * normal[0]) + 
	      (normal[1] * normal[1]) + 
	      (normal[2] * normal[2]) );

  normal[0] = normal[0] / mag;
  normal[1] = normal[1] / mag;
  normal[2] = normal[2] / mag;

  //set up the view vector values
  vx = 0; //m[0][i]
  vy = 0; //m[1][i]
  vz = -1;//m[2][i]

  //calculate the dot product
  dot = normal[0] * vx + normal[1] * vy + normal[2] * vz;

  free( normal );
  
  return dot;
}
