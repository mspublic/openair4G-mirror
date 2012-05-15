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


/*! \file mywindow.cpp
* \brief manages the window and its components
* \author M. Mosli
* \date 2012
* \version 0.1 
* \company Eurecom
* \email: mosli@eurecom.fr
*/ 

#include "mywindow.h"
#include "communicationthread.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QString>

//extern pid_t simulator_pid;
extern int pfd[2];
extern struct Geo geo[NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX];
extern CommunicationThread* communication_thread;
extern int x_area, y_area;
extern int nb_frames;
extern int nb_enb;
extern int nb_ue;

MyWindow::MyWindow() : QWidget()
{
    pattern = 0;
    
    this->setFixedSize(950, 750);
    QObject::connect(this, SIGNAL(exitSignal()), qApp, SLOT(quit()));

    /* Control widgets */
    QLabel *pattern = new QLabel;
    pattern->setPixmap(QPixmap("motif.png"));
    pattern->setFixedSize(120,120);

    QLabel *pattern1 = new QLabel;
    pattern1->setPixmap(QPixmap("motif1.png"));
    pattern1->setFixedSize(120,120);

    simulation_data = new QLabel;
    QString sim_data;
    sim_data.sprintf("Area dimensions: %dx%d\n\nUE node number: %d\n\neNb node number: %d\n",x_area,y_area,nb_ue,nb_enb);
    simulation_data->setText(sim_data);
    simulation_data->setFixedHeight(200);

    QCheckBox *drawConnections = new QCheckBox("Draw Connections");
    drawConnections->setChecked(true);
    QObject::connect(drawConnections, SIGNAL(stateChanged(int)), this, SLOT(setDrawConnections(int)));

    /* Control area */
    control_field = new QFrame(this);
    control_field->setFrameShape(QFrame::StyledPanel);
    control_field->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    control_field->setLineWidth( 2 );

    QVBoxLayout *control_layout = new QVBoxLayout;
    control_layout->addWidget(simulation_data);
    control_layout->addWidget(drawConnections);
    control_field->setLayout(control_layout);

    /* Drawing area */
       openGL_field = new QFrame(this);
    openGL_field->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    openGL_field->setLineWidth( 2 );
    openGl = new OpenGLWidget();
    openGl->setFixedSize(640,480);
    QVBoxLayout *l1 = new QVBoxLayout;
    l1->addWidget(openGl);
    openGL_field->setLayout(l1);
    
    /* Console area */
    console_field = new QFrame(this);
    output = new QTextEdit();
    output->setReadOnly(true);
    QVBoxLayout *l2 = new QVBoxLayout;
    l2->addWidget(output);
    console_field->setLayout(l2);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(openGL_field, 0, 0, 1, 3);
    layout->addWidget(control_field, 0, 3, 3, 1);
    layout->addWidget(console_field, 3, 0, 3, 4);

    this->setLayout(layout);

    communication_thread = new CommunicationThread(this);
    communication_thread->start();
}

QTextEdit* MyWindow::getConsoleField(){
    return this->output;
}

OpenGLWidget *MyWindow::getGL(){
    return this->openGl;
}

void MyWindow::endOfTheSimulation(){
    communication_thread->wait();  // do not exit before the thread is completed!
    delete communication_thread;
    emit exitSignal();
}

void MyWindow::setDrawConnections(int draw){
    openGl->setDrawConnections(draw);
}

void MyWindow::writeToConsole(QString data){
    this->output->append(data);
}

MyWindow::~MyWindow(){
    if (::close (pfd[0]) == -1 ) /* we close the read desc. */
        perror( "close on read" );
}
