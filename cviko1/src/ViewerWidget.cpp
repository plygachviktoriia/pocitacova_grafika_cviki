#include   "ViewerWidget.h"
#include <cmath>

ViewerWidget::ViewerWidget(QSize imgSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	setMouseTracking(true);
	if (imgSize != QSize(0, 0)) {
		img = new QImage(imgSize, QImage::Format_ARGB32);
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
	}
}
ViewerWidget::~ViewerWidget()
{
	delete img;
	img = nullptr;
	data = nullptr;
}
void ViewerWidget::resizeWidget(QSize size)
{
	this->resize(size);
	this->setMinimumSize(size);
	this->setMaximumSize(size);
}

//Image functions
bool ViewerWidget::setImage(const QImage& inputImg)
{
	if (img) {
		delete img;
		img = nullptr;
		data = nullptr;
	}
	img = new QImage(inputImg.convertToFormat(QImage::Format_ARGB32));
	if (!img || img->isNull()) {
		return false;
	}
	resizeWidget(img->size());
	setDataPtr();
	update();

	return true;
}
bool ViewerWidget::isEmpty()
{
	if (img == nullptr) {
		return true;
	}

	if (img->size() == QSize(0, 0)) {
		return true;
	}
	return false;
}

bool ViewerWidget::changeSize(int width, int height)
{
	QSize newSize(width, height);

	if (newSize != QSize(0, 0)) {
		if (img != nullptr) {
			delete img;
		}

		img = new QImage(newSize, QImage::Format_ARGB32);
		if (!img || img->isNull()) {
			return false;
		}
		img->fill(Qt::white);
		resizeWidget(img->size());
		setDataPtr();
		update();
	}

	return true;
}

void ViewerWidget::setPixel(int x, int y, int r, int g, int b, int a)
{
	if (!img || !data) return;
	if (!isInside(x, y)) return;

	r = r > 255 ? 255 : (r < 0 ? 0 : r);
	g = g > 255 ? 255 : (g < 0 ? 0 : g);
	b = b > 255 ? 255 : (b < 0 ? 0 : b);
	a = a > 255 ? 255 : (a < 0 ? 0 : a);

	size_t startbyte = y * img->bytesPerLine() + x * 4;
	data[startbyte] = static_cast<uchar>(b);
	data[startbyte + 1] = static_cast<uchar>(g);
	data[startbyte + 2] = static_cast<uchar>(r);
	data[startbyte + 3] = static_cast<uchar>(a);
}
void ViewerWidget::setPixel(int x, int y, double valR, double valG, double valB, double valA)
{
	valR = valR > 1 ? 1 : (valR < 0 ? 0 : valR);
	valG = valG > 1 ? 1 : (valG < 0 ? 0 : valG);
	valB = valB > 1 ? 1 : (valB < 0 ? 0 : valB);
	valA = valA > 1 ? 1 : (valA < 0 ? 0 : valA);

	setPixel(x, y, static_cast<int>(255 * valR + 0.5), static_cast<int>(255 * valG + 0.5), static_cast<int>(255 * valB + 0.5), static_cast<int>(255 * valA + 0.5));
}
void ViewerWidget::setPixel(int x, int y, const QColor& color)
{
	if (color.isValid()) {
		setPixel(x, y, color.red(), color.green(), color.blue(), color.alpha());
	}
}

bool ViewerWidget::isInside(int x, int y)
{
	return img && x >= 0 && y >= 0 && x < img->width() && y < img->height();
}

