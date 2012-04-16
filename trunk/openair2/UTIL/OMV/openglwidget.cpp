/*******************************************************************************

  Eurecom OpenAirInterface 2
  Copyright(c) 1999 - 2010 Eurecom

  This program is free software; you can redistribute it and/or modify it
  under the terms and conditions of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This program is distributed in the hope it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.

  The full GNU General Public License is included in this distribution in
  the file called "COPYING".

  Contact Information
  Openair Admin: openair_admin@eurecom.fr
  Openair Tech : openair_tech@eurecom.fr
  Forums       : http://forums.eurecom.fsr/openairinterface
  Address      : Eurecom, 2229, route des crêtes, 06560 Valbonne Sophia Antipolis, France

*******************************************************************************/


/*! \file openglwidget.cpp
* \brief area devoted to draw the nodes and their connections
* \author M. Mosli
* \date 2012
* \version 0.1 
* \company Eurecom
* \email: mosli@eurecom.fr
*/ 

#include "openglwidget.h"
#include <stdio.h>

extern struct Geo geo[NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX];
extern int x_area;
extern int y_area;
extern int z_area;
extern int node_number;

OpenGLWidget::OpenGLWidget()
{/*
    glEnable(GL_TEXTURE_2D);
    QImage t;
    QImage b;
    
    if ( !b.load( "red.png" ) )
    {
      b = QImage( 16, 16, 32 );
      b.fill( Qt::green.rgb() );
    }
    
    t = QGLWidget::convertToGLFormat( b );
    glGenTextures( 1, &textures[0] );
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexImage2D( GL_TEXTURE_2D, 0, 3, t.width(), t.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, t.bits() );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );    
*/
    geo[0].x = -1;
    draw_connections = true;
    
}

void OpenGLWidget::paintGL()
{

  glClearColor(0,0,0,0);
  //glClearColor(255,255,255,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  //gluPerspective(70,(double)600/440,1,5000);
  gluOrtho2D(0,640,0,480); 

/*
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  gluLookAt(camera[0], camera[1], camera[2], camera[3], camera[4], camera[5],camera[6],camera[7],camera[8]);
*/
  glTranslated(20,20,0);
  drawGrid();
  drawNodes();

  if (draw_connections)
    drawConnections();

}

void OpenGLWidget::drawNewPosition(){
    updateGL();
}

void OpenGLWidget::setDrawConnections(int draw){
    this->draw_connections = draw;
    updateGL();
}

OpenGLWidget::~OpenGLWidget(){}

void OpenGLWidget::drawConnections(){

    for (int i=0; i<node_number; i++){
        for (int j=i+1; j<node_number; j++){
            int k=0;

            while((geo[i].Neighbor[k]!=j)&&(k<geo[i].Neighbors)){
                k++;
            }

            if(k < geo[i].Neighbors){
	      	glColor3d(255,255,255);
           
                //choose it according to the number of displayed nodes
                glLineWidth(1.2);
                glBegin(GL_LINES);
                    glVertex2d((int)(((float)geo[i].x/(float)x_area)*600),(int)(((float)geo[i].y/(float)y_area)*440));
                    glVertex2d((int)(((float)geo[j].x/(float)x_area)*600),(int)(((float)geo[j].y/(float)y_area)*440));
                glEnd();
            }
        }
    }


}

void OpenGLWidget::drawSquare(int half_side){
    glBegin(GL_QUADS);
      glVertex2d(-half_side,-half_side);
      glVertex2d(-half_side, half_side);
      glVertex2d( half_side, half_side);
      glVertex2d( half_side,-half_side);
    glEnd();
}

void OpenGLWidget::drawNodes(){

    /*GLUquadric* params;
    params = gluNewQuadric();
    gluQuadricDrawStyle(params,GLU_FILL);*/

    if (geo[0].x != -1){
        
	if (geo[0].node_type == 0)
	    glColor3d(255,255,255);
	else
	    glColor3d(0,255,0);

        glTranslated((int)(((float)geo[0].x/(float)x_area)*600), (int)(((float)geo[0].y/(float)y_area)*440),0);
        //gluSphere(params,50,25,25);
	drawSquare(5);

        for (int i=1; i<node_number; i++){

	    if (geo[i].node_type == 0)
	        glColor3d(255,255,255);
	    else
	        glColor3d(0,255,0);

            glTranslated((int)(((float)geo[i].x/(float)x_area)*600) - (int)(((float)geo[i-1].x/(float)x_area)*600),
			(int)(((float)geo[i].y/(float)y_area)*440) - (int)(((float)geo[i-1].y/(float)y_area)*440),0);
            //gluSphere(params,50,25,25);
	    drawSquare(5);
        }

        glTranslated(-(int)(((float)geo[node_number - 1].x/(float)x_area)*600),-(int)(((float)geo[node_number - 1].y/y_area)*440),0);
    }
    //gluDeleteQuadric(params);
}

// To be ket for the moment until knowing the best position of the camera. 
// allows to see if all the area is displayed

void OpenGLWidget::drawGrid(){
    glColor3d(0,0,255);
    glLineWidth(1.0);

    glBegin(GL_LINES);

    /* Lines that are parallel to (Ox) */
    for (int i=0; i <= y_area; i+= y_area){
        glVertex2d(0,(int)(((float)i/(float)y_area)*440));
        glVertex2d(600,(int)(((float)i/(float)y_area)*440));
    }

     /* Lines that are parallel to (Oy) */
     for (int i=0; i <= x_area; i+= x_area){
        glVertex2d((int)(((float)i/(float)x_area)*600),0);
        glVertex2d((int)(((float)i/(float)x_area)*600),440);
     }

    glEnd();
}
