/*
 * Copyright (c) 2006, Ken McDonell.  All Rights Reserved.
 * Copyright (c) 2006-2007, Aconex.  All Rights Reserved.
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef TAB_H
#define TAB_H

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPixmap>
#include <QtGui/QSplitter>
#include <QtGui/QTabWidget>
#include <QtGui/QPushButton>
#include <qwt_plot.h>
#include <qwt_scale_draw.h>
#include <kmtime.h>
#include "chart.h"
#include "timebutton.h"

class PmLogger;

class Tab : public QWidget
{
    Q_OBJECT

public:
    Tab();
    void init(QTabWidget *, int, int, QmcGroup *, KmTime::Source,
			QString, struct timeval *, struct timeval *);

    QWidget *splitter() { return my.splitter; }
    int index() { return my.tab->indexOf(my.splitter); }

    Chart *chart(int);		// Nth chart
    Chart *currentChart();	// current chart (can be NULL)
    Chart *addChart();		// append a new chart to tab, make it current
    int deleteChart(int);	// remove Nth chart, return current
    int deleteChart(Chart *);	// remove given chart, return current
    int deleteCurrent(void);	// remove current chart, return current
    int numChart(void);		// number of charts
    int setCurrent(Chart *);	// set current chart based on choice

    bool isArchiveSource();	// query if tab is for archives
    QmcGroup *group();

    bool isRecording(void);
    bool setRecording(bool);
    void setFolio(QString);
    void addLogger(PmLogger *);

    void setVisibleHistory(int);
    int visibleHistory(void);
    void setSampleHistory(int);
    int sampleHistory(void);

    void setConfig(char *);
    char *config(void);		// config filename from -c
    double *timeAxisData(void);

    void step(KmTime::Packet *);
    void VCRMode(KmTime::Packet *, bool);
    void setTimezone(char *);

    void setupWorldView();
    void showTimeControl();
    void updateTimeButton();
    void updateTimeAxis(void);
    void updateTimeAxis(time_t secs);

    TimeButton::State buttonState();
    KmTime::State kmtimeState();
    void newButtonState(KmTime::State s, KmTime::Mode m, int mode, bool record);

private:
    typedef enum {
	StartState,
	ForwardState,
	BackwardState,
	EndLogState,
	StandbyState,
    } State;

    char *timeState();
    void refreshCharts();
    void adjustWorldView(KmTime::Packet *, bool);
    void adjustLiveWorldView(KmTime::Packet *);
    void adjustArchiveWorldView(KmTime::Packet *, bool);
    void adjustArchiveWorldViewForward(KmTime::Packet *, bool);
    void adjustArchiveWorldViewBackward(KmTime::Packet *, bool);
    void adjustArchiveWorldViewStop(KmTime::Packet *, bool);

    struct {
	Chart **charts;
	int count;			// total number of charts
	int current;			// currently selected chart

	double interval;		// current update interval
	struct timeval previousDelta;
	struct timeval previousPosition;

	int visible;			// -v visible points
	int samples;			// -s total number of samples
	double *timeData;		// time array (intervals)

	Tab::State timeState;
	TimeButton::State buttonState;
	KmTime::Source source;		// reliable archive/host test
	KmTime::State previousState;

	QmcGroup *group;

	bool recording;			// running any pmlogger's?
	QString folio;			// archive folio, if logging
	QList<PmLogger*> loggerList;	// list of pmloggers for our Tab

	QTabWidget *tab;		// the parent widget
	QSplitter *splitter;		// dynamically divides charts
    } my;
};

#endif	// TAB_H