//Draw functions
void ViewerWidget::drawLine(QPoint start, QPoint end, QColor color, int algType)
{
	if (!img || !data) return;

	double tE = 0.0, tL = 1.0;
	
	double dx = (double)end.x() - start.x();
	double dy = (double)end.y() - start.y();

	// 0: left, 1: right, 2: up, 3: down
	QVector<QPoint> normals = { QPoint(1, 0), QPoint(-1, 0), QPoint(0, 1), QPoint(0, -1) };
	QVector<QPoint> pointsOnEdges = { QPoint(0, 0), QPoint(img->width() - 1, 0), QPoint(0, 0), QPoint(0, img->height() - 1) };

	for (int i = 0; i < 4; i++)
	{
		double dn = dx * normals[i].x() + dy * normals[i].y();
		double wn = (start.x() - pointsOnEdges[i].x()) * normals[i].x() + (start.y() - pointsOnEdges[i].y()) * normals[i].y();

		if (dn != 0) 
		{
			double t = -wn / dn;

			if (dn > 0) 
			{ 
				if (t > tE) tE = t;
			}
			else {
				if (t < tL) tL = t;
			}
		}
		else {
			if (wn < 0) return; 
		}
	}

	if (tE <= tL)
	{
		QPoint clippedStart(qRound(start.x() + tE * dx), qRound(start.y() + tE * dy));
		QPoint clippedEnd(qRound(start.x() + tL * dx), qRound(start.y() + tL * dy));

		if (algType == 0) drawLineDDA(clippedStart, clippedEnd, color);
		else drawLineBresenham(clippedStart, clippedEnd, color);
	}
	update();
}

void ViewerWidget::clear()
{
	if (!img) return;
	img->fill(Qt::white);
	update();
}

void ViewerWidget::drawLineDDA(QPoint start, QPoint end, QColor color)
{
	double x = start.x(), y = start.y();
	double dx = end.x() - x, dy = end.y() - y;

	double maxDif = std::max(std::abs(dx), std::abs(dy));

	if (maxDif == 0) 
	{ 
		setPixel(int(x + 0.5), int(y + 0.5), color); 
		return;
	}

	double x_inc = dx / maxDif, y_inc = dy / maxDif;

	for (int i = 0; i <= maxDif; i++) 
	{
		setPixel(int(x + 0.5), int(y + 0.5), color);
		x += x_inc; 
		y += y_inc;
	}
}

void ViewerWidget::drawLineBresenham(QPoint start, QPoint end, QColor color)
{
	int x = start.x(), y = start.y();
	int dx = abs(end.x() - x), dy = abs(end.y() - y);
	int sx = (x < end.x()) ? 1 : -1;
	int sy = (y < end.y()) ? 1 : -1;

	int err = dx - dy;

	while (true) {
		setPixel(x, y, color);
		if (x == end.x() && y == end.y()) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x += sx; }
		if (e2 < dx) { err += dx; y += sy; }
	}
}

void ViewerWidget::drawCircleBresenham(QPoint center, int radius, QColor color)
{
	int x = 0, y = radius;
	int p = 1 - radius;

	while (x <= y) 
	{
		setPixel(center.x() + x, center.y() + y, color);
		setPixel(center.x() - x, center.y() + y, color);
		setPixel(center.x() + x, center.y() - y, color);
		setPixel(center.x() - x, center.y() - y, color);

		setPixel(center.x() + y, center.y() + x, color);
		setPixel(center.x() - y, center.y() + x, color);
		setPixel(center.x() + y, center.y() - x, color);
		setPixel(center.x() - y, center.y() - x, color);

		if (p > 0) 
		{ 
			p += -2 * y + 2 * x + 5; 
			y--; 
		}
		else { p += 2 * x + 3; }
		x++;
	}
	update();
}

