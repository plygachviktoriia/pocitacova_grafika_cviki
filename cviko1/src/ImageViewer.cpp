#include "ImageViewer.h"
#include <QRandomGenerator>

ImageViewer::ImageViewer(QWidget* parent)
	: QMainWindow(parent), ui(new Ui::ImageViewerClass)
{
	ui->setupUi(this);

	ui->comboBoxLineAlg->setCurrentIndex(0);          // DDA selected
	ui->toolButtonDrawLine->setEnabled(true);      // Line is active
	ui->toolButtonDrawLine->setChecked(true);
	ui->CircletoolButton->setEnabled(false);

	vW = new ViewerWidget(QSize(500, 500), ui->scrollArea);
	ui->scrollArea->setWidget(vW);

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
	
	bool isPolygonthere = !w->getPolygonPoints().isEmpty();
	bool isLinethere = (w->getDrawLineBegin() != w->getDrawLineEnd());
	bool isCirclethere = (w->getCircleRadius() > 0);

	if (!isPolygonthere && !isLinethere && !isCirclethere) return;

	int angel_delta = wheelEvent->angleDelta().y();
	if (angel_delta == 0) return;
	double scale_unit = (angel_delta > 0) ? 1.25 : 0.75;

	w->clear();

	// LINE WHEEL
	if (isLinethere)
	{
		QPoint p1 = w->getDrawLineBegin();
		QPoint p2 = w->getDrawLineEnd();

		double X = p1.x() + (p2.x() - p1.x()) * scale_unit;
		double Y = p1.y() + (p2.y() - p1.y()) * scale_unit;
		QPoint scaledLine(qRound(X), qRound(Y));

		w->setDrawLineEnd(scaledLine);

		switch (ui->comboBoxLineAlg->currentIndex())
		{
		case 0:
			w->drawLineDDA(p1, scaledLine, globalColor);
			break;
		case 1:
			w->drawLineBresenham(p1, scaledLine, globalColor);
			break;
		}
	}

	// CIRCLE WHEEL
	if (isCirclethere)
	{
		int newRadius = qRound(w->getCircleRadius() * scale_unit);
		if (newRadius < 1) newRadius = 1;

		w->setCircleRadius(newRadius);
		w->drawCircleBresenham(w->getDrawCircleCenter(), newRadius, globalColor);
	}

	// POLYGON WHEEL
	if (isPolygonthere)
	{
		QVector<QPoint> polygonPoints = w->getPolygonPoints(); 
		QPointF point1 = polygonPoints[0];                  

		int size = polygonPoints.size();

		for (int i = 1; i < size; i++)
		{
			QPointF point = polygonPoints[i];
			double X = point1.x() + (point.x() - point1.x()) * scale_unit;
			double Y = point1.y() + (point.y() - point1.y()) * scale_unit;
			polygonPoints[i] = QPoint(qRound(X), qRound(Y));
		}

		w->getPolygonPoints() = polygonPoints;
		w->drawPolygon(polygonPoints, globalColor, true);
	}

	w->update();
}

void ImageViewer::on_DegeeresDoubleSpinBox_valueChanged(double value)
{
	vW->rotationAngle = value;
}

