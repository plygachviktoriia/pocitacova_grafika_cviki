#pragma once
#include <QtWidgets/QWidget>
#include <vector>
#include <string>

struct Vertex3D {
	double x, y, z;
	Vertex3D() : x(0), y(0), z(0) {}
	Vertex3D(double x, double y, double z) : x(x), y(y), z(z) {}
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

	// CAMERA
	double theta = M_PI / 4.0;  //zenit
	double phi = M_PI / 4.0;   // azymut

	Vertex3D camera_n = { 0, 0, 1 };
	Vertex3D camera_u = { 1, 0, 0 };
	Vertex3D camera_v = { 0, 1, 0 };

	int  proj_type = 0;
	bool wireframe = true;
	double Sz = 10;

	QVector<QVector<double>> ZBuffer;
	QVector<QVector<QColor>> FBuffer;

public:
	Viewer3DWidget(QSize widgetSize, QWidget* parent = Q_NULLPTR);
	~Viewer3DWidget();

	void create_cube(double size);
	void create_sphere(double radius, int medians, int parallels);
	QPointF revert_3d(Vertex3D& v);        //prevod 3d obrazku v 2d
	double scalar(Vertex3D& a, Vertex3D& b);
	Vertex3D view_coordinates(Vertex3D& v);
	Vertex3D projekcia(Vertex3D& proj);
	void buffers();

	//SAVE IN FILE
	void SaveVTK(const std::string& path);
	void LoadVTK(const std::string& path);

	//GETTER / SETTER
	std::vector<Vertex3D>& getVertices() { return vertices; }
	std::vector<Triangle>& getTriangles() { return triangles; }

	void setTetha(double t);
	void setPhi(double p);
	void projectionType(int type);
	void setWireframe(bool w);
	void setSz(double sz);


public slots:
	void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
};
