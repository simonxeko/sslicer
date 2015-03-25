#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <iterator>
#include <list>
#include "math.h"
#include "geom2d.h"
#define GEOM3D

#define max3(x, y, z) x>y?(x>z?x:z):(y>z?y:z)
#define min3(x, y, z) x<y?(x<z?x:z):(y<z?y:z)

using namespace std;
/* Vertex: Alias 3D Point */
class Vertex{
public:
	float x;
	float y;
	float z;
	bool equals(Vertex &b){
		return this->x == b.x && this->y == b.y && this->z == b.z;
	}
	Point intersect(Vertex &b, double z){
		double x =  (double)b.x + (z - b.z) * ((double)this->x - b.x) / (double)(this->z-b.z);
		double y =  (double)b.y + (z - b.z) * ((double)this->y - b.y) / (double)(this->z-b.z);
		return Point(x,y);
	}
	void print(){
		printf("(%f, %f, %f)", this->x, this->y, this->z);
	}
};
/* Face: Alias 3D Mesh */
class Face{
public:
	Vertex n;
	Vertex v1;
	Vertex v2;
	Vertex v3;
	void print(){
		printf("%f %f %f %f\n", n.z, v1.z, v2.z, v3.z);
	}
	float xmax(){
		return max3(this->v1.x,this->v2.x,this->v3.x);
	}
	float xmin(){
		return min3(this->v1.x,this->v2.x,this->v3.x);
	}
	float ymax(){
		return max3(this->v1.y,this->v2.y,this->v3.y);
	}
	float ymin(){
		return min3(this->v1.y,this->v2.y,this->v3.y);
	}
	float zmax(){
		return max3(this->v1.z,this->v2.z,this->v3.z);
	}
	float zmin(){
		return min3(this->v1.z,this->v2.z,this->v3.z);
	}
};
/* Geom: Alias 3D Object, which contains boundaries, faces and slicing algorithm */
class Geom{
public:
	list<Face> faces;
	float xmax, xmin, ymax, ymin, zmax, zmin;
	/* Calculate object size for resizing */
	void calcBoundary(){
		xmax = ymax = zmax = 0;
		xmin = ymin = zmin = 9999;
		for (list<Face>::iterator it=faces.begin(); it != faces.end(); ++it){
			if(it->xmax() > this->xmax) this->xmax = it->xmax();
			if(it->xmin() < this->xmin) this->xmin = it->xmin();
			if(it->ymax() > this->ymax) this->ymax = it->ymax();
			if(it->ymin() < this->ymin) this->ymin = it->ymin();
			if(it->zmax() > this->zmax) this->zmax = it->zmax();
			if(it->zmin() < this->zmin) this->zmin = it->zmin();
		}
	}
	/* Slice at specific height ( model size ratio ) */
	Layer* slice(float z){
		list<Segment> path;
		Layer *layer = new Layer(z);
		//Iterate all faces, find all intersections of z-plane and faces
		for (list<Face>::iterator it=faces.begin(); it != faces.end(); ++it){
			if( it->zmin() >= z || it->zmax() <= z ) continue;
			Vertex v1 = it->v1;
			Vertex v2 = it->v2;
			Vertex v3 = it->v3;
			float d1 = v1.z - z;
			float d2 = v2.z - z;
			float d3 = v3.z - z;
			// Calculate the intersections of z-plane and face
			if( d1 < 0 && d2 < 0 && d3 > 0 ){
				path.push_back(Segment(v3.intersect(v1,z),v3.intersect(v2,z),1));
			}else if( d1 < 0 && d2 > 0 && d3 < 0 ){
				path.push_back(Segment(v2.intersect(v1,z),v2.intersect(v3,z),2));
			}else if( d1 > 0 && d2 < 0 && d3 < 0 ){
				path.push_back(Segment(v1.intersect(v2,z),v1.intersect(v3,z),3));
			}else if( d1 > 0 && d2 > 0 && d3 < 0 ){
				path.push_back(Segment(v3.intersect(v1,z),v3.intersect(v2,z),4));
			}else if( d1 > 0 && d2 < 0 && d3 > 0 ){
				path.push_back(Segment(v2.intersect(v1,z),v2.intersect(v3,z),5));
			}else if( d1 < 0 && d2 > 0 && d3 > 0 ){
				path.push_back(Segment(v1.intersect(v2,z),v1.intersect(v3,z),6));
			}else if( d1 == 0 && d2 * d3 < 0 ){
				path.push_back(Segment(Point(v1.x,v1.y),v2.intersect(v3,z),7));
			}else if( d2 == 0 && d1 * d3 < 0 ){
				path.push_back(Segment(Point(v2.x,v2.y),v1.intersect(v3,z),8));
			}else if( d3 == 0 && d1 * d2 < 0 ){
				path.push_back(Segment(Point(v3.x,v3.y),v1.intersect(v2,z),9));
			}else if( d1 == 0 && d2 == 0 && d3 != 0 ){
				path.push_back(Segment(Point(v1.x,v1.y),Point(v2.x,v2.y),10));
			}else if( d1 == 0 && d2 != 0 && d3 == 0 ){
				path.push_back(Segment(Point(v1.x,v1.y),Point(v3.x,v3.y),11));
			}else if( d1 != 0 && d2 == 0 && d3 == 0 ){
				path.push_back(Segment(Point(v2.x,v2.y),Point(v3.x,v3.y),12));
			}else{
				printf("SPECIAL FACE!\n");
			}
		}
		//We got all intersections(segment), but we need to sort them
		layer->segments = path;
		//No path available, empty layer
		if(path.size() == 0) return layer;
		bool base_data = false;
		int tolrence = 0;
		Shape shape;
		Segment base(Point(0,1),Point(0,0));
		//Cleaning up the mess data
		for(list<Segment>::iterator it=path.begin(); it!=path.end(); it++){
			if(it->a == it->b){
				path.erase(it);
			}
		}
		path.unique(same_segment);
		//Iterate all segments
		while(path.size()>0){
			bool found = false;
			if(!base_data){
				//Select one segment as "base"
				base_data = true;
				base = *path.begin();
				shape.points.push_back(base.a);
				shape.points.push_back(base.b);
				path.erase(path.begin());
			}
			//Find a segment that is connected to base, and combine them
			for (list<Segment>::iterator it = path.begin(); it != path.end(); it++){
				if(base.a == it->a){
					base.a = it->b;
					shape.points.push_front(base.a);
					it = path.erase(it);
					found = true;
					break;
				}else if(base.a == it->b){
					base.a = it->a;
					shape.points.push_front(base.a);
					it = path.erase(it);
					found = true;
					break;
				}else if(base.b == it->a){
					base.b = it->b;
					shape.points.push_back(base.b);
					it = path.erase(it);
					found = true;
					break;
				}else if(base.b == it->b){
					base.b = it->a;
					shape.points.push_back(base.b);
					it = path.erase(it);
					found = true;
					break;
				}
			}
			//Check if we found a closed loop, if yes, then we found a "closed loop shape"
			if(base.a == base.b && base_data){
				layer->push_back(shape);
				shape.z = z;
				shape.clear();
				base_data = false;
				continue;
			}
			//In case shape is broken, we force it to be closed
			if(!found){
				printf("<!--ERROR, MATCH NOT FOUND #Z=%lf ",z);
				base.a.print();
				base.b.print();
				printf(">\n");
				layer->push_back(shape);
				shape.clear();
				if(tolrence++>5){
					tolrence = 0;
					printf("<!--ERROR, BAD DATA #Z=%lf-->\n",z);
					layer->bad = true;
					break;
				}
				base_data = false;
				continue;
			}
			//Continue to find next connected segment :D
		}
		if(path.size()==0){
			//In case shape is not closed, we force it to be closed
			if(shape.size()>0){
				printf("<!--ERROR, SHAPE NOT CLOSED #Z=%lf -->\n",z);
				layer->push_back(shape);
				shape.clear();
			}
		}
		return layer;
	}
};
