#pragma once

#include <QtWidgets/QMainWindow>
#include <QtWidgets>
#include "ui_ImageViewer.h"
#include "ViewerWidget.h"

class ImageViewer : public QMainWindow
{
	Q_OBJECT

public:
	ImageViewer(QWidget* parent = Q_NULLPTR);
	~ImageViewer() { delete ui; }
private:
	Ui::ImageViewerClass* ui;
	ViewerWidget* vW;

	QColor globalColor;
	QSettings settings;
	QMessageBox msgBox;

	QColor colorV1 = Qt::red;
	QColor colorV2 = Qt::green;
	QColor colorV3 = Qt::blue;

	//Event filters
	bool eventFilter(QObject* obj, QEvent* event);

	//ViewerWidget Events
	bool ViewerWidgetEventFilter(QObject* obj, QEvent* event);
	void ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event);
	void ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event);
	void ViewerWidgetLeave(ViewerWidget* w, QEvent* event);
	void ViewerWidgetEnter(ViewerWidget* w, QEvent* event);
	void ViewerWidgetWheel(ViewerWidget* w, QEvent* event);

	//ImageViewer Events
	void closeEvent(QCloseEvent* event);

	//Image functions
	bool openImage(QString filename);
	bool saveImage(QString filename);

private slots:
	void on_actionOpen_triggered();
	void on_actionSave_as_triggered();
	void on_actionClear_triggered();
	void on_actionExit_triggered();

	//Tools slots
	void on_pushButtonSetColor_clicked();
	void on_comboBoxLineAlg_currentIndexChanged(int index);

	void on_PolygontoolButton_clicked();
	void on_RotationtoolButton_clicked();
	void on_DegeeresDoubleSpinBox_valueChanged(double value);
	void on_ScailingpushButton_clicked();
	void on_SymmetrypushButton_clicked();
	void on_ResetpushButton_clicked();
	void on_FillpushButton_clicked();
	void on_SlashpushButton_clicked();

	void on_T1pushButtonColor_clicked();
	void on_T2pushButtonColor_clicked();
	void on_T3pushButtonColor_clicked();
};
