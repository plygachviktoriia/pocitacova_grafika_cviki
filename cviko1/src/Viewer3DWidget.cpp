#include "Viewer3DWidget.h"
#include <cmath>
#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Viewer3DWidget::Viewer3DWidget(ViewerWidget* viewer)
	: vW(viewer)
{
}

Viewer3DWidget::~Viewer3DWidget() {}

// SAVE FILE
void Viewer3DWidget::SaveVTK(const std::string& path)
{
	std::ofstream out(path);
	if (!out.is_open()) return;

	out << "# vtk DataFile Version 3.0\n"
		<< "vtk output\n"
		<< "ASCII\n"
		<< "DATASET POLYDATA\n"
		<< "POINTS " << vertices.size() << " double\n";

	for (size_t i = 0; i < vertices.size(); i++)       // zapis vrcholov
	{
		out << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << "\n";
	}

	out << "POLYGONS " << triangles.size() << " " << triangles.size() * 4 << "\n";

	for (size_t i = 0; i < triangles.size(); i++)     //zapis polygonov
	{
		out << "3 " << triangles[i].v1 << " " << triangles[i].v2 << " " << triangles[i].v3 << "\n";
	}
}

// FILE LOAD
void Viewer3DWidget::LoadVTK(const std::string& path)
{
	std::ifstream in(path);
	if (!in.is_open()) return;

	vertices.clear();
	triangles.clear();

	std::string word;

	while (in >> word)
	{
		if (word == "POINTS")
		{
			int count; 
			std::string type;
			in >> count >> type;        // citanie vrcholov a ich typ
			vertices.resize(count);

			for (int i = 0; i < count; ++i)
			{
				in >> vertices[i].x >> vertices[i].y >> vertices[i].z;     // citanie sur podla kazdeho bodu a zapis vo vektor
			}
		}
		else if (word == "POLYGONS")
		{
			int count, total;
			in >> count >> total;     // citanie polygonov a pocet

			for (int i = 0; i < count; i++)
			{
				int n, v1, v2, v3;
				in >> n >> v1 >> v2 >> v3;    //citanie pocetu vrcholov a indexy
				if (n == 3) triangles.push_back({ v1, v2, v3 });
			}
		}
	}
	vW->update();
}

QPointF Viewer3DWidget::revert_3d(Vertex3D& v)
{
	Vertex3D view = view_coordinates(v);
	Vertex3D proj = projekcia(view);
	return QPointF(proj.x, proj.y);
}

// Slots
void Viewer3DWidget::paint(QColor color)
{
	for (size_t i = 0; i < triangles.size(); ++i) 
	{
		const Triangle& t = triangles[i];

		QPoint p1 = revert_3d(vertices[t.v1]).toPoint();
		QPoint p2 = revert_3d(vertices[t.v2]).toPoint();
		QPoint p3 = revert_3d(vertices[t.v3]).toPoint();

		vW->drawLineDDA(p1, p2, color);
		vW->drawLineDDA(p2, p3, color);
		vW->drawLineDDA(p3, p1, color);
	}
	vW->update();
}

// CREATE CUBE
void Viewer3DWidget::create_cube(double size)
{
	vertices.clear();
	triangles.clear();

	double s = size / 2.0;         // aby on bol odcentrovany

	// zapis vrcholob
	vertices.push_back({ -s, -s, -s });  // 0
	vertices.push_back({ s, -s, -s });  // 1
	vertices.push_back({ s,  s, -s });  // 2
	vertices.push_back({ -s,  s, -s });  // 3
	vertices.push_back({ -s, -s,  s });  // 4
	vertices.push_back({ s, -s,  s });  // 5
	vertices.push_back({ s,  s,  s });  // 6
	vertices.push_back({ -s,  s,  s }); // 7

	// front strana  z = + s
	triangles.push_back({ 4, 5, 6 });
	triangles.push_back({ 4, 6, 7 });

	// zadna strana  z = - s
	triangles.push_back({ 1, 0, 3 });
	triangles.push_back({ 1, 3, 2 });

	// lava strana  x = - s 
	triangles.push_back({ 0, 4, 7 });
	triangles.push_back({ 0, 7, 3 });

	// prava strana  x = + s  
	triangles.push_back({ 5, 1, 2 });
	triangles.push_back({ 5, 2, 6 });

	// horna  y = + s
	triangles.push_back({ 7, 6, 2 });
	triangles.push_back({ 7, 2, 3 });

	//dolna  y = - s
	triangles.push_back({ 0, 1, 5 });
	triangles.push_back({ 0, 5, 4 });

	vW->update();
}