void ViewerWidget::drawPolygon(const QVector<QPoint>& points, QColor color, bool closePolygon)
{
	int n = points.size();
	if (n < 2)
		return;

	QVector<QPoint> output = points;
	int w = img->width() - 1;
	int h = img->height() - 1;

	for (int edge = 0; edge < 4; edge++) 
	{
		QVector<QPoint> input = output;
		output.clear();
		if (input.isEmpty()) break;
		
		int size = input.size();

		for (int i = 0; i < size; i++) 
		{
			QPoint p1 = input[i];
			QPoint p2 = input[(i + 1) % size];

			bool p1Inside = false;
			bool p2Inside = false;

			if (edge == 0) { p1Inside = (p1.x() >= 0); p2Inside = (p2.x() >= 0); }
			else if (edge == 1) { p1Inside = (p1.x() <= w); p2Inside = (p2.x() <= w); }
			else if (edge == 2) { p1Inside = (p1.y() >= 0); p2Inside = (p2.y() >= 0); }
			else if (edge == 3) { p1Inside = (p1.y() <= h); p2Inside = (p2.y() <= h); }

			if (p2Inside) {
				if (!p1Inside) 
				{
					double x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
					double ix, iy;

					if (edge == 0) 
					{ 
						iy = y1 + (y2 - y1) * (0 - x1) / (x2 - x1); 
						ix = 0; 
					}
					else if (edge == 1) 
					{ 
						iy = y1 + (y2 - y1) * (w - x1) / (x2 - x1); 
						ix = w; 
					}
					else if (edge == 2) 
					{ 
						ix = x1 + (x2 - x1) * (0 - y1) / (y2 - y1); 
						iy = 0; 
					}
					else {
						ix = x1 + (x2 - x1) * (h - y1) / (y2 - y1); 
						iy = h; 
					}

					output.append(QPoint(qRound(ix), qRound(iy)));
				}
				output.append(p2);
			}
			else if (p1Inside) 
			{
		
				double x1 = p1.x(), y1 = p1.y(), x2 = p2.x(), y2 = p2.y();
				double ix, iy;

				if (edge == 0) 
				{ 
					iy = y1 + (y2 - y1) * (0 - x1) / (x2 - x1); 
					ix = 0; 
				}
				else if (edge == 1) 
				{
					iy = y1 + (y2 - y1) * (w - x1) / (x2 - x1); 
					ix = w; 
				}
				else if (edge == 2) 
				{ 
					ix = x1 + (x2 - x1) * (0 - y1) / (y2 - y1); 
					iy = 0; 
				}
				else { 
					ix = x1 + (x2 - x1) * (h - y1) / (y2 - y1); 
					iy = h; 
				}
				output.append(QPoint(qRound(ix), qRound(iy)));
			}
		}
	}

	if (output.size() >= 2) {
		for (int i = 0; i < output.size() - 1; i++)
		{
			drawLineDDA(output[i], output[i + 1], color);
		}
		if (closePolygon && output.size() >= 3) 
		{
			drawLineDDA(output.last(), output.first(), color);
		}
	}
	update();
}

void ViewerWidget::DrawObjects(QColor color, int index)
{
	switch (index)
	{
	case 0:
		drawLineDDA(getDrawLineBegin(), getDrawLineEnd(), color);
		break;

	case 1:
		drawLineBresenham(getDrawLineBegin(), getDrawLineEnd(), color);
		break;
	}

	drawCircleBresenham(getDrawCircleCenter(), getCircleRadius(), color);

	drawPolygon(polygonPoints, color, true);
}

void ViewerWidget::MoveObjects(QPoint delta, int index, QColor color)
{
	// LINE
	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	QPoint p1_new = p1 + delta;
	QPoint p2_new = p2 + delta;

	setDrawLineBegin(p1_new);
	setDrawLineEnd(p2_new);

	// CIRCLE 
	QPoint center = getDrawCircleCenter();
	QPoint center_new = center + delta;

	setDrawCircleCenter(center_new);

	// POLYGON 
	QVector<QPoint>& polygonPoints = getPolygonPoints();
	int size = polygonPoints.size();

	for (int i = 0; i < size; i++)
	{
		polygonPoints[i] += delta;
	}

	clear();
	DrawObjects(color, index);
	update();
}

void ViewerWidget::WheelMove(QPoint angle_delta, int index, QColor color)
{
	//LINE
	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	double scale_unit = (angle_delta.y() > 0) ? 1.25 : 0.75;    //vertikal 
	double X = p1.x() + (p2.x() - p1.x()) * scale_unit;
	double Y = p1.y() + (p2.y() - p1.y()) * scale_unit;

	QPoint scaledLine(qRound(X), qRound(Y));
	setDrawLineEnd(scaledLine);

	//CIRCLE
	int radius_new = qRound(getCircleRadius() * scale_unit);
	if (radius_new < 1) radius_new = 1;
	setCircleRadius(radius_new);

	//POLYGON
	QVector<QPoint> polygonPoints = getPolygonPoints();
	QPointF point1 = polygonPoints[0];

	int size = polygonPoints.size();

	for (int i = 1; i < size; i++)
	{
		QPoint point = polygonPoints[i];

		double X = point1.x() + (point.x() - point1.x()) * scale_unit;
		double Y = point1.y() + (point.y() - point1.y()) * scale_unit;
		polygonPoints[i] = QPoint(qRound(X), qRound(Y));
	}
	getPolygonPoints() = polygonPoints;

	clear();
	DrawObjects(color, index);
	update();
}