void ImageViewer::on_RotationtoolButton_clicked()
{
	if (!vW) return;

	vW->rotationAngle += ui->DegeeresDoubleSpinBox->value();
	double angle = vW->rotationAngle * M_PI / 180.0;

	vW->clear();

	// LINE ROTATE
	QPoint p1 = vW->getDrawLineBegin();
	QPoint p2 = vW->originalLineEnd;

	double Sx = p1.x();
	double Sy = p1.y();
	double x = p2.x();
	double y = p2.y();
	double dx, dy;

	dx = (x - Sx) * cos(angle) + (y - Sy) * sin(angle) + Sx;
	dy = -(x - Sx) * sin(angle) + (y - Sy) * cos(angle) + Sy;
	
	QPoint rotatedLineEnd(qRound(dx), qRound(dy));
	vW->setDrawLineEnd(rotatedLineEnd);

	if (p1 != rotatedLineEnd)
	{
		switch (ui->comboBoxLineAlg->currentIndex())
		{
		case 0:
			vW->drawLineDDA(p1, rotatedLineEnd, globalColor);
			break;
		case 1:
			vW->drawLineBresenham(p1, rotatedLineEnd, globalColor);
			break;
		}
	}

	// POLYGON ROTATE 
	QVector<QPoint> polygonPoints = vW->getPolygonPoints();

	if (!polygonPoints.isEmpty())
	{
		QPoint point1_polygon = polygonPoints[0];
		double Sx_polygon = point1_polygon.x();
		double Sy_polygon = point1_polygon.y();

		QVector<QPoint> rotation;
		rotation.append(point1_polygon);
		int size = vW->getPolygonPoints().size();

		for (int i = 1; i < size; i++)
		{
			QPoint polygon_point = polygonPoints[i];
			double x = polygon_point.x();
			double y = polygon_point.y();
			double dx, dy;

			dx = (x - Sx_polygon) * cos(angle) + (y - Sy_polygon) * sin(angle) + Sx_polygon;
			dy = -(x - Sx_polygon) * sin(angle) + (y - Sy_polygon) * cos(angle) + Sy_polygon;
			
			rotation.append(QPoint((int)dx, (int)dy));
		}

		vW->clear();
		vW->drawPolygon(rotation, globalColor, true);
	}
	vW->update();
}

void ImageViewer::on_ScailingpushButton_clicked()
{
	if (!vW) return;

	double Sx = ui->XscaledoubleSpinBox->value();
	double Sy = ui->YscaledoubleSpinBox->value();

	vW->clear();

	// LINE SCALE 
	QPoint p1 = vW->getDrawLineBegin();
	QPoint p2 = vW->getDrawLineEnd();

	double dx = p1.x();
	double dy = p1.y();
	double x = p2.x();
	double y = p2.y();

	double scale_X = dx + (x - dx) * Sx;
	double scale_Y = dy + (y - dy) * Sy;

	QPoint scaled((int)scale_X, (int)scale_Y);

	if (p1 != scaled)
	{
		switch (ui->comboBoxLineAlg->currentIndex())
		{
		case 0:
			vW->drawLineDDA(p1, scaled, globalColor);
			break;
		case 1:
			vW->drawLineBresenham(p1, scaled, globalColor);
			break;
		}
	}

	// POLYGON SCALE 
	QVector<QPoint> polygonPoints = vW->getPolygonPoints();

	if (!polygonPoints.isEmpty())
	{
		QPoint point1_polygon = polygonPoints[0];
		QVector<QPoint> scaled;
		scaled.append(point1_polygon);

		int size = polygonPoints.size();

		for (int i = 1; i < size; ++i)
		{
			double x = polygonPoints[i].x();
			double y = polygonPoints[i].y();

			double scale_X = point1_polygon.x() + (x - point1_polygon.x()) * Sx;
			double scale_Y = point1_polygon.y() + (y - point1_polygon.y()) * Sy;

			scaled.append(QPoint((int)scale_X, (int)scale_Y));
		}

		vW->drawPolygon(scaled, globalColor, true);
	}

	// CIRCLE SCALE
	double scale = (Sx + Sy) / 2.0;
	int newRadius = qRound(vW->getCircleRadius() * scale);

	if (newRadius < 1)
		newRadius = 1;
	
	vW->setCircleRadius(newRadius);
	vW->drawCircleBresenham(vW->getDrawCircleCenter(), newRadius, globalColor);

	vW->update();
}

