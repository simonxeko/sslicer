#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <iterator>
#include <list>
#include "math.h"
#define GEOM2D

using namespace std;
/* Point: Alias 2D Point */
class Point{
public:
	double x;
	double y;

	Point(){
		x = 0;
		y = 0;
	}

	Point(double x, double y){
		this->x = x;
		this->y = y;
	}

	bool equals(Point a){
		return ((a.x-this->x)*(a.x-this->x) + (a.y-this->y)*(a.y-this->y))<0.0000000000001;
	}

	bool operator == (const Point& a) const {
	 	return ((a.x-this->x)*(a.x-this->x) + (a.y-this->y)*(a.y-this->y))<0.0000000000001;
	}

	Point operator - (const Point& b){
	 	return Point(this->x-b.x, this->y-b.y);
	}

	Point operator + (const Point& b){
	 	return Point(this->x+b.x, this->y+b.y);
	}

	double operator * (const Point& b){
	 	return this->x * b.y - this->y * b.x;
	}

	Point operator * (const double scale){
	 	return Point(this->x*scale, this->y*scale);
	}

	void print(){
		printf("(%.8f, %.8f)", this->x, this->y);
	}
};

/* Segment: Alias 2D Line, with start point and end point */
class Segment{
public:
	Point a;
	Point b;

	Segment(Point a, Point b, int debug = 0){
		this->a = a;
		this->b = b;
		#ifdef DEBUG_SLICER
		if(a.equals(b)){
			printf("//Segment Building Error %d\n",debug);
		}
		#endif
	}
	
	void swap(){
		Point c = a;
		a = b;
		b = c;
	}

	double calcYfromX(double x){
		return b.y + (x - b.x) * (a.y-b.y) / (a.x-b.x);
	}
	
	void print(){
		printf("(%.8lf,%.8lf)->(%.8lf,%.8lf)\n", a.x,a.y,b.x,b.y);
	}

	void outputSvg(){
		printf("<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" style=\"stroke:rgb(255,0,0);stroke-width:0.1\" />\n", a.x * 10, a.y* 10, b.x* 10, b.y* 10);
	}

	bool intersect(Segment c, Point &ref) {
		Point p = a;
		Point p2 = b;
		Point q = c.a;
		Point q2 = c.b;
		Point r = p2 - p;
		Point s = q2 - q;
		double uNumerator = (q-p) * r;
		double denominator = r * s;

		if (uNumerator == 0 && denominator == 0) {
			// colinear, so do they overlap?
			return ((q.x - p.x < 0) != (q.x - p2.x < 0) != (q2.x - p.x < 0) != (q2.x - p2.x < 0)) || 
				((q.y - p.y < 0) != (q.y - p2.y < 0) != (q2.y - p.y < 0) != (q2.y - p2.y < 0));
		}

		if (denominator == 0) {
			// lines are paralell
			return false;
		}

		double u = uNumerator / denominator;
		double t = ((q-p) * s) / denominator;

		ref = p + r * t;

		return (t >= 0) && (t <= 1) && (u >= 0) && (u <= 1);
	}

};

bool same_segment(Segment &a, Segment &b){
	return (a.a.equals(b.a) && a.b.equals(b.b)) || (a.a.equals(b.b) && a.b.equals(b.a));
}

bool points_sorter_x (const Point &a, const Point &b){ return a.x > b.x; }
bool points_sorter_y (const Point &a, const Point &b){ return a.y > b.y; }

/* Line: Unlimited length 2D Line, y = mx+b */
class Line{
public:
	double m;
	double b;
	bool vertical;
	Line(double m, double b, bool vertical = false){
		this->m = m;
		this->b = b;
		this->vertical = vertical;
	}
	bool intersect(Segment a, Point &ref){
		if(!vertical){
			return Segment(Point(-9999,-9999*m+b), Point(9999,9999*m+b)).intersect(a, ref);
		}else{
			if((a.a.x-b)*(a.b.x-b)<0){
				ref = Point(b, a.calcYfromX(b));
				return true;
			}
			return false;
		}
	}
};