void ViewerWidget::RotationObjects(double angle_rotation, int index, QColor color)
{
	double angle = angle_rotation * M_PI / 180.0;

	clear();

	//LINE 
	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	double Sx = p1.x(), Sy = p1.y();
	double x = p2.x(), y = p2.y();

	double dx = (x - Sx) * cos(angle) + (y - Sy) * sin(angle) + Sx;
	double dy = -(x - Sx) * sin(angle) + (y - Sy) * cos(angle) + Sy;

	QPoint rotatedLineEnd(qRound(dx), qRound(dy));
	setDrawLineEnd(rotatedLineEnd);

	//POLYGON
	QVector<QPoint>& polygonPoints = getPolygonPoints();

	if (!polygonPoints.isEmpty()) 
	{
		QPoint point1_polygon = polygonPoints[0];

		double Sx_polygon = point1_polygon.x();
		double Sy_polygon = point1_polygon.y();

		int size = getPolygonPoints().size();

		for (int i = 1; i < size; i++)
		{
			QPoint polygon_point = polygonPoints[i];
			double x = polygon_point.x(), y = polygon_point.y();

			double dx = (x - Sx_polygon) * cos(angle) + (y - Sy_polygon) * sin(angle) + Sx_polygon;
			double dy = -(x - Sx_polygon) * sin(angle) + (y - Sy_polygon) * cos(angle) + Sy_polygon;

			polygonPoints[i] = QPoint(qRound(dx), qRound(dy));
		}
	}

	DrawObjects(color, index);
	update();
}

void ViewerWidget::ScaleObjects(double X_value, double Y_value, int index, QColor color)
{
	double Sx = X_value;
	double Sy = Y_value;

	clear();

	//LINE SCALE 
	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	double x0 = p1.x(), y0 = p1.y();
	double x = p2.x(), y = p2.y();

	double scale_X = x0 + (x - x0) * Sx;
	double scale_Y = y0 + (y - y0) * Sy;

	QPoint scaled(qRound(scale_X), qRound(scale_Y));
	setDrawLineEnd(scaled);

	// POLYGON SCALE 
	QVector<QPoint>& polygonPoints = getPolygonPoints();

	if (!polygonPoints.isEmpty())
	{
		QPoint point1_polygon = polygonPoints[0];
		int size = polygonPoints.size();

		for (int i = 1; i < size; ++i)
		{
			double x = polygonPoints[i].x();
			double y = polygonPoints[i].y();

			double scale_X = point1_polygon.x() + (x - point1_polygon.x()) * Sx;
			double scale_Y = point1_polygon.y() + (y - point1_polygon.y()) * Sy;

			polygonPoints[i] = QPoint(qRound(scale_X), qRound(scale_Y));
		}
	}
	DrawObjects(color, index);
	update();	
}

void ViewerWidget::SymmetryObjects(int symmetry_index, int index, QColor color)
{
	// LINE SYMMETRY

	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	if (p1 != p2)
	{
		double a, b, c;

		switch (symmetry_index)
		{
			case 0:       
				a = 0;   // ked' x = 0 -> a = 0
				b = 1;
				c = -p1.y();
				break;

			case 1:
				a = 1;
				b = 0;
				c = -p1.x();
				break;
		}

		double d1 = (a * p1.x() + b * p1.y() + c) / (a * a + b * b);
		p1.setX(qRound(p1.x() - 2 * a * d1));
		p1.setY(qRound(p1.y() - 2 * b * d1));

		double d2 = (a * p2.x() + b * p2.y() + c) / (a * a + b * b);
		p2.setX(qRound(p2.x() - 2 * a * d2));
		p2.setY(qRound(p2.y() - 2 * b * d2));

		setDrawLineBegin(p1);
		setDrawLineEnd(p2);
	}

	// POLYGON SYMETRY
	QVector<QPoint>& polygonPoints = getPolygonPoints();
	int size = polygonPoints.size();

	if (!polygonPoints.isEmpty() && size > 1)
	{
		QPoint p1 = polygonPoints[0];
		QPoint p2 = polygonPoints[1];

		double u = p2.x() - p1.x();
		double v = p2.y() - p1.y();

		double a = v;
		double b = -u;
		double c = -a * p1.x() - b * p1.y();

		for (int i = 0; i < size; i++)
		{
			QPoint point = polygonPoints[i];

			double d = (a * point.x() + b * point.y() + c) / (a * a + b * b);
			point.setX(qRound(point.x() - 2 * a * d));
			point.setY(qRound(point.y() - 2 * b * d));

			polygonPoints[i] = point;
		}
	}
	clear();
	DrawObjects(color, index);
	update();
}

