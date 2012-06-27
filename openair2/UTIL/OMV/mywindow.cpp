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
#include <QHBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QString>
#include <QGroupBox>
#include <QComboBox>
#include <QIcon>
#include <QPixmap>
#include <QSpinBox>
#include <QFormLayout>
#include <QPalette>

//extern pid_t simulator_pid;
extern int pfd[2];
extern struct Geo geo[NUMBER_OF_eNB_MAX+NUMBER_OF_UE_MAX];
extern CommunicationThread* communication_thread;
extern int x_area, y_area;
extern int nb_frames;
extern int nb_enb;
extern int nb_ue;
int supervised_id = 0;

MyWindow::MyWindow() : QWidget()
{
    pattern = 0;
    
    this->setFixedSize(920, 820);
    QObject::connect(this, SIGNAL(exitSignal()), qApp, SLOT(quit()));

    /* Control widgets */
    simulation_data = new QLabel;
    QString sim_data;
    sim_data.sprintf("Area dimensions: %dm x %dm\n\neNb node number: %d\n\nUE node number: %d\n", x_area, y_area, nb_enb, nb_ue);
    simulation_data->setText(sim_data);

    QCheckBox *drawConnections = new QCheckBox("Draw Connections");
    drawConnections->setChecked(true);
    QObject::connect(drawConnections, SIGNAL(stateChanged(int)), this, SLOT(setDrawConnections(int)));
    
    QCheckBox *useMap = new QCheckBox("Use a Map");
    useMap->setChecked(true);
    QObject::connect(useMap, SIGNAL(stateChanged(int)), this, SLOT(setUseMap(int)));
    
    QComboBox *nodes_color = new QComboBox(this);
    QComboBox *links_color = new QComboBox(this);
    QHBoxLayout *nodes_color_layout = new QHBoxLayout(this);
    QHBoxLayout *links_color_layout = new QHBoxLayout(this);
    QLabel *nodes_color_label = new QLabel;
    QLabel *links_color_label = new QLabel;
    nodes_color_label->setText("Nodes color ");
    links_color_label->setText("Links color ");
    QFrame *node_color_frame = new QFrame;
    QFrame *link_color_frame = new QFrame;
    
    nodes_color->setIconSize(* (new QSize(30,15)));
    nodes_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/red.png")))),"   Red");
    nodes_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/blue.png")))),"   Blue");
    nodes_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/green.png")))),"   Green");
    nodes_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/white.png")))),"   White");
    QObject::connect(nodes_color, SIGNAL(currentIndexChanged(int)), this, SLOT(setNodesColor(int)));

    links_color->setIconSize(* (new QSize(30,15)));
    links_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/white.png")))),"   White");
    links_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/red.png")))),"   Red");
    links_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/blue.png")))),"   Blue");
    links_color->addItem(*(new QIcon (* (new QPixmap("../../../openair2/UTIL/OMV/green.png")))),"   Green");
    QObject::connect(links_color, SIGNAL(currentIndexChanged(int)), this, SLOT(setLinksColor(int)));
    
    
    QFrame *node_size = new QFrame(this);
    QLabel *nodes_size_label = new QLabel("<html><b>Size:</b></html>");
    QLabel *min = new QLabel("Small");
    QLabel *max = new QLabel("Big");
    QSlider* size = new QSlider;
    size->setMinimum(1);
    size->setMaximum(3);
    size->setValue(2);
    size->setOrientation(Qt::Horizontal);
    QObject::connect(size, SIGNAL(sliderMoved(int)), this, SLOT(updateSize(int)));
    QHBoxLayout *size_layout = new QHBoxLayout;
    size_layout->addWidget(nodes_size_label);
    size_layout->addWidget(min);
    size_layout->addWidget(size);
    size_layout->addWidget(max);
    node_size->setLayout(size_layout);
    
    
    nodes_color_layout->addWidget(nodes_color_label);
    nodes_color_layout->addWidget(nodes_color);
    links_color_layout->addWidget(links_color_label);
    links_color_layout->addWidget(links_color);
    node_color_frame->setLayout(nodes_color_layout);
    link_color_frame->setLayout(links_color_layout);
    
  
    QSpinBox* id_choice = new QSpinBox;
    id_choice->setRange(0,nb_ue - 1);
    QObject::connect(id_choice, SIGNAL(valueChanged(int)), this, SLOT(updateSupervNode(int)));
   
    
    QGroupBox *generic = new QGroupBox("Generic Information", this);
    QVBoxLayout *generic_layout = new QVBoxLayout;
    generic_layout->addWidget(simulation_data);  
    generic->setLayout(generic_layout);
    
    QGroupBox *specific = new QGroupBox("Specific Information", this);
    QFormLayout *specific_layout = new QFormLayout;
    specific_layout->setVerticalSpacing(12);
    specific_layout->setHorizontalSpacing(40);
    //specific_layout->setRowWrapPolicy(QFormLayout::WrapAllRows);
    QLabel *position = new QLabel("(0,0)");
    QLabel *state = new QLabel("Attached");
    QLabel *dist = new QLabel("100");
    QLabel *rssi0 = new QLabel(" 0"), *rssi1 = new QLabel(" -1"), *rssi2 = new QLabel(" -1");
    QLabel *path_loss = new QLabel("0");
    
    //QVBoxLayout *specific_layout = new QVBoxLayout;
    specific_layout->addRow(new QLabel("<html><b>Node Id  </b></html>"), id_choice);
    specific_layout->addRow(new QLabel("<html><b>Position  </b></html>"), position);
    specific_layout->addRow(new QLabel("<html><b>State  </b></html>"), state);
    specific_layout->addRow(new QLabel("<html><b>Dist. to eNb</b></html>"), dist);
    specific_layout->addRow(new QLabel("<html><b>Pathloss</b></html>"), path_loss);
    
    
    specific_layout->addRow(new QLabel("<html><b>RSSI  </b></html>"));
    QHBoxLayout *rssi = new QHBoxLayout;
    rssi->setSpacing(7);
    
    QLabel *ant0 = new QLabel("ANT0"), *ant1 = new QLabel("ANT1"), *ant2 = new QLabel("ANT2");
    
    ant0->setAlignment(Qt::AlignCenter);
    ant1->setAlignment(Qt::AlignCenter);
    ant2->setAlignment(Qt::AlignCenter);
  
    rssi0->setFrameStyle( QFrame::Raised | QFrame::Box );
    rssi1->setFrameStyle( QFrame::Raised | QFrame::Box );
    rssi2->setFrameStyle( QFrame::Raised | QFrame::Box );
    
    rssi->addWidget(ant0);
    rssi->addWidget(rssi0);
    rssi->addWidget(ant1);
    rssi->addWidget(rssi1);
    rssi->addWidget(ant2);
    rssi->addWidget(rssi2);
    
    specific_layout->addRow(rssi);
    
    
    //specific_layout->addWidget(supervised_node);  
    specific->setLayout(specific_layout);
    
    QGroupBox *cntl = new QGroupBox("Control Panel", this);
    QVBoxLayout *cntl_layout = new QVBoxLayout;
    cntl_layout->addWidget(drawConnections);
    cntl_layout->addWidget(useMap);
    cntl_layout->addWidget(node_color_frame);
    cntl_layout->addWidget(link_color_frame);
    cntl_layout->addWidget(node_size);
    cntl->setLayout(cntl_layout);

    /* Control area */
    control_field = new QFrame(this);
    control_field->setFrameShape(QFrame::StyledPanel);
    control_field->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    control_field->setLineWidth( 2 );

    QVBoxLayout *control_layout = new QVBoxLayout;
    control_layout->addWidget(generic);
    control_layout->addWidget(specific);
    control_layout->addWidget(cntl);
    control_field->setLayout(control_layout);

    /* Drawing area */
    openGL_field = new QFrame(this);
    openGL_field->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    openGL_field->setLineWidth( 2 );
    openGl = new OpenGLWidget();
    openGl->setFixedSize(620,540);
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
    layout->addWidget(openGL_field, 0, 0, 3, 3);
    layout->addWidget(control_field, 0, 3, 4, 1);
    layout->addWidget(console_field, 3, 0, 1, 3);

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

void MyWindow::setUseMap(int use){
    openGl->setUseMap(use);
}

void MyWindow::setNodesColor(int index){
    openGl->setNodesColor(index);
}

void MyWindow::setLinksColor(int index){
    openGl->setLinksColor(index);
}

void MyWindow::updateSize(int size){
    openGl->updateNodeSize(size);
}

void MyWindow::updateSupervNode(int id){
    supervised_id = id;
}

void MyWindow::updateSupervData(){
  
}

void MyWindow::writeToConsole(QString data){
    this->output->append(data);
}

MyWindow::~MyWindow(){
    if (::close (pfd[0]) == -1 ) /* we close the read desc. */
        perror( "close on read" );
}
