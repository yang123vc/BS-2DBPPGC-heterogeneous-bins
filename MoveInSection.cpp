

#include "classes_BPGC_HetBins.h"

double calculate_rotation_match (EDGES &e1,EDGES &e2);

bool edge_of_section(EDGES ed, IRR_BIN ib)
{
	bool is_edge = false;
	vector<PUNTO> ib_ptos = *ib.get_ptos();
	//Create all edges of section
	vector<EDGES> ib_ar;
	EDGES aux;
	unsigned long last = ib_ptos.size();
	//Create edges of IRRBIN
	for(int i = 0; i<last; i++)
		{
		aux.ini = ib_ptos[i];
		if(i+1!= last) aux.fin = ib_ptos[i+1];
		else aux.fin = ib_ptos[0];
		aux.set_mod();
		ib_ar.push_back(aux);
		}
	//If ed.ini and ed.fin is aligned with ib.ini and ib.fin then is_edge = true 
	for(int i = 0; i<last; i++)
		{
		if(Position(ib_ar[i].ini, ib_ar[i].fin, ed.ini) == 0 &&
		   Position(ib_ar[i].ini, ib_ar[i].fin, ed.fin) == 0 ) return true;
		}
	return is_edge;
}

void trans_piece_fin_ini(PIEZA &item2, EDGES e1, EDGES e2)
{
	//Calculate translation:
	double trasX = e1.ini.coordx - e2.fin.coordx;
	double trasY = e1.ini.coordy - e2.fin.coordy;
	//Translate all points in P2 to vertex1
	item2.translate(trasX, trasY);
}
void trans_piece_ini_fin(PIEZA &item2, EDGES e1, EDGES e2)
{
	//Calculate translation:
	double trasX = e1.fin.coordx - e2.ini.coordx ;
	double trasY = e1.fin.coordy - e2.ini.coordy ;
	//Translate all points in P2 to vertex1
	item2.translate(trasX, trasY);
}

void Attach(PIEZA item1, PIEZA &item2,int v1, int v2)
{
	vector<EDGES> *ed_i2 = item2.getEdges();
	vector<EDGES> *ed_i1 = item1.getEdges();
	double rot_angle; 
	EDGES e1 = (*ed_i1)[v1];
	EDGES e2 = (*ed_i2)[v2];
	//Rotate item2 so e2  matches edge e1
	rot_angle = calculate_rotation_match(e2, e1);
	item2.obtener_rotacion(rot_angle);
	ed_i2 = item2.getEdges();
	//Translate end of edge p2 from piece 2 to begining of edge p1 of piece 1
	//trans_piece_fin_ini(item2, e1, (*ed_i2)[v2]);
	//Translate beginning of edge p2 from piece 2 to end of edge p1 of piece 1
	trans_piece_ini_fin(item2, e1, (*ed_i2)[v2]);
}


bool Feasible_Slide_InSect(IRR_BIN bin, PIEZA item1,PIEZA &item2, int v1, int v2, double dis)
{
	bool feasible = true;
	//This function slides edge from piece p2 over edge 1 a distance dist as long as is a feasible move.
	vector<EDGES> *ed1 = item1.getEdges();
	EDGES e1 = (*ed1)[v1];//arista de la envoltura convexa.

	vector<PUNTO> ptos_i2 = *item2.obtener_puntos();//ptos de la pieza
	double xx = (e1.fin.coordx - e1.ini.coordx);
	double yy= (e1.fin.coordy-e1.ini.coordy);
	double alpha = atan2(yy, xx);//angle in radians (-180, 180).
	double xslide = dis*cos(alpha);// slide over the x axis
	double yslide = dis*sin(alpha);//slide over the y axis.
	//======================================================
	//Feasibility Check. Let's check if with this slide the piece fits in the bin.
	//First we have to slide copy of the points.
	vector<PUNTO> border = *bin.get_ptos();
	for(int i = 0; i<ptos_i2.size(); i++)
		{
		ptos_i2[i].coordx +=xslide;
		ptos_i2[i].coordy +=yslide;
		}
	//Ver si es factible (si entra en la sección)
	unsigned long last = border.size();
	for(int i = 0; i<ptos_i2.size(); i++)
		{
		for(int j= 0; j<border.size();j++)
			{
			if(j+1 !=last)
				{
				if(Position(border[j], border[j+1], ptos_i2[i])==1) return false;
				}
			else if (Position(border[last-1], border[0], ptos_i2[i]) == 1) return false;
			}
		
		}

	//Slide piece
	item2.translate(xslide, yslide);
	return feasible;
}


double match_dist(EDGES e1, EDGES e2)
{
	double md;
	double abs_dist = fabs(e1.mod - e2.mod);
	double max =e1.mod;
	if(e2.mod> e1.mod) max = e2.mod;
	md = 1-abs_dist/max;
	return md;
}



