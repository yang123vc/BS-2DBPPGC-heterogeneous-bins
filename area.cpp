#include "classes_BPGC_HetBins.h"

double calcular_area_exacta (vector<PUNTO> &poligono)
{
	if (poligono.empty()==true) return -1;
	if (poligono.size()<3) return -1;
	double suma=0;
	//Calculo del área utilizando la fórmula A = 1/2(x(i)y(i+1) - x(i+1)y(i))
	for(int i=0;i<poligono.size()-1;i++)//Sentido antihorario
		{
		suma+=poligono[i].coordx*poligono[i+1].coordy-
		poligono[i].coordy*poligono[i+1].coordx;
		}//for
	//ahora el ultimo con el primero
	int i=poligono.size()-1;
	suma+=poligono[i].coordx*poligono[0].coordy-
	poligono[i].coordy*poligono[0].coordx;
	//Si la suma es negativa, entonces recorremos los vértices en sentido horario
	if (suma<=TOL)
		{
		std::reverse(poligono.begin(),poligono.end());
		}
	return fabs(suma)/2;
}

bool orden_area(PIEZA i , PIEZA j)
{
	return(i.getArea()>j.getArea());
}

bool orden_base(PIEZA i, PIEZA j)
{
	return(i.getbase()>j.getbase());
}

bool orden_perimeter(PIEZA i, PIEZA j)
{
	//Perimeter of piece i
	vector<EDGES> *ed_i = i.getEdges();
	double per_i = 0;
	for(int k  = 0; k<(*ed_i).size(); k++)
		{
		per_i = per_i + (*ed_i)[k].mod;
		}
	//====================
	//Perimeter of piece j
	vector<EDGES> *ed_j = j.getEdges();
	double per_j = 0;
	for(int k  = 0; k<(*ed_j).size(); k++)
		{
		per_j = per_j + (*ed_j)[k].mod;
		}
	//====================
	return(per_i>per_j);
	
}