// SPHERE
void Viewer3DWidget::create_sphere(double radius, int medians, int parallels)
{
	vertices.clear();
	triangles.clear();

	for (int i = 0; i <= parallels; i++)
	{
		double theta = i * M_PI / parallels;  

		for (int j = 0; j <= medians; j++)
		{
			double phi = j * 2.0 * M_PI / medians;

			double x = radius * std::sin(theta) * std::cos(phi);
			double y = radius * std::cos(theta);
			double z = radius * std::sin(theta) * std::sin(phi);

			vertices.push_back({ x, y, z });
		}
	}

	int cols = medians + 1;  // kruznici navkolo sphery

	for (int i = 0; i < parallels; i++)
	{
		for (int j = 0; j < medians; j++)
		{
			int top_left = i * cols + j;       // horny lavy bod
			int top_right = i * cols + j + 1;   // horny pravy
			int bottom_left = (i + 1) * cols + j;     // spodny lavy
			int bottom_right = (i + 1) * cols + j + 1;   // spodny pravy

			triangles.push_back({ top_left, bottom_left, top_right });
			triangles.push_back({ top_right, bottom_left, bottom_right });
		}
	}
	vW->update();
}

void Viewer3DWidget::setTetha(double t)
{
	theta = t;

	camera_n.x = std::sin(theta) * std::sin(phi);       // normalovy vektor premetnie
	camera_n.y = std::sin(theta) * std::cos(phi);
	camera_n.z = std::cos(theta);

	camera_u.x = std::sin(theta + M_PI / 2.0) * std::sin(phi);   // orientacia kamery hore
	camera_u.y = std::sin(theta + M_PI / 2.0) * std::cos(phi);
	camera_u.z = std::cos(theta + M_PI / 2.0);

	camera_v.x = camera_u.y * camera_n.z - camera_u.z * camera_n.y;  // sucin  n * u
	camera_v.y = camera_u.z * camera_n.x - camera_u.x * camera_n.z;
	camera_v.z = camera_u.x * camera_n.y - camera_u.y * camera_n.x;

	vW->update();
}

void Viewer3DWidget::setPhi(double p)
{
	phi = p;
	setTetha(theta);
}

void Viewer3DWidget::projectionType(int type)
{
	proj_type = type;
	vW->update();
}

void Viewer3DWidget::setWireframe(bool w) 
{ 
	wireframe = w; 
	vW->update();
}

void Viewer3DWidget::setSz(double sz) 
{ 
	Sz = sz;      
	vW->update();
}

double Viewer3DWidget::scalar(Vertex3D& a, Vertex3D& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vertex3D Viewer3DWidget::view_coordinates(Vertex3D& v)
{
	double x = scalar(v, camera_u);
	double y = scalar(v, camera_v);
	double z = scalar(v, camera_n);

	return { x, y, z };
}

Vertex3D Viewer3DWidget::projekcia(Vertex3D& proj)
{
	double centred_X = vW->getImage()->width() / 2.0;
	double centred_Y = vW->getImage()->height() / 2.0;

	double proj_X, proj_Y, proj_Z;
	if (proj_type == 0)   // rovnobezne pravouhle premetanie do roviny xy
	{
		proj_X = proj.x;
		proj_Y = proj.y;
		proj_Z = proj.z;
	}
	else {              // perspectivne, stred premitanie sur (0, 0, Sz) - proj do roviny z = dz
		double menovatel = Sz - proj.z;
		if (std::abs(menovatel) < 1e-6) menovatel = 1e-6;  // podmienka na bezpecnost delenia 
		
		proj_X = Sz * proj.x / menovatel;
		proj_Y = Sz * proj.y / menovatel;
		proj_Z = proj.z;
	}

	double x_projected = centred_X + proj_X;
	double y_projected = centred_Y - proj_Y;
	double z_projected = proj_Z;

	return Vertex3D(x_projected, y_projected, z_projected);
}

void Viewer3DWidget::buffers()
{
	int w = vW->getImage()->width();
	int h = vW->getImage()->height();

	ZBuffer.resize(h);         // zmena rozmerov bufferov podla obrazobky
	FBuffer.resize(h);
	for (int i = 0; i < h; i++)
	{
		ZBuffer[i].resize(w);
		FBuffer[i].resize(w);
	}

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			ZBuffer[i][j] = -1e18;       // nastavenie na nekonecnost 
			FBuffer[i][j] = Qt::white;   
		}
	}
}