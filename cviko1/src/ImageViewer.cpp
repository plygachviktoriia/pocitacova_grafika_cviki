#include "ImageViewer.h"
#include <QRandomGenerator>

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);

	vW = new ViewerWidget(QSize(700, 700), ui->scrollArea);
	ui->scrollArea->setWidget(vW);
	v3D = new Viewer3DWidget(QSize(700, 700), ui->scrollArea);

	ui->comboBoxLineAlg->setCurrentIndex(0);          // DDA selected
	ui->toolButtonDrawLine->setEnabled(true);      // Line is active
	ui->toolButtonDrawLine->setChecked(true);
	ui->CircletoolButton->setEnabled(false);

	ui->scrollArea->setBackgroundRole(QPalette::Dark);
	ui->scrollArea->setWidgetResizable(false);
	ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	vW->setObjectName("ViewerWidget");
	vW->installEventFilter(this);

	globalColor = Qt::blue;
	QString style_sheet = QString("background-color: %1;").arg(globalColor.name(QColor::HexRgb));
	ui->pushButtonSetColor->setStyleSheet(style_sheet);
}

void ImageViewer::on_T1pushButtonColor_clicked() 
{
	QColor c = QColorDialog::getColor(colorV1, this, "Select Color for Vertex 1");
	if (c.isValid()) {
		colorV1 = c;
		ui->T1pushButtonColor->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}

void ImageViewer::on_T2pushButtonColor_clicked() 
{
	QColor c = QColorDialog::getColor(colorV2, this, "Select Color for Vertex 2");
	if (c.isValid()) {
		colorV2 = c;
		ui->T2pushButtonColor->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}

void ImageViewer::on_T3pushButtonColor_clicked() {
	QColor c = QColorDialog::getColor(colorV3, this, "Select Color for Vertex 3");
	if (c.isValid()) {
		colorV3 = c;
		ui->T3pushButtonColor->setStyleSheet(QString("background-color: %1;").arg(c.name()));
	}
}

// Event filters
bool ImageViewer::eventFilter(QObject* obj, QEvent* event)
{
	if (obj->objectName() == "ViewerWidget") {
		return ViewerWidgetEventFilter(obj, event);
	}
	return QMainWindow::eventFilter(obj, event);
}

//ViewerWidget Events
bool ImageViewer::ViewerWidgetEventFilter(QObject* obj, QEvent* event)
{
	ViewerWidget* w = static_cast<ViewerWidget*>(obj);

	if (!w) {
		return false;
	}

	if (event->type() == QEvent::MouseButtonPress) {
		ViewerWidgetMouseButtonPress(w, event);
	}
	else if (event->type() == QEvent::MouseButtonRelease) {
		ViewerWidgetMouseButtonRelease(w, event);
	}
	else if (event->type() == QEvent::MouseMove) {
		ViewerWidgetMouseMove(w, event);
	}
	else if (event->type() == QEvent::Leave) {
		ViewerWidgetLeave(w, event);
	}
	else if (event->type() == QEvent::Enter) {
		ViewerWidgetEnter(w, event);
	}
	else if (event->type() == QEvent::Wheel) {
		ViewerWidgetWheel(w, event);
	}

	return QObject::eventFilter(obj, event);
}

void ImageViewer::on_ResetpushButton_clicked()
{
	vW->clear();

	// LINE RESET
	vW->setDrawLineActivated(false);
	vW->setDrawLineBegin(QPoint(0, 0));
	vW->setDrawLineEnd(QPoint(0, 0));

	// CIRCLE RESET
	vW->setCircleRadius(0);
	vW->setDrawCircleActivated(false);
	vW->setDrawCircleCenter(QPoint(0, 0));

	// POLYGON RESET
	vW->getPolygonPoints().clear();
	vW->setPolygonActive(false);

	vW->update();
}

void ImageViewer::ViewerWidgetMouseButtonPress(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* current = static_cast<QMouseEvent*>(event);
	
	// POLYGON PRESS
	if (w->activePolygon()) 
	{ 
		if (current->button() == Qt::LeftButton) 
		{ 
			QPoint point = current->pos(); 

			if (w->getPolygonPoints().isEmpty()) 
			{ 
				w->addPolygonPoint(point); 
				w->setPixel(point.x(), point.y(), globalColor); 
			} 
			else { 
				QPoint prev = w->getPolygonPoints().last(); 
				w->addPolygonPoint(point); 
				w->drawLineDDA(prev, point, globalColor); 
			} 
			w->update(); 
		} 
		else if (current->button() == Qt::RightButton) 
		{ 
			QVector<QPoint>& pointsPolygon = w->getPolygonPoints(); 
			if (pointsPolygon.size() >= 3) 
			{ 
				w->drawLineDDA(pointsPolygon.last(), pointsPolygon.first(), globalColor); 
			
			} 
			w->setPolygonActive(false); 
			ui->PolygontoolButton->setChecked(false); 
			w->update(); 
		} 
		return; 
	}

	// LINE PRESS
	if (ui->toolButtonDrawLine->isChecked())
	{
		if (!w->getDrawLineActivated())
		{
			w->setDrawLineBegin(current->pos());
			w->setDrawLineActivated(true);
			w->setPixel(current->pos().x(), current->pos().y(), globalColor);
			w->update();
		}
		else
		{
			w->setDrawLineEnd(current->pos());
			w->drawLine(w->getDrawLineBegin(), w->getDrawLineEnd(), globalColor, ui->comboBoxLineAlg->currentIndex());
			w->setDrawLineActivated(false);
		}
		vW->originalLineEnd = vW->getDrawLineEnd();
	}

	// CIRCLE PRESS
	if (ui->CircletoolButton->isChecked())
	{
		if (!w->getDrawCircleActivated())
		{
			w->setDrawCircleCenter(current->pos());
			w->setDrawCircleActivated(true);

			w->setPixel(current->pos().x(), current->pos().y(), globalColor);
			w->update();
		}
		else
		{
			QPoint center = w->getDrawCircleCenter();
			QPoint p = current->pos();

			int dx = p.x() - center.x();
			int dy = p.y() - center.y();
			int radius = qRound(sqrt(dx * dx + dy * dy));

			w->setCircleRadius(radius);
			w->drawCircleBresenham(center, w->getCircleRadius(), globalColor);
			w->setDrawCircleActivated(false);
		}
	}

	if (current->button() == Qt::LeftButton &&
		!w->activePolygon() &&
		!w->getDrawLineActivated() &&
		!w->getDrawCircleActivated())
	{
		w->startDragging = current->pos();
		w->dragging = true;
	}
}

void ImageViewer::ViewerWidgetMouseButtonRelease(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (e->button() != Qt::LeftButton || !w->dragging)
		return;

	QPoint delta = e->pos() - w->startDragging;

	int index = ui->comboBoxLineAlg->currentIndex();
	w->MoveObjects(delta, index, globalColor);

	w->dragging = false;
	w->update();
}

void ImageViewer::ViewerWidgetMouseMove(ViewerWidget* w, QEvent* event)
{
	QMouseEvent* e = static_cast<QMouseEvent*>(event);

	if (!w->dragging)
		return;

	QPoint delta = e->pos() - w->startDragging;

	int index = ui->comboBoxLineAlg->currentIndex();
	w->MoveObjects(delta, index, globalColor);

	w->startDragging = e->pos();
}
            
void ImageViewer::ViewerWidgetLeave(ViewerWidget* w, QEvent* event)
{
}

void ImageViewer::ViewerWidgetEnter(ViewerWidget* w, QEvent* event)
{
}

void ImageViewer::ViewerWidgetWheel(ViewerWidget* w, QEvent* event)
{
	QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);

	int angel_delta = wheelEvent->angleDelta().y();
	if (angel_delta == 0) return;
	int index = ui->comboBoxLineAlg->currentIndex();

	w->WheelMove(QPoint(0, angel_delta), index, globalColor);
}

void ImageViewer::on_DegeeresDoubleSpinBox_valueChanged(double value)
{
	vW->rotationAngle = value;
}

void ImageViewer::on_RotationtoolButton_clicked()
{
	if (!vW) return;

	double rotation_angle = ui->DegeeresDoubleSpinBox->value();
	int index = ui->comboBoxLineAlg->currentIndex();

	vW->RotationObjects(rotation_angle, index, globalColor);
}

void ImageViewer::on_ScailingpushButton_clicked()
{
	if (!vW) return;

	double Sx = ui->XscaledoubleSpinBox->value();
	double Sy = ui->YscaledoubleSpinBox->value();
	int index = ui->comboBoxLineAlg->currentIndex();

	vW->ScaleObjects(Sx, Sy, index, globalColor);
	
}

void ImageViewer::on_SymmetrypushButton_clicked()
{
	if (!vW) return;

	int symmetry_index = ui->SymmetrycomboBox->currentIndex();
	int index = ui->comboBoxLineAlg->currentIndex();

	vW->SymmetryObjects(symmetry_index, index, globalColor);
}

void ImageViewer::on_SlashpushButton_clicked()
{
	if (!vW) return;

	double value = ui->SlashdoubleSpinBox->value();
	int index = ui->comboBoxLineAlg->currentIndex();

	vW->SlashObjects(value, index, globalColor);
}

void ImageViewer::on_FillpushButton_clicked()
{
	if (!vW) return;

	// POLYGON FILL

	QVector<QPoint>& polygonPoints = vW->getPolygonPoints();
	int size = polygonPoints.size();

	if (size > 3)
	{
		vW->ScanLine(globalColor);
	}
	vW->update();
}

void ImageViewer::on_createCube_clicked()
{
	if (ui->scrollArea->widget() != v3D) 
	{
		ui->scrollArea->takeWidget();
		ui->scrollArea->setWidget(v3D);
	}

	int size = ui->cubeSizeSpinBox->value();

	v3D->create_cube(size);  
	v3D->update();
}

void ImageViewer::on_createSphere_clicked()
{
	if (ui->scrollArea->widget() != v3D) 
	{
		ui->scrollArea->takeWidget();
		ui->scrollArea->setWidget(v3D);
	}

	double radius = ui->RadiusSphereSpinBox->value();
	int medians = ui->MerediansSphereSpinBox->value();
	int parallels = ui->ParallelsSphereSpinBox->value();

	v3D->create_sphere(radius, medians, parallels);
	v3D->update();
}

//ImageViewer Events
void ImageViewer::closeEvent(QCloseEvent* event)
{
	if (QMessageBox::Yes == QMessageBox::question(this, "Close Confirmation", "Are you sure you want to exit?", QMessageBox::Yes | QMessageBox::No))
	{
		event->accept();
	}
	else {
		event->ignore();
	}
}

//Image functions
bool ImageViewer::openImage(QString filename)
{
	QImage loadedImg(filename);
	if (!loadedImg.isNull()) {
		return vW->setImage(loadedImg);
	}
	return false;
}
bool ImageViewer::saveImage(QString filename)
{
	QFileInfo fi(filename);
	QString extension = fi.completeSuffix();

	QImage* img = vW->getImage();
	return img->save(filename, extension.toStdString().c_str());
}

//Slots
void ImageViewer::on_actionOpen_triggered()
{
	QString folder = settings.value("folder_img_load_path", "").toString();

	QString fileFilter = "All Supported (*.bmp *.jpg *.png *.vtk);;3D Models (*.vtk);;Image data (*.bmp *.gif *.jpg *.jpeg *.png);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load file", folder, fileFilter);

	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (fi.suffix().toLower() == "vtk")
	{
		if (!v3D) {
			v3D = new Viewer3DWidget(QSize(700, 700), ui->scrollArea);
		}

		if (ui->scrollArea->widget() != v3D) {
			ui->scrollArea->takeWidget();
			ui->scrollArea->setWidget(v3D);
		}

		v3D->LoadVTK(fileName.toStdString()); //[cite: 12]
		v3D->update(); // Ïåðåìàëüîâóºìî ÁÅÇ v3D->show()
	}
	else
	{
		if (ui->scrollArea->widget() != vW) {
			ui->scrollArea->takeWidget();
			ui->scrollArea->setWidget(vW); //[cite: 10]
		}

		if (!openImage(fileName)) { //[cite: 10]
			msgBox.setText("Unable to open image.");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
	}
}
void ImageViewer::on_actionSave_as_triggered()
{
	QString folder = settings.value("folder_img_save_path", "").toString();

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getSaveFileName(this, "Save image", folder, fileFilter);
	if (!fileName.isEmpty()) {
		QFileInfo fi(fileName);
		settings.setValue("folder_img_save_path", fi.absoluteDir().absolutePath());

		if (!saveImage(fileName)) {
			msgBox.setText("Unable to save image.");
			msgBox.setIcon(QMessageBox::Warning);
		}
		else {
			msgBox.setText(QString("File %1 saved.").arg(fileName));
			msgBox.setIcon(QMessageBox::Information);
		}
		msgBox.exec();
	}
}
void ImageViewer::on_actionClear_triggered()
{
	vW->clear();

	vW->setDrawLineActivated(false);
	vW->setDrawCircleActivated(false);
	vW->clearPolygon();

	ui->PolygontoolButton->setChecked(false);
}
void ImageViewer::on_actionExit_triggered()
{
	this->close();
}

void ImageViewer::on_pushButtonSetColor_clicked()
{
	QColor newColor = QColorDialog::getColor(globalColor, this);

	if (newColor.isValid()) 
	{
		QString style_sheet = QString("background-color: %1;").arg(newColor.name(QColor::HexRgb));

		ui->pushButtonSetColor->setStyleSheet(style_sheet);
		globalColor = newColor;
	}
}

void ImageViewer::on_PolygontoolButton_clicked()
{
	ui->toolButtonDrawLine->setChecked(false);
	ui->CircletoolButton->setChecked(false);
	ui->PolygontoolButton->setChecked(true);

	vW->getPolygonPoints().clear();
	vW->setPolygonActive(true);
}


// COMBOBOX
void ImageViewer::on_comboBoxLineAlg_currentIndexChanged(int index)
{
	vW->setDrawLineActivated(false);
	vW->setDrawCircleActivated(false);

	if (index == 0 || index == 1)
	{
		ui->toolButtonDrawLine->setEnabled(true);
		ui->toolButtonDrawLine->setChecked(true);

		ui->CircletoolButton->setEnabled(false);
		ui->CircletoolButton->setChecked(false);

		vW->setPolygonActive(false); 
	}
	else if (index == 2)
	{
		ui->toolButtonDrawLine->setEnabled(false);
		ui->toolButtonDrawLine->setChecked(false);

		ui->CircletoolButton->setEnabled(true);
		ui->CircletoolButton->setChecked(true);

		vW->setPolygonActive(false);
	}
}