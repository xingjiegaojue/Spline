#include "spline.h"


void SplineABT::add(const QPoint &p)
{
	if (finished)
		return;
	points.push_back(p);

	_add(p);
}

void SplineABT::finish()
{
	genSpline();
	finished = true;
}

void CardinalSpline::draw(QPaintDevice* pd, const Options& opt)
{
	SplineABT::draw(pd, opt);

	QPainter pir(pd);
	pir.setRenderHint(QPainter::Antialiasing);
	for (int i = 0; i < size(); ++i)
		pir.drawEllipse((*this)[i], 2, 2);

	pir.drawPath(path);
}

int SplineABT::size()
{
	return points.size();
}

const QPoint & SplineABT::at(int index)
{
	Q_ASSERT(index < points.size());
	return points.at(index);
}

int SplineABT::at(const QPoint &p)
{
	for (int i = 0; i < size(); ++i) {
		auto scope = QRect((*this)[i] - QPoint(4, 4), QSize(8, 8));
		if (scope.contains(p))
			return i;
	}
	return -1;
}

QPoint &SplineABT::operator[](int index)
{
	Q_ASSERT(index < points.size());
	return points[index];
}

void SplineABT::clear()
{
	finished = false;
	points.clear();
	path.swap(QPainterPath());
}

void SplineABT::genSpline()
{
	Path();
}

void CardinalSpline::Path()
{
	if (size() > 1) {
		path.swap(QPainterPath());
		path.moveTo(points[1]);
		for (int i = 2; i < size() - 1; i++) {
			for (int j = 0; j < opts.interpolation; j++) {
				float u = (float)j / opts.interpolation;
				path.lineTo(xy(i - 1, u));
			}
		}
	}
}

void CardinalSpline::_add(const QPoint & p)
{
	if (opts.real_paint) {
		if(points.size() > 2)
			points.erase(points.end() - 2);
		if (points.size() == 1)
			points.push_back(p);
		points.push_back(points.back());
		Path();
	}
}

void CardinalSpline::genSpline()
{
	if (!points.isEmpty() && !finished && !opts.real_paint) {
		points.push_front(points.front());
		points.push_back(points.back());
	}

	SplineABT::genSpline();
}

QPoint CardinalSpline::xy(int i, float u)
{
	float s = (1 - opts.tension) / 2;
	float a = -s * u  *u * u + 2 * s * u * u - s * u;
	float b = (2 - s)*u*u*u + (s - 3)*u*u + 1;
	float c = (s - 2) * u*u*u + (3 - 2 * s)*u*u + s*u;
	float d = s*u*u*u - s*u*u;
	float x = points[i - 1].x() * a + points[i].x() * b + points[i + 1].x() * c + points[i + 2].x()*d;
	float y = points[i - 1].y() * a + points[i].y() * b + points[i + 1].y() * c + points[i + 2].y()*d;

	return QPoint(x, y);
}


BezierSpline::BezierSpline()
{
}

BezierSpline::~BezierSpline()
{
}

void BezierSpline::draw(QPaintDevice *pd, const Options &opt)
{
    SplineABT::draw(pd, opt);

    QPainter pir(pd);
    pir.setRenderHint(QPainter::Antialiasing);
    for (int i = 0; i < size(); ++i)
        pir.drawEllipse((*this)[i], 2, 2);

    pir.drawPath(path);
}

void BezierSpline::Path()
{
	int pt_size = (int)points.size();
	if (pt_size == 0) return;

	int n = pt_size - 1;
	auto cb = computeCoeff(n);

    if (points.size() > 1) {
        path.swap(QPainterPath());
        path.moveTo(points[0]);
		QVector<QPointF> tp;
        for (int j = 0; j <= opts.interpolation; j++) {
			float u = (float)j / opts.interpolation;
            float x = 0.0, y = 0.0;
            for (int k = 0; k < pt_size; k++) {
                float bezier = cb[k] * std::pow(u, k) * std::pow(1 - u, n - k);
                x += points[k].x() * bezier;
                y += points[k].y() * bezier;
            }
            path.lineTo(x, y);
			tp.push_back(QPointF(x, y));
        }
		tp.size();
    }
}

void BezierSpline::_add(const QPoint &p)
{
	if (opts.real_paint)
		Path();
}

QVector<float> BezierSpline::computeCoeff(int n)
{
	QVector<float> cb(n + 1, 0);
	for (int k = 0; k <= n; k++)
		cb[k] = combCoeff(n, k);

	return cb;
}

float BezierSpline::combCoeff(int n, int k)
{
	if (k == 0) return 1;

	return ((n - k + 1) / (float)k) * combCoeff(n, k - 1);
}


////////////////////
SplineABT* SplineBuilder::Build(Options::SplineType t)
{
	switch (t)
	{
	case Options::Cardinal:
		return new CardinalSpline();
	case Options::Bezier:
		return new BezierSpline();
	}

	return nullptr;
}