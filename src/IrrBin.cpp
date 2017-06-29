#include"classes_BPGC_HetBins.h"

//===============================================
//Funciones asociadas a la clase IRR_BIN
//===============================================
void IRR_BIN::set_ID(int i) {
	ID = i;
}
int IRR_BIN::getID() {
	return ID;
}
;

void IRR_BIN::setBin(int b) {
	binID = b;
}

void IRR_BIN::setArea() {
	area_ib = calcular_area_exacta(ib_ptos);
	//reverse ptos, dado que la función anterior está diseñada para polígonos en sentido antihorario, por lo que en este caso hace un reverse de los ptos. 
	std::reverse(ib_ptos.begin(), ib_ptos.end());

}

double IRR_BIN::getArea() {
	return area_ib;
}

void IRR_BIN::set_waste(PIEZA &p) {
	double area_p = p.getArea();
	waste_ib = area_ib - area_p;
}
double IRR_BIN::get_waste() {
	return waste_ib;
}
double IRR_BIN::get_prop_used() {
	return prop_used;
}

void IRR_BIN::set_prop_used() {
	prop_used = 1 - waste_ib / area_ib;
}

void IRR_BIN::update_waste() {
	double tot_pz_area = 0;
	for (int i = 0; i < ib_pzas.size(); i++)
		tot_pz_area = tot_pz_area + ib_pzas[i]->getArea();
	waste_ib = area_ib - tot_pz_area;
}

int IRR_BIN::getBin() {
	return binID;
}

void IRR_BIN::add_points(PUNTO &pto) {
	ib_ptos.push_back(pto);
}

vector<PUNTO> *
IRR_BIN::get_ptos() {
	return &ib_ptos;
}
;

vector<PIEZA*> *
IRR_BIN::get_pzas() {
	return (&ib_pzas);
}

void IRR_BIN::add_GC(EDGES &e) {
	ib_GC.push_back(e);
}

vector<EDGES> *
IRR_BIN::get_ibGC() {
	return (&ib_GC);
}

void IRR_BIN::add_piece(PIEZA &pza) {
	//Incluimos pieza p en el bin (i.e. aumentamos en uno el número de piezas e incluimos la pieza en el vector piezas_incluidas.
	ib_pzas.push_back(&pza);
	//Actualizar largo y ancho utilizado y waste;
	set_waste(pza);
	set_prop_used();
}

void IRR_BIN::empty_irrbin() {
	ID = -1;
	binID = -1;
	prop_used = 0;
	ib_pzas.clear();
	ib_ptos.clear();
	ib_angles.clear();
	ib_GC.clear();
}

vector<double> IRR_BIN::get_angles() {
	vector<EDGES> ar;
	EDGES aux;
	int last = ib_ptos.size();
	//Create edges of IRRBIN
	for (int i = 0; i < last; i++) {
		aux.ini = ib_ptos[i];
		if (i + 1 != last)
			aux.fin = ib_ptos[i + 1];
		else
			aux.fin = ib_ptos[0];
		aux.set_mod();
		ar.push_back(aux);
	}
	ib_angles = calculateInnerAngles(ar);
	return ib_angles;
}