/* Shape: contains groups of 2D Points */
class Shape{
public:
	list<Point> points;
	float z;
	float xmax, xmin, ymax, ymin;

	/* Calculate object size for resizing */
	void calcBoundary(){
		xmax = ymax = 0;
		xmin = ymin = 9999;
		for (list<Point>::iterator it=points.begin(); it != points.end(); ++it){
			if(it->x > this->xmax) this->xmax = it->x;
			if(it->x < this->xmin) this->xmin = it->x;
			if(it->y > this->ymax) this->ymax = it->y;
			if(it->y < this->ymin) this->ymin = it->y;
		}
	}

	void clear(){
		points.clear();
	}

	list<Point> intersect(Line line){
		list<Point>::iterator list_end = points.end();
		list_end--;
		list<Point> result;
		Point ref;
		for(list<Point>::iterator it = points.begin(); it!= list_end; ++it){
			Point b = *it;
			Segment seg = Segment(b,*(++it));
			--it;
			if(line.intersect(seg, ref)){
				result.push_back(ref);
			}
		}
		if(result.size()>2){
			bool horizontal = (result.begin()->y - result.rbegin()->y) == 0;
			if(horizontal){
				result.sort(points_sorter_x);
			}else{
				result.sort(points_sorter_y);
			}
		}
		return result;
	}

	void outputSvg(){
		list<Point>::iterator list_end = points.end();
		list_end--;
		int count = 0;
		printf("<g>\n");
		for(list<Point>::iterator it = points.begin(); it!= list_end; ++it){
			int c = (255 * (count++)) / points.size();
			Point a = *it;
			Point b = *(++it);
			--it;
			printf("<line x1=\"%lf\" y1=\"%lf\" x2=\"%lf\" y2=\"%lf\" style=\"stroke:rgb(255,%d,%d);stroke-width:0.1\" />\n", a.x * 10, a.y* 10, b.x* 10, b.y* 10, c, (int)(z*20));
		}
		printf("</g>\n");
	}

	int size(){
		return points.size();
	}
};

/* Layer: Contains groups of 2D Shape and raw segments */
class Layer{
public:
	list<Shape> shapes;
	list<Segment> segments;
	float z;
	bool bad;
	float xmax, xmin, ymax, ymin;

	Layer(float z){
		this->z = z;
		bad = false;
	}

	/* Calculate object size for resizing */
	void calcBoundary(){
		xmax = ymax = 0;
		xmin = ymin = 9999;
		for (list<Shape>::iterator it=shapes.begin(); it != shapes.end(); ++it){
			it->calcBoundary();
			if(it->xmax > this->xmax) this->xmax = it->xmax;
			if(it->xmin < this->xmin) this->xmin = it->xmin;
			if(it->ymax > this->ymax) this->ymax = it->ymax;
			if(it->ymin < this->ymin) this->ymin = it->ymin;
		}
	}

	void fill(){
		this->calcBoundary();
		for(int z = ymin; z < ymax; z++){
			for(list<Shape>::iterator it = shapes.begin(); it != shapes.end(); it++){
				list<Point> result = it->intersect(Line(0,z));
				for(list<Point>::iterator it2 = result.begin(); it2 != result.end(); it2++){
					Shape s;
					s.points.push_back(*it2);
					s.points.push_back(*(++it2));
					shapes.push_back(s);
				}
			}
		}
		for(int z = xmin; z < xmax; z++){
			for(list<Shape>::iterator it = shapes.begin(); it != shapes.end(); it++){
				list<Point> result = it->intersect(Line(0,z,true));
				for(list<Point>::iterator it2 = result.begin(); it2 != result.end(); it2++){
					Shape s;
					s.points.push_back(*it2);
					if(++it2 != result.end()){
						s.points.push_back(*(it2));
					}else break;
					shapes.push_back(s);
				}
			}
		}
	}

	void push_back(Shape &shape){
		shapes.push_back(shape);
	}

};