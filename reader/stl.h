#ifndef GEOM3D
#include "../geom/geom3d.h"
#endif
#include "stdio.h"
#include "string.h"

/* STL file reader */
Geom* readSTL(const char* filename){
	bool isBinary = false;
	char header[80];
	char buff[2];
	int number_of_triangles;
	FILE *fp = fopen (filename,"rb");
	if (fp==NULL) return NULL;
	fread(header, 1, 80, fp);
	header[6] = 0;
	isBinary = (strcmp(header, "solid")!=0);
	if(!isBinary){
		printf("Plain text not support.\n");
		return NULL;
	}
	fseek(fp, 0, SEEK_SET);
	fread(header, 1, 80, fp);
	fread(&number_of_triangles,4,1,fp);
	Geom* g = new Geom();
	for(int i = 0; i < number_of_triangles; i++){
		Face f;
		fread(&f.n, 4, 3, fp);
		fread(&f.v1, 4, 3, fp);
		fread(&f.v2, 4, 3, fp);
		fread(&f.v3, 4, 3, fp);
		fread(&buff, 1, 2, fp);
		g->faces.push_back(f);
	}
	fclose (fp);
	g->calcBoundary();
	return g;
}