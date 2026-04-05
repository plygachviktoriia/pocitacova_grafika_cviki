#pragma once
#include <QtWidgets>
class ViewerWidget :public QWidget {
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);
	QImage* img = nullptr;
	uchar* data = nullptr;

	// LINE
	bool drawLineActivated = false;
	QPoint drawLineBegin = QPoint(0, 0);
	QPoint drawLineEnd = QPoint(0, 0);

	// CIRCLE
	bool drawCircleActivated = false;
	QPoint drawCircleCenter = QPoint(0, 0);
	int circleRadius = 0;

	//POLYGON
	QVector<QPoint> polygonPoints;
	bool polygonActive = false;
	bool polygonFinished = false;

public:
	ViewerWidget(QSize imgSize, QWidget* parent = Q_NULLPTR);
	~ViewerWidget();
	void resizeWidget(QSize size);

	//Image functions
	bool setImage(const QImage& inputImg);
	QImage* getImage() { return img; };
	bool isEmpty();
	bool changeSize(int width, int height);

	void setPixel(int x, int y, int r, int g, int b, int a = 255);
	void setPixel(int x, int y, double valR, double valG, double valB, double valA = 1.);
	void setPixel(int x, int y, const QColor& color);
	bool isInside(int x, int y);

	//Draw functions
	void drawLine(QPoint start, QPoint end, QColor color, int algType = 0);
	void drawCircleBresenham(QPoint center, int radius, QColor color);
	void clear();

	//POSUVANIE
	bool dragging = false;
	QPoint startDragging;
	QImage backupImg;

	//OTACAVANIE
	bool rotation = false;
	QPoint startRotation;
	double rotationAngle = 0.0;
	QPoint originalLineEnd;
	
	// GET / SET functions
	uchar* getData() { return data; }
	void setDataPtr() { data = img ? img->bits() : nullptr; }

	int getImgWidth() { return img ? img->width() : 0; };
	int getImgHeight() { return img ? img->height() : 0; };

	// Line 
	void setDrawLineBegin(QPoint begin) { drawLineBegin = begin; }
	QPoint getDrawLineBegin() { return drawLineBegin; }

	void setDrawLineActivated(bool state) { drawLineActivated = state; }
	bool getDrawLineActivated() { return drawLineActivated; }

	void setDrawLineEnd(QPoint point) { drawLineEnd = point; }
	QPoint getDrawLineEnd() { return drawLineEnd; }

	// Circle
	void setDrawCircleCenter(QPoint point) { drawCircleCenter = point; }
	QPoint getDrawCircleCenter() { return drawCircleCenter; }

	void setDrawCircleActivated(bool s) { drawCircleActivated = s; }
	bool getDrawCircleActivated() { return drawCircleActivated; }

	void setCircleRadius(int radius) { circleRadius = radius; }
	int getCircleRadius() const { return circleRadius; }

	// Polygon
	void addPolygonPoint(const QPoint& p) { polygonPoints.append(p); }
	bool activePolygon() const { return polygonActive; }
	void setPolygonActive(bool state) { polygonActive = state; }

	QVector<QPoint>& getPolygonPoints() { return polygonPoints; }

	//Algorithms
	void drawLineDDA(QPoint start, QPoint end, QColor color);
	void drawLineBresenham(QPoint start, QPoint end, QColor color);
	
	void drawPolygon(const QVector<QPoint>& polygonPoints, QColor color, bool closePolygon = false);
	void clearPolygon() { polygonPoints.clear(); polygonActive = false; polygonFinished = false; }
	void finishPolygon() { polygonFinished = true; polygonActive = false; }

	void DrawObjects(QColor color, int index);
	void MoveObjects(QPoint delta, int index, QColor color);
	
	void WheelMove(QPoint angel_delta, int index, QColor color);

	void RotationObjects(double angle_rotation, int index, QColor color);

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};