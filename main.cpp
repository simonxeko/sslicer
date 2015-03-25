#define LAYER_HEIGHT 0.5 //0.5mm
#define CUT_BOTTOM 1 //1mm
#define OBJECT_HEIGHT (30-CUT_BOTTOM) //30mm
//#define DEBUG_SLICER
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <iterator>
#include <list>
#include "geom/geom3d.h"
#include "reader/stl.h"
#include "math.h"

int main(int argc, char const *argv[])
{
	/* Load file and settings */
	Geom *g = readSTL("../util/Octopus.stl");
	if(!g) return -1;
	int segments_total = 1 + OBJECT_HEIGHT / LAYER_HEIGHT;
	double per_height = (g->zmax - g->zmin - CUT_BOTTOM) / ((double)(segments_total));
	printf("<!--Successfully loaded.-->\n");
	printf("<!--Zmax = %f, Zmin = %f, Total Layers = %d-->\n", g->zmax, g->zmin, segments_total);
	/* Initial layers */
	list<Layer*> layers;
	for(int i = 0; i < segments_total; i++){
		layers.push_back(g->slice(g->zmin+(i+CUT_BOTTOM/LAYER_HEIGHT)*per_height));
	}
	/* Output svg ( for debug ) */
	printf("<svg width=\"100\" height=\"100\">\n");
	for(list<Layer*>::iterator it = layers.begin(); it != layers.end(); it++){
		Layer* l = *it;
		l->fill();
		for(list<Shape>::iterator it2 = l->shapes.begin(); it2 != l->shapes.end(); it2++){
		 	it2->outputSvg();
		}
	}
	printf("</svg>\n");
	printf("<!--Finished-->\n");
	return 0;
}