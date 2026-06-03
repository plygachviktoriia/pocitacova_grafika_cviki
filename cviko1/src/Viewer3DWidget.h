#pragma once
#include <QtWidgets/QWidget>
#include <vector>
#include <string>

struct Vertex3D {
	double x, y, z;
};

struct Triangle {
	int v1, v2, v3;
};

class Viewer3DWidget : public QWidget
{
	Q_OBJECT
private:
	QSize areaSize = QSize(0, 0);

	std::vector<Vertex3D> vertices;   // zoznam vrcholov
	std::vector<Triangle> triangles;

	double cube_size = 10.0;
	double sphere_radius = 10.0;

public:
	Viewer3DWidget(QSize widgetSize, QWidget* parent = Q_NULLPTR);
	~Viewer3DWidget();

	void create_cube(double size);
	void create_sphere(double radius, int medians, int parallels);
	QPointF revert_3d(const Vertex3D& v);        //prevod 3d obrazku v 2d

	//SAVE IN FILE
	void SaveVTK(const std::string& path);
	void LoadVTK(const std::string& path);

	//GETTER / SETTER
	std::vector<Vertex3D>& getVertices() { return vertices; }
	std::vector<Triangle>& getTriangles() { return triangles; }

public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};
