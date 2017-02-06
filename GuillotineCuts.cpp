
#include "classes_BPGC_HetBins.h"
double distancia(PUNTO p1, PUNTO p2);
EDGES IntersectBin(EDGES gc, vector<PUNTO> rbin);
PUNTO IntersectTwoLines(PUNTO ini_l1, PUNTO fin_l1, PUNTO ini_l2, PUNTO fin_l2);
int Position(PUNTO A, PUNTO B, PUNTO C);
double CalculatePropWasteCut(vector<PUNTO> ib_ptos, EDGES gc, double area);
vector<PUNTO> Redefine_IrrBin_withPieces(vector<PUNTO> &rbin, EDGES gc);
bool edge_of_section(EDGES ed, IRR_BIN ib);


//Función que devuelve el corte guillotina cuando se le pasa un punto y los lados de un "residual bin"
EDGES IntersectBin(EDGES gc, vector<PUNTO> rbin)
{
	PUNTO A = gc.ini;
	PUNTO B = gc.fin;
	
	//We are checking intersection of line AB vs all lines in rbin.
	for (int j = 0; j < rbin.size() - 1; j++)
		{
		PUNTO p;//Intersection point
		PUNTO C = rbin[j];
		PUNTO D = rbin[j + 1];
		double xmin = C.coordx;
		double xmax = D.coordx;
		if (C.coordx > D.coordx)
			{
			xmin = D.coordx;
			xmax = C.coordx;
			}
		double ymin = C.coordy;
		double ymax = D.coordy;
		if (C.coordy > D.coordy)
			{
			ymin = D.coordy;
			ymax = C.coordy;
			}
		p = IntersectTwoLines(A, B, C, D);
		//Check whether the intersection point is whithin the limits of rbin.
		if (p.coordx > xmin - TOL && p.coordx < xmax + TOL && p.coordy > ymin - TOL && p.coordy < ymax + TOL)
			{
			//The intersection point is in rbin.
			//Check distance to ini or fin of current to update limits.
			double dist_ini = distancia(p, gc.ini);
			double dist_fin = distancia(p, gc.fin);
			if (dist_ini < dist_fin)
				gc.ini = p;//The intersection point is the ini of the current GC
			else
				gc.fin = p;//The intersection point is the fin of the current GC
			}
		//The intersection point is not in the bin, so we do not change anything
		}
	//Check last edge of rbin
	//===========================
	unsigned long j = rbin.size()-1;
	PUNTO p;//Intersection point
	PUNTO C = rbin[j];
	PUNTO D = rbin[0];
	double xmin = C.coordx;
	double xmax = D.coordx;
	if (C.coordx > D.coordx)
		{
		xmin = D.coordx;
		xmax = C.coordx;
		}
	double ymin = C.coordy;
	double ymax = D.coordy;
	if (C.coordy > D.coordy)
		{
		ymin = D.coordy;
		ymax = C.coordy;
		}
	p = IntersectTwoLines(A, B, C, D);
	//Check whether the intersection point is whithin the limits of rbin.
	if (p.coordx > xmin - TOL && p.coordx < xmax + TOL && p.coordy > ymin - TOL && p.coordy < ymax + TOL)
		{
		//The intersection point is in rbin.
		//Check distance to ini or fin of current to update limits.
		double dist_ini = distancia(p, gc.ini);
		double dist_fin = distancia(p, gc.fin);
		if (dist_ini < dist_fin)
			gc.ini = p;//The intersection point is the ini of the current GC
		else
			gc.fin = p;//The intersection point is the fin of the current GC
		}
	//The intersection point is not in the bin, so we do not change anything
	gc.set_mod();
	return gc;
}

//Función que dado un "residual bin" y un corte guillotina, redefine el "residual bin" añadiendo ese corte guillotina y eliminando los puntos correspondientes. Devuelve la parte donde quedaría la pieza

vector<PUNTO> Redefine_IrrBin_withPieces(vector<PUNTO> &rbin, EDGES gc)
{
	vector<PUNTO>::iterator it;
	it = rbin.begin();
	//Change orientation of guillotine cut.
	PUNTO aux;
	aux = gc.ini;
	gc.ini = gc.fin;
	gc.fin = aux;
	//====================================
	//Add new limits of gc to rbin, to make the cut.
	//==============================================
	//For each edge check if limits of gc are aligned with the edge
	int j = 0;
	int j_next = 1;
	while (j < rbin.size())
		{
		j_next = j+1;
		if(j_next == rbin.size()) j_next = 0;
		if(Position(rbin[j], rbin[j_next], gc.ini) != 0 )
			j++;
		else
			break;
		}
	
	if (j < rbin.size()) //Insert gc after rbin[j]
		{
		int s = rbin.size();
		//If gc.ini not equal to vertex of residual bin, include point
		if(!equal_pto(gc.ini, rbin[j]) && !equal_pto(gc.ini, rbin[j_next])) //Antes j+1
			{
			rbin.insert(it+j_next,gc.ini);//Antes j+1
			it = rbin.begin();
			}
		if(!equal_pto(gc.fin, rbin[j_next]) && !equal_pto(gc.fin, rbin[j]))//Antes j+1
			//If gc.fin not equal to vertex of residual bin, include point.
			{
			it = it + j_next + 1; //Antes j+1
			if(j_next + 1 >= s)//Antes j+1
				//the guillotine cut is added at the end of rbin
				rbin.push_back(gc.fin);
			else rbin.insert(it, gc.fin);
			it = rbin.begin();
			}
		}
	else if (j == rbin.size())//Last edge
		{
		rbin.push_back(gc.ini);
		rbin.push_back(gc.fin);
		}
	j = 0;
	//Delete points that are to the left of the gc
	while (j < rbin.size())
		{
		if (Position(gc.ini, gc.fin, rbin[j]) != 1)
			j++;
		else
			rbin.erase(rbin.begin() + j);
		
		}
	return rbin;
}

