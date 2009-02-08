/*
 * Copyright (c) 2007, Aconex.  All Rights Reserved.
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
#include <QtCore/QDir>
#include <QtGui/QPainter>
#include <QtGui/QMessageBox>
#include <QtGui/QImageWriter>
#include <qwt_plot_printfilter.h>
#include "main.h"
#include "exportdialog.h"

// ExportFileDialog is the one which is displayed when you click on
// the image file selection push button
ExportFileDialog::ExportFileDialog(QWidget *parent) : QFileDialog(parent)
{
    setAcceptMode(QFileDialog::AcceptSave);
    setFileMode(QFileDialog::AnyFile);
    setIconProvider(fileIconProvider);
    setConfirmOverwrite(true);
}

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent)
{
    setupUi(this);
    init();
}

ExportDialog::~ExportDialog()
{
    free(my.format);
}

void ExportDialog::languageChange()
{
    retranslateUi(this);
}

void ExportDialog::init()
{
    QString imgfile = QDir::homePath();

    my.quality = 0;
    my.format = strdup("png");
    imgfile.append("/export.png");
    fileLineEdit->setText(imgfile);

    int png = 0;
    QStringList formats;
    QList<QByteArray> array = QImageWriter::supportedImageFormats();
    for (int i = 0; i < array.size(); i++) {
	if (array.at(i) == "png")
	    png = i;
	formats << QString(array.at(i));
    }
    formatComboBox->blockSignals(true);
    formatComboBox->addItems(formats);
    formatComboBox->setCurrentIndex(png);
    formatComboBox->blockSignals(false);

    qualitySlider->setValue(100);
    qualitySlider->setRange(0, 100);
    qualitySpinBox->setValue(100);
    qualitySpinBox->setRange(0, 100);
}

void ExportDialog::reset()
{
    QSize size = imageSize();
    widthSpinBox->setValue(size.width());
    heightSpinBox->setValue(size.height());
}

void ExportDialog::selectedRadioButton_clicked()
{
    selectedRadioButton->setChecked(true);
    allChartsRadioButton->setChecked(false);
    reset();
}

void ExportDialog::allChartsRadioButton_clicked()
{
    selectedRadioButton->setChecked(false);
    allChartsRadioButton->setChecked(true);
    reset();
}

void ExportDialog::quality_valueChanged(int value)
{
    if (value != my.quality) {
	my.quality = value;
	displayQualitySpinBox();
	displayQualitySlider();
    }
}

void ExportDialog::displayQualitySpinBox()
{
    qualitySpinBox->blockSignals(true);
    qualitySpinBox->setValue(my.quality);
    qualitySpinBox->blockSignals(false);
}

void ExportDialog::displayQualitySlider()
{
    qualitySlider->blockSignals(true);
    qualitySlider->setValue(my.quality);
    qualitySlider->blockSignals(false);
}

void ExportDialog::filePushButton_clicked()
{
    ExportFileDialog file(this);

    file.setDirectory(QDir::homePath());
    if (file.exec() == QDialog::Accepted)
	fileLineEdit->setText(file.selectedFiles().at(0));
}

void ExportDialog::formatComboBox_currentIndexChanged(QString suffix)
{
    char *format = strdup((const char *)suffix.toAscii());
    QString file = fileLineEdit->text().trimmed();
    QString regex = my.format;

    regex.append("$");
    file.replace(QRegExp(regex), suffix);
    fileLineEdit->setText(file);
    free(my.format);
    my.format = format;
}

QSize ExportDialog::imageSize()
{
    Tab *tab = pmchart->activeTab();
    int height = 0, width = 0;

    for (int i = 0; i < tab->gadgetCount(); i++) {
	Gadget *gadget = tab->gadget(i);
	if (gadget != tab->currentGadget() && selectedRadioButton->isChecked())
	    continue;
	width = qMax(width, gadget->width());
	height += gadget->height();
    }
    height += pmchart->timeAxis()->height();

    return QSize(width, height);
}

void ExportDialog::flush()
{
    QString file = fileLineEdit->text().trimmed();
    int width = widthSpinBox->value();
    int height = heightSpinBox->value();
    bool everything = allChartsRadioButton->isChecked();
    bool transparent = transparentCheckBox->isChecked();

    if (ExportDialog::exportFile(file, my.format, my.quality, width, height,
				 transparent, everything) == false) {
	QString message = tr("Failed to save image file\n");
	message.append(file);
	QMessageBox::warning(this, pmProgname, message,
		    QMessageBox::Ok|QMessageBox::Default|QMessageBox::Escape,
		    QMessageBox::NoButton, QMessageBox::NoButton);
    }
}

bool ExportDialog::exportFile(QString &file, const char *format, int quality,
		int width, int height, bool transparent, bool everything)
{
    enum QImage::Format rgbFormat = transparent ?
				QImage::Format_ARGB32 : QImage::Format_RGB32;
    QImage image(width, height, rgbFormat);
    if (transparent == false)
	image.invertPixels();	// white background
    QPainter qp(&image);

    pmchart->painter(&qp, width, height, everything == false);
    QImageWriter writer(file, format);
    writer.setQuality(quality);
    bool sts = writer.write(image);
    if (!sts)
	fprintf(stderr, "%s: error writing %s (%s): %s\n",
		pmProgname, (const char *) file.toAscii(), format,
		(const char *) writer.errorString().toAscii());
    return sts;
}

int ExportDialog::exportFile(char *outfile, char *geometry, bool transparent)
{
    QRegExp regex;
    QString file(outfile), format;
    bool noFormat = false;
    char suffix[32];
    int i;

    // Ensure the requested image format is supported, else use GIF
    regex.setPattern("\\.([a-z]+)$");
    regex.setCaseSensitivity(Qt::CaseInsensitive);
    if (regex.indexIn(file) == 0) {
	noFormat = true;
    }
    else {
	format = regex.cap(1);
	QList<QByteArray> array = QImageWriter::supportedImageFormats();
	for (i = 0; i < array.size(); i++) {
	    const char *f1 = array.at(i);
	    const char *f2 = (const char *)format.toAscii();
	    if (strcmp(f1, f2) == 0)
		break;
	}
	if (i == array.size())
	    noFormat = true;
    }
    if (noFormat) {
	file.append(".png");
	format = QString("png");
    }
    strncpy(suffix, (const char *)format.toAscii(), sizeof(suffix));
    suffix[sizeof(suffix)-1] = '\0';

    regex.setPattern("(\\d+)x(\\d+)");
    if (regex.indexIn(QString(geometry)) != -1) {
	QSize fixed = QSize(regex.cap(1).toInt(), regex.cap(2).toInt());
	pmchart->setFixedSize(fixed);
    }

    return ExportDialog::exportFile(file, suffix, 100, pmchart->width(),
			pmchart->exportHeight(), transparent, true) == false;
}