void ViewerWidget::SlashObjects(double value, int index, QColor color)
{
	// LINE SLASH
	QPoint p1 = getDrawLineBegin();
	QPoint p2 = getDrawLineEnd();

	double x0_new = p1.x() + value * p1.y();
	double x1_new = p2.x() + value * p2.y();

	QPoint slash_x0(qRound(x0_new), p1.y());
	QPoint slash_x1(qRound(x1_new), p2.y());

	setDrawLineBegin(slash_x0);
	setDrawLineEnd(slash_x1);

	// POLYGON SLASH
	QVector<QPoint>& polygonPoints = getPolygonPoints();
	int size = polygonPoints.size();

	if (!polygonPoints.isEmpty())
	{
		for (int i = 0; i < size; ++i)
		{
			int newX = qRound(polygonPoints[i].x() + value * polygonPoints[i].y());
			polygonPoints[i].setX(newX);
		}
	}
	clear();
	DrawObjects(color, index);
	update();
}

struct Side{          //vytvorenie structury podlia ukladanie udajov o stranach aby mohla ich odsortorovat
	int delta_y;      
	double x;      
	double w;
	double start_y;
};

void ViewerWidget::ScanLine(QColor color)
{
	//POlYGON 
	QVector<QPoint>& polygonPoints = getPolygonPoints();
	int size = polygonPoints.size();

	if (size > 3)
	{
		double y_min = polygonPoints[0].y();
		double y_max = polygonPoints[0].y();

		for (int i = 1; i < size; i++)
		{
			double point = polygonPoints[i].y();
			if (point < y_min)
			{
				y_min = point;
			}
			if (point > y_max)
			{
				y_max = point;
			}
		}

		QVector<Side> allSides;  

		for (int i = 0; i < size; i++)
		{
			QPoint z = polygonPoints[i];                   //zaciatocny bod pre hranu
			QPoint k = polygonPoints[(i + 1) % size];      // koncovy bod pre hranu (% size nadobne na to aby tento bod ne prechadzal rozmier)
			if (z.y() == k.y()) continue;                  // nic nie robimo s horizontal

			if (z.y() > k.y())
			{
				std::swap(z, k);
			}
			
			double dx = k.x() - z.x();
			double dy = k.y() - z.y();
			double m = 0.0, w = 0.0;

			if (dx != 0)
			{
				m = (k.y() - z.y()) / (k.x() - z.x());
				w = 1 / m;
			}
			else {
				w = 0.0;
			}
	
			double y_delta = (k.y() - 1) - z.y();
			if (y_delta == 0) continue;

			Side side;

			side.delta_y = y_delta;
			side.w = w;
			side.x = z.x();
			side.start_y = z.y();
			
			allSides.append(side);
		}
		
		int size_side = allSides.size();
		for (int i = 0; i < size_side; i++)
		{
			for (int j = 0; j < size_side - 1; j++)
			{
				if (allSides[j].start_y > allSides[j + 1].start_y)
				{
					Side t = allSides[j];
					allSides[j] = allSides[j + 1];
					allSides[j + 1] = t;
				}
			}
		}

		double miny = allSides[0].start_y;
		double maxy = y_max;
		int size_table = maxy - miny;

		QVector<QVector<Side>> TH(size_table);

		for (int i = 0; i < size_side; i++)
		{
			Side cur_side = allSides[i];
			int index = cur_side.start_y - miny;
			TH[index].append(cur_side);
		}
	}
}

//Slots
void ViewerWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	if (!img || img->isNull()) return;

	QRect area = event->rect();
	painter.drawImage(area, *img, area);
}