//Función que devuelve el punto de intersección entre dos líneas, si es que existe.
//(-1,-1) si las líneas son paralelas.

PUNTO IntersectTwoLines(PUNTO ini_l1, PUNTO fin_l1, PUNTO ini_l2, PUNTO fin_l2)
{
	PUNTO r;
	//Write first line in slope intercept mode
	double x0 = ini_l1.coordx;
	double y0 = ini_l1.coordy;
	double x1 = fin_l1.coordx;
	double y1 = fin_l1.coordy;
	double slope1;
	if (!equal_double(x0, x1, TOL))
		slope1 = (y1 - y0) / (x1 - x0);//Slope of first line
	else
		slope1 = GRANDE; //Line 1 is vertical
	//===================================================
	//Write second line in slope intercept mode
	double x2 = ini_l2.coordx;
	double y2 = ini_l2.coordy;
	double x3 = fin_l2.coordx;
	double y3 = fin_l2.coordy;
	double slope2;
	if (!equal_double(x2, x3, TOL))
		slope2 = (y3 - y2) / (x3 - x2);//Slope of second line
	else
		slope2 = GRANDE;
	//=====================================================
	if (equal_double(slope1, slope2, TOL))//Lines are parallel
		{
		r.coordx = -1;
		r.coordy = -1;
		return r;
		}
	r.coordx = (y2 - slope2*x2 + slope1*x0 - y0) / (slope1 - slope2);
	r.coordy = slope1*(r.coordx - x0) + y0;
	if (slope1 == GRANDE)//Line 1 is vertical
		{
		r.coordx = x0;
		r.coordy = slope2*(x0 - x2) + y2;
		return r;
		}
	if (slope2 == GRANDE)//Line 2 is vertical
		{
		r.coordx = ini_l2.coordx;
		return r;
		}
	return r;
}


//Function that calculates the proportional waste generated by all possible guillotine cuts, of a given (set of) piece(s)
// and keeps the cut with minimum proportional waste.
double  GuillotineCutWaste(PIEZA ch,IRR_BIN current_sect,EDGES &best_gc)
{
	double min_waste = GRANDE;
	double prop_waste;
	vector<EDGES> ed_ch = *ch.getEdges();//Get the edges of the convex hull of the pieces in the section
	vector<EDGES>::iterator it_ed;
	vector<PUNTO> ib_ptos;
	double area_used = ch.getArea();
	bool is_edge_sect = false;
	ib_ptos = *current_sect.get_ptos();
	EDGES gc;
	int countfalse = 0;
	//Initialize best_gc so we know if there are no more gc associated with that section
	//======================
	best_gc.ini.coordx = -1;
	best_gc.ini.coordy = -1;
	best_gc.fin.coordx = -1;
	best_gc.fin.coordy = -1;
	//======================
	if(equal_double(area_used, current_sect.getArea(), 5)) return 0; //The convex hull fulfills the entire section.
	for(it_ed = ed_ch.begin(); it_ed !=ed_ch.end();it_ed++)//For each edge in the convex hull
		{
		is_edge_sect = edge_of_section(*it_ed, current_sect);
		if(!is_edge_sect)//That is not on the edge of a section
			{
			countfalse++;
			gc = IntersectBin(*it_ed, ib_ptos);//get the guillotine cut defined by the edge of the ch.
			prop_waste = CalculatePropWasteCut(ib_ptos, gc, area_used);
			if(prop_waste < min_waste)
				{
				best_gc = gc;
				min_waste = prop_waste;
				}
	
			}
		}
	return min_waste;
}

double CalculatePropWasteCut(vector<PUNTO> ib_ptos, EDGES gc, double area_used)
{
	double waste;
	double prop_waste;
	vector<PUNTO> ptos_new;
	ptos_new = Redefine_IrrBin_withPieces(ib_ptos, gc);
	double totalarea;
	reverse(ptos_new.begin(), ptos_new.end());//To have the points in anticlockwise order
	totalarea = calcular_area_exacta(ptos_new);
	waste = totalarea-area_used;
	prop_waste = waste/totalarea;
	return prop_waste;
}

