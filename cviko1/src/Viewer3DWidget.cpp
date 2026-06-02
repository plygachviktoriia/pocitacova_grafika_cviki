#include "Viewer3DWidget.h"
#include <cmath>
#include <fstream>
#include <QPainter>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Viewer3DWidget::Viewer3DWidget(QSize widgetSize, QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_StaticContents);
	this->resize(widgetSize);
}

Viewer3DWidget::~Viewer3DWidget() {}

// LOAD / SAVE FILE
void Viewer3DWidget::SaveVTK(const std::string& path)
{
	std::ofstream save(path);
	if (!save.is_open()) return;

	save << "# vtk DataFile Version 3.0\n"
		<< "geometry\n"
		<< "ASCII\n"
		<< "DATASET POLYDATA\n"
		<< "POINTS " << vertices.size() << " double\n";

	for (size_t i = 0; i < vertices.size(); ++i)
	{
		const Vertex3D& v = vertices[i];
		save << v.x << " " << v.y << " " << v.z << "\n";
	}

	save << "POLYGONS " << triangles.size() << " " << triangles.size() * 4 << "\n";

	for (size_t i = 0; i < triangles.size(); ++i)
	{
		const Triangle& t = triangles[i];
		save << "3 " << t.v1 << " " << t.v2 << " " << t.v3 << "\n";
	}
}

void Viewer3DWidget::LoadVTK(const std::string& path)
{
	std::ifstream load(path);
	if (!load.is_open()) return;

	vertices.clear();
	triangles.clear();

	std::string word;
	while (load >> word)
	{
		if (word == "POINTS")
		{
			size_t count;
			std::string type;

			load >> count >> type;

			for (size_t i = 0; i < count; i++)
			{
				double x, y, z;
				load >> x >> y >> z;
				vertices.push_back({ x, y, z });
			}
		}
		else if (word == "POLYGONS")
		{
			size_t count;
			size_t total_numbers;

			load >> count >> total_numbers;

			for (size_t i = 0; i < count; i++)
			{
				int vertex_count;
				int v1, v2, v3;

				load >> vertex_count >> v1 >> v2 >> v3;
				triangles.push_back({ v1, v2, v3 });
			}
		}
	}
}

QPointF Viewer3DWidget::revert_3d(const Vertex3D& v)
{
	double x_half = width() / 2.0;
	double y_half = height() / 2.0;
	double scale = 10.0;

	double x2d = x_half + (v.x - v.z) * 0.866 * scale; // 0.866 = cos(pi/6)
	double y2d = y_half - v.y * scale + (v.x + v.z) * 0.5 * scale * 0.5; // 0.5 = sin(pi/6)

	return QPointF(x2d, y2d);
}

// Slots
void Viewer3DWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.fillRect(rect(), Qt::white);

	for (size_t i = 0; i < triangles.size(); ++i) {
		const Triangle& t = triangles[i];

		QPointF p1 = revert_3d(vertices[t.v1]);
		QPointF p2 = revert_3d(vertices[t.v2]);
		QPointF p3 = revert_3d(vertices[t.v3]);

		painter.drawLine(p1, p2);
		painter.drawLine(p2, p3);
		painter.drawLine(p3, p1);
	}
}

// CREATE CUBE
void Viewer3DWidget::create_cube(double size)
{
	cube_size = size;
	double s = size / 2.0;

	vertices = {
		{-s, +s, -s},
		{+s, +s, -s},
		{+s, -s, -s},
		{-s, -s, -s},
		{-s, +s, +s},
		{+s, +s, +s},
		{+s, -s, +s},
		{-s, -s, +s}
	};

	triangles = {
		{0, 1, 2}, {0, 2, 3}, // zadna
		{5, 4, 7}, {5, 7, 6}, // predna
		{4, 0, 3}, {4, 3, 7}, // lava
		{1, 5, 6}, {1, 6, 2}, // prava
		{4, 5, 1}, {4, 1, 0}, // horna
		{3, 2, 6}, {3, 6, 7}  // dolna
	};

	update();
}
