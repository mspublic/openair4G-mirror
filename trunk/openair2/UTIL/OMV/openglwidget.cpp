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
extern int nb_enb;

OpenGLWidget::OpenGLWidget()
{
    geo[0].x = -1;
    draw_connections = true;   
}

void OpenGLWidget::paintGL()
{

  glClearColor(0,0,0,0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  gluOrtho2D(0,640,0,480); 

  glTranslated(20,20,0);
  drawGrid();

  if (draw_connections)
    drawConnections();

  drawNodes();
}

void OpenGLWidget::loadTexture(){
    QImage b;
    
    glEnable(GL_TEXTURE_2D);
    if ( !b.load( "Blog.bmp" ) )
    {
      b = QImage( 16, 16, QImage::Format_RGB32 );
      b.fill( Qt::green );
    }
   // textures[0] = bindTexture(QPixmap("Blog.bmp"), GL_TEXTURE_2D);

    b_station = QGLWidget::convertToGLFormat( b );
    glGenTextures( 1, &textures[0] );
    glBindTexture( GL_TEXTURE_2D, textures[0] );
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0,0 , b_station.width(), b_station.height(),  GL_RGB, GL_UNSIGNED_BYTE, b_station.bits() ); 
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}



void OpenGLWidget::drawNewPosition(){
    updateGL();
}

void OpenGLWidget::setDrawConnections(int draw){
    this->draw_connections = draw;
    updateGL();
}

OpenGLWidget::~OpenGLWidget(){
    glDeleteTextures(1, textures);
}

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
                glLineWidth(0.7);
                glBegin(GL_LINES);
                    glVertex2d((int)(((float)geo[i].x/(float)x_area)*600),(int)(((float)geo[i].y/(float)y_area)*440));
                    glVertex2d((int)(((float)geo[j].x/(float)x_area)*600),(int)(((float)geo[j].y/(float)y_area)*440));
                glEnd();
            }
        }
    }


}

void OpenGLWidget::drawNodes(){

    if (geo[0].x != -1){

        glTranslated((int)(((float)geo[0].x/(float)x_area)*600), (int)(((float)geo[0].y/(float)y_area)*440),0);
        //gluSphere(params,50,25,25);
	
	if (geo[0].node_type == 0){
	    glColor3d(150,0,150);
	    drawBaseStation();
	}else{
	    glColor3d(0,255,100);	
	    drawSquare(0);
	}
	
        for (int i=1; i<node_number; i++){

	    glTranslated((int)(((float)geo[i].x/(float)x_area)*600) - (int)(((float)geo[i-1].x/(float)x_area)*600),
			(int)(((float)geo[i].y/(float)y_area)*440) - (int)(((float)geo[i-1].y/(float)y_area)*440),0);

	    if (geo[i].node_type == 0){
	        glColor3d(150,0,150);
	        drawBaseStation();
	    }else{
	        glColor3d(0,255,100);	
	        drawSquare(i - nb_enb);
	    }

        }

        glTranslated(-(int)(((float)geo[node_number - 1].x/(float)x_area)*600),-(int)(((float)geo[node_number - 1].y/y_area)*440),0);
    }
}

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

void OpenGLWidget::drawSquare(int digit){
    glBegin(GL_QUADS);
	glVertex2d(-6,-6);
	glVertex2d(-6, 6);
	glVertex2d( 6, 6);
	glVertex2d( 6,-6);
    glEnd();

    //draw the digit
    glColor3d(255,0,0);
    glBegin(GL_LINES);
     switch (digit){
       	case 0:
	glVertex2d(-3,4);
	glVertex2d(3,4);
	glVertex2d(-3,-4);
	glVertex2d(3,-4);
	glVertex2d(3,-4);
	glVertex2d(3,4);
	glVertex2d(-3,-4);
	glVertex2d(-3,4);
	break;
	
	case 1:
	glVertex2d(3,4);
	glVertex2d(3,-4);
	break;

	case 2:
	glVertex2d(-3,4);
	glVertex2d(3,4);
	glVertex2d(3,4);
	glVertex2d(3,0);	
	glVertex2d(-3,0);
	glVertex2d(3,0);
	glVertex2d(-3,-4);
	glVertex2d(-3,0);
	glVertex2d(-3,-4);
	glVertex2d(3,-4);
	break;

	case 3:
	glVertex2d(3,4);
	glVertex2d(3,-4);
	glVertex2d(-3,4);
	glVertex2d(3,4);
	glVertex2d(3,0);	
	glVertex2d(-3,0);
	glVertex2d(-3,-4);
	glVertex2d(3,-4);
	break;

	case 4:
	glVertex2d(3,-4);
	glVertex2d(3,4);
	glVertex2d(-3,4);
	glVertex2d(-3,0);
	glVertex2d(-3,0);
	glVertex2d(3,0);
	break;

	case 5:
	glVertex2d(-3,4);
	glVertex2d(3,4);
	glVertex2d(-3,4);
	glVertex2d(-3,0);	
	glVertex2d(-3,0);
	glVertex2d(3,0);
	glVertex2d(3,-4);
	glVertex2d(3,0);
	glVertex2d(-3,-4);
	glVertex2d(3,-4);
	break;

	case 6:
	glVertex2d(-3,4);
	glVertex2d(3,4);
	glVertex2d(-3,4);
	glVertex2d(-3,-4);	
	glVertex2d(-3,0);
	glVertex2d(3,0);
	glVertex2d(3,-4);
	glVertex2d(3,0);
	glVertex2d(-3,-4);
	glVertex2d(3,-4);
	break;

	case 7:
	glVertex2d(3,4);
	glVertex2d(3,-4);
	glVertex2d(-3,4);
	glVertex2d(3,4);
	break;
     }
    glEnd();
}

void OpenGLWidget::drawBaseStation(){
    glLineWidth(2.0);
    glBegin(GL_LINES);
	glVertex2d(-6,0);
	glVertex2d(6,0);

	glVertex2d(-6,0);
	glVertex2d(-6,5);

	glVertex2d(6,0);
	glVertex2d(6,5);

	glVertex2d(0,0);
	glVertex2d(-6,-12);

	glVertex2d(0,0);
	glVertex2d(6,-12);

	glVertex2d(0,5);
	glVertex2d(0,-12);
    glEnd();

}