void ImageViewer::on_SymmetrypushButton_clicked()
{
	if (!vW) return;

	vW->clear();

	// LINE SYMMETRY

	QPoint p1 = vW->getDrawLineBegin();
	QPoint p2 = vW->getDrawLineEnd();

	if (p1 != p2)
	{
		double a, b, c;

		switch (ui->SymmetrycomboBox->currentIndex()) 
		{ 
		case 0:
			a = 0; 
			b = 1; 
			c = -p1.y(); 
			break;
		
		case 1:
			a = 1; 
			b = 0; 
			c = -p1.x(); 
			break;
		}
		
		double d = (a * p1.x() + b * p1.y() + c) / (a * a + b * b);
		p1.setX(qRound(p1.x() - 2 * a * d));
		p1.setY(qRound(p1.y() - 2 * b * d));

		d = (a * p2.x() + b * p2.y() + c) / (a * a + b * b);
		p2.setX(qRound(p2.x() - 2 * a * d));
		p2.setY(qRound(p2.y() - 2 * b * d));

		vW->setDrawLineBegin(p1);
		vW->setDrawLineEnd(p2);

		switch (ui->comboBoxLineAlg->currentIndex())
		{
		case 0:
			vW->drawLineDDA(p1, p2, globalColor);
			break;
		case 1:
			vW->drawLineBresenham(p1, p2, globalColor);
			break;
		}
	}

	// POLYGON SYMETRY
	QVector<QPoint>& polygonPoints = vW->getPolygonPoints();
	int size = polygonPoints.size();

	if (!polygonPoints.isEmpty() && size > 1)
	{
		QPoint p1 = polygonPoints[0];
		QPoint p2 = polygonPoints[1];

		double lenghtX = p2.x() - p1.x();
		double lengthY = p2.y() - p1.y();

		double a = lengthY;
		double b = -lenghtX;
		double c = -a * p1.x() - b * p1.y();

		for (int i = 0; i < size; i++)
		{
			QPoint point = polygonPoints[i];

			double d = (a * point.x() + b * point.y() + c) / (a * a + b * b);
			point.setX(qRound(point.x() - 2 * a * d));
			point.setY(qRound(point.y() - 2 * b * d));

			polygonPoints[i] = point;
		}
		vW->drawPolygon(polygonPoints, globalColor, true);
	}
	vW->update();
}

void ImageViewer::on_SlashpushButton_clicked()
{
	if (!vW) return;

	double value = ui->SlashdoubleSpinBox->value();

	vW->clear();

	// LINE SLASH
	QPoint p1 = vW->getDrawLineBegin();
	QPoint p2 = vW->getDrawLineEnd();

	if (p1 != p2)
	{
		int newX1 = qRound(p1.x() + value * p1.y());
		int newX2 = qRound(p2.x() + value * p2.y());

		vW->setDrawLineBegin(QPoint(newX1, p1.y()));
		vW->setDrawLineEnd(QPoint(newX2, p2.y()));

		vW->drawLine(vW->getDrawLineBegin(), vW->getDrawLineEnd(), globalColor, ui->comboBoxLineAlg->currentIndex());
	}

	// POLYGON SLASH
	QVector<QPoint>& polygonPoints = vW->getPolygonPoints();
	int size = polygonPoints.size();

	if (!polygonPoints.isEmpty())
	{
		for (int i = 0; i < size; ++i) 
		{
			int newX = qRound(polygonPoints[i].x() + value * polygonPoints[i].y());
			polygonPoints[i].setX(newX);
		}
		vW->drawPolygon(polygonPoints, globalColor, true);
	}
	vW->update();
}

