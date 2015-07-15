#define LAYER_HEIGHT 0.3
#define CUT_BOTTOM 5
#define OFFSET_Z 10
#define OBJECT_HEIGHT (76-CUT_BOTTOM) //30mm
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
	Geom *g = readSTL("bunny.stl");
	if(!g){
		printf("Format error, program terminated.\n");
		return -1;	
	} 
	int segments_total = 1 + OBJECT_HEIGHT / LAYER_HEIGHT;
	double per_height = (g->zmax - g->zmin - CUT_BOTTOM) / ((double)(segments_total));
	//printf(";Successfully loaded.");
	//printf(";Zmax = %f, Zmin = %f, Total Layers = %d-->\n", g->zmax, g->zmin, segments_total);
	//return 0;
	list<Layer*> layers;
	for(int i = 0; i < segments_total; i++){
		layers.push_back(g->slice(g->zmin+(i+CUT_BOTTOM/LAYER_HEIGHT)*per_height));
	}
	#ifndef SVGOUT
	printf("M109 S190\nG28\nG29\nG1 F1200\n");
	double extrusion = 0;
	for(list<Layer*>::iterator it = layers.begin(); it != layers.end(); it++){
		Layer* l = *it;
		printf("G1 F7000 Z%lf\n", l->z-CUT_BOTTOM-0.05-OFFSET_Z);
		if(extrusion == 0){ //first layer, two time.
            printf(";First layer start\n");
            l->fillAll();
			for(list<Shape>::iterator it2 = l->shapes.begin(); it2 != l->shapes.end(); it2++){
                if(it2->type == SHAPE_SHELL){
                    it2->compact();
                    it2->outputGcode(extrusion, 2000 ,2);
                }else if(it2->type == SHAPE_FILL){
                    it2->outputGcode(extrusion, 9000, 2);
                }else if(it2->type == SHAPE_BOTTOM){
                    it2->outputGcode(extrusion, 1000, 2);
                }
			}
            printf(";First layer end\n");
		}
		l->fill();
		printf("G1 F7000 Z%lf\n", l->z-CUT_BOTTOM-OFFSET_Z);
		for(list<Shape>::iterator it2 = l->shapes.begin(); it2 != l->shapes.end(); it2++){
            if(it2->type == SHAPE_SHELL){
                it2->compact();
                it2->outputGcode(extrusion, l->z>2.0 ? 8000 : 4000, l->z>1 ? 1 : 2);
            }else if(it2->type == SHAPE_FILL){
                it2->outputGcode(extrusion, 9000, l->z>1 ? 1 : 2);
            }else if(it2->type == SHAPE_BOTTOM){
                it2->outputGcode(extrusion, 1200, 2);
            }
		}
	}
	printf("G28\n");
	#endif
	#ifdef SVGOUT
	/* Output svg ( for debug ) */
	printf("<svg width=\"100\" height=\"100\">\n");
	for(list<Layer*>::iterator it = layers.begin(); it != layers.end(); it++){
		Layer* l = *it;
		l->fill();
		for(list<Shape>::iterator it2 = l->shapes.begin(); it2 != l->shapes.end(); it2++){
			it2->compact();
		 	it2->outputSvg();
		}
	}
	printf("</svg>\n");
	printf("<!--Finished-->\n");
	#endif
	return 0;
}