void ImageViewer::on_FillpushButton_clicked()
{
	if (!vW) return;

	// POLYGON FILL
	QVector<QPoint>& polygonPoints = vW->getPolygonPoints();
	int size = polygonPoints.size();

	if (!polygonPoints.isEmpty() && size > 3)
	{
		int minY = polygonPoints[0].y();
		int maxY = polygonPoints[0].y();

		for (int i = 0; i < size; i++)
		{
			int point_y = polygonPoints[i].y();
			
			if (point_y > maxY)	maxY = point_y;
			if (point_y < minY) minY = point_y;
		}

		for (int y = minY; y <= maxY; y++)
		{
			QVector<int> edge;

			for (int i = 0; i < size; i++)
			{
				QPoint p1 = polygonPoints[i];
				QPoint p2 = polygonPoints[(i + 1) % size];

				if (p1.y() == p2.y()) continue;

				if ((y >= p1.y() && y < p2.y()) || (y >= p2.y() && y < p1.y()))
				{
					double x = p1.x() + (double)(y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y());
					edge.append(qRound(x));
				}
			}

			int edge_size = edge.size();

			for (int i = 0; i < edge_size - 1; i++)
			{
				for (int j = 0; j < edge_size - i - 1; j++)
				{
					if (edge[j] > edge[j + 1])
					{
						int temp = edge[j];
						edge[j] = edge[j + 1];
						edge[j + 1] = temp;
					}
				}
			}

			for (int k = 0; k + 1 < edge_size; k += 2)
			{
				int start_x = edge[k];
				int end_x = edge[k + 1];

				if (start_x > end_x)
				{
					int temp = start_x;
					start_x = end_x;
					end_x = temp;
				}
			
				for (int x = start_x; x <= end_x; x++)
				{
					vW->setPixel(x, y, globalColor);
				}

			}
		}
		
	}

	// TRIANGLE FILL
	if (size == 3)
	{
		QVector<QPoint> tP = vW->getPolygonPoints();

		QColor c1 = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256));

		QColor c2 = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256));

		QColor c3 = QColor::fromRgb(QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256),
			QRandomGenerator::global()->bounded(256));

		QPoint T1 = tP[0], T2 = tP[1], T3 = tP[2];

		if (T1.y() > T2.y() || (T1.y() == T2.y() && T1.x() > T2.x())) std::swap(T1, T2);
		if (T1.y() > T3.y() || (T1.y() == T3.y() && T1.x() > T3.x())) std::swap(T1, T3);
		if (T2.y() > T3.y() || (T2.y() == T3.y() && T2.x() > T3.x())) std::swap(T2, T3);

		int minY = T1.y();
		int maxY = T3.y();

		double det = (double)(tP[1].y() - tP[2].y()) * (tP[0].x() - tP[2].x()) +
			(double)(tP[2].x() - tP[1].x()) * (tP[0].y() - tP[2].y());

		for (int y = minY; y <= maxY; y++)
		{
			QVector<int> intrs_x;

			for (int i = 0; i < 3; i++) 
			{
				QPoint pA = tP[i];
				QPoint pB = tP[(i + 1) % 3];

				if ((y >= pA.y() && y < pB.y()) || (y >= pB.y() && y < pA.y())) 
				{
					double x = pA.x() + (double)(y - pA.y()) * (pB.x() - pA.x()) / (pB.y() - pA.y());
					intrs_x.append(qRound(x));
				}
			}

			std::sort(intrs_x.begin(), intrs_x.end());

			if (intrs_x.size() >= 2)
			{
				for (int x = intrs_x.first(); x <= intrs_x.last(); x++)
				{
					double alpha = ((double)(tP[1].y() - tP[2].y()) * (x - tP[2].x()) +
						(double)(tP[2].x() - tP[1].x()) * (y - tP[2].y())) / det;

					double beta = ((double)(tP[2].y() - tP[0].y()) * (x - tP[2].x()) +
						(double)(tP[0].x() - tP[2].x()) * (y - tP[2].y())) / det;

					double gamma = 1.0 - alpha - beta;

					int index = ui->FillcomboBox->currentIndex();
					switch (index)
					{
						case 0: // Nearest Neighbor
						{
							if (alpha >= beta && alpha >= gamma) vW->setPixel(x, y, c1);

							else if (beta >= alpha && beta >= gamma) vW->setPixel(x, y, c2);

							else vW->setPixel(x, y, c3);
							break;
						}
						case 1: // Barycentric
						{
							int r = qBound(0, int(alpha * c1.red() + beta * c2.red() + gamma * c3.red()), 255);
							int g = qBound(0, int(alpha * c1.green() + beta * c2.green() + gamma * c3.green()), 255);
							int b = qBound(0, int(alpha * c1.blue() + beta * c2.blue() + gamma * c3.blue()), 255);

							vW->setPixel(x, y, QColor(r, g, b));
							break;
						}
					}
				}
			}
		}
	}
	vW->update();
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

	QString fileFilter = "Image data (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*)";
	QString fileName = QFileDialog::getOpenFileName(this, "Load image", folder, fileFilter);
	if (fileName.isEmpty()) { return; }

	QFileInfo fi(fileName);
	settings.setValue("folder_img_load_path", fi.absoluteDir().absolutePath());

	if (!openImage(fileName)) {
		msgBox.setText("Unable to open image.");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
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
	if (newColor.isValid()) {
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