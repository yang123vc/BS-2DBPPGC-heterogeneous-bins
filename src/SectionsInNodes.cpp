#include "classes_BPGC_HetBins.h"
vector<PUNTO>
convex_hull(vector<PUNTO> P);

//Esta función crea un elemento de la clase IRR_BIN a partir de otro de la clase NODE
//básicamente para tener un IRR BIN inicial y trabajar después sólo con IRR BINs.
IRR_BIN ConvertToIRRBIN(NODE &bin) {
	IRR_BIN ib;
	int ID = bin.getID();
	ib.setBin(ID);
	ib.set_ID(0);
	PUNTO aux;
	//Add coordinates of bin in clockwise order
	aux.coordx = 0;
	aux.coordy = 0;
	ib.add_points(aux);
	aux.coordx = 0;
	aux.coordy = bin.getW();
	ib.add_points(aux);
	aux.coordx = bin.getL();
	aux.coordy = bin.getW();
	ib.add_points(aux);
	aux.coordx = bin.getL();
	aux.coordy = 0;
	ib.add_points(aux);
	//=======================================
	ib.setArea();
	int np = bin.getNumPiezas();
	for (int i = 0; i < np; i++) {
		PIEZA p = *(bin.getPI())[i];
		ib.add_piece(*(bin.getPI())[i]);
		ib.set_waste(*(bin.getPI())[i]);
		ib.set_prop_used();
	}
	ib.get_angles();
	return ib;
}

IRR_BIN Redefine_NewSection(vector<PUNTO> &rbin, EDGES gc) {
	IRR_BIN sect;
	PUNTO ini, fin;
	ini = gc.ini;
	fin = gc.fin;
	vector<PUNTO> ptos_sec;
	ptos_sec.push_back(ini);
	ptos_sec.push_back(fin);

	//Create the points of the new section
	//====================================
	//Add every point from rbin that are to the right of the gc.
	int j = 0;
	//Add points that are to the right of the gc
	while (j < rbin.size()) {
		int pos = Position(ini, fin, rbin[j]);
		if (pos != -1)
			j++;
		else {
			ptos_sec.push_back(rbin[j]);
			j++;
		}
	}
	//====================================
	//Order ptos of ptos_sec in clockwise order
	ptos_sec = convex_hull(ptos_sec);
	std::reverse(ptos_sec.begin(), ptos_sec.end());
	//Eliminate last point
	ptos_sec.back();
	ptos_sec.pop_back();
	//Add them to the new section
	for (int i = 0; i < ptos_sec.size(); i++) {
		sect.add_points(ptos_sec[i]);
	}
	sect.setArea();
	sect.get_angles();
	return sect;
}

//This function returns a bool that is TRUE if the edges are all inside the section and FALSE if there is any edge outside the section.
bool FitInSect_IniPcFinSec(vector<EDGES> e_pza, vector<PUNTO> ptos_Sec,
		EDGES esec_match, EDGES epza_match) {
	esec_match.set_mod();
	epza_match.set_mod();
	if (esec_match.mod < epza_match.mod)
		return false; //Edge on the piece is longer than the edge of the section

	//Calculate translation: Matching Ini_pz with fin_sect
	double trasX = esec_match.fin.coordx - epza_match.ini.coordx;
	double trasY = esec_match.fin.coordy - epza_match.ini.coordy;
	//Translate all points in P2 to vertex1
	for (int i = 0; i < e_pza.size(); i++) {
		e_pza[i].ini.coordx += trasX;
		e_pza[i].ini.coordy += trasY;
	}
	int last_pto = ptos_Sec.size() - 1;
	for (int i = 0; i < e_pza.size(); i++) {
		for (int j = 0; j < ptos_Sec.size() - 1; j++) {
			int side = Position(ptos_Sec[j], ptos_Sec[j + 1], e_pza[i].ini);
			if (side == 1)
				return false; //the point is to the left of the section (outside)
		}
		//Last edge of section
		int side = Position(ptos_Sec[last_pto], ptos_Sec[0], e_pza[i].ini);
		if (side == 1)
			return false;	//the point is to the left of the section (outside)
	}

	return true;
}
bool FitInSect_FinPcIniSec(vector<EDGES> e_pza, vector<PUNTO> ptos_Sec,
		EDGES esec_match, EDGES epza_match) {
	esec_match.set_mod();
	epza_match.set_mod();
	if (esec_match.mod < epza_match.mod)
		return false;//Edge on the piece is longer than the edge of the section

	//Calculate translation: Matching Ini_pz with fin_sect
	double trasX = esec_match.ini.coordx - epza_match.fin.coordx;
	double trasY = esec_match.ini.coordy - epza_match.fin.coordy;
	//Translate all points in P2 to vertex1
	for (int i = 0; i < e_pza.size(); i++) {
		e_pza[i].ini.coordx += trasX;
		e_pza[i].ini.coordy += trasY;
	}
	int last_pto = ptos_Sec.size() - 1;
	for (int i = 0; i < e_pza.size(); i++) {
		for (int j = 0; j < ptos_Sec.size() - 1; j++) {
			int side = Position(ptos_Sec[j], ptos_Sec[j + 1], e_pza[i].ini);
			if (side == 1)
				return false;//the point is to the left of the section (outside)
		}
		//Last edge of section
		int side = Position(ptos_Sec[last_pto], ptos_Sec[0], e_pza[i].ini);
		if (side == 1)
			return false;	//the point is to the left of the section (outside)
	}

	return true;
}

int BreakTie(vector<EDGES> e_pza, vector<PUNTO> ptos_sect, EDGES esec,
		EDGES epza) {
	int best = -1;
	int IpFs = 0; //counts the number of matches between the piece and the section, when the ini of the piece is matched with the end of the section.
	int FpIs = 0; //counts the number of matches between the piece and the section, when the end of the piece is matched with the ini of the section.
	//Number of matches IpFs:
	//Calculate translation: Matching Ini_pz with fin_sect
	//=======================================================
	double trasX = esec.fin.coordx - epza.ini.coordx;
	double trasY = esec.fin.coordy - epza.ini.coordy;
	//Translate all points in piece to vertex in section
	for (int i = 0; i < e_pza.size(); i++) {
		e_pza[i].ini.coordx += trasX;
		e_pza[i].ini.coordy += trasY;
	}
	//Identify ptos_sec that matches the end of esec
	int k = 0;
	while (!equal_pto(ptos_sect[k], esec.fin))
		k++;
	int next = k + 1;
	if (k + 1 >= ptos_sect.size())
		next = 0;
	for (int i = 0; i < e_pza.size(); i++) //For all points in the piece, add those that are in line with the adjacent edge
			{
		int match = Position(ptos_sect[k], ptos_sect[next], e_pza[i].ini);
		if (match == 0)
			IpFs++;
	}
	//Edges back to original position
	for (int i = 0; i < e_pza.size(); i++) {
		e_pza[i].ini.coordx -= trasX;
		e_pza[i].ini.coordy -= trasY;
	}
	//Calculate translation: Matching Fin_pz with Ini_sect
	//=======================================================
	trasX = esec.ini.coordx - epza.fin.coordx;
	trasY = esec.ini.coordy - epza.fin.coordy;
	//Translate all points in piece to vertex in section
	for (int i = 0; i < e_pza.size(); i++) {
		e_pza[i].ini.coordx += trasX;
		e_pza[i].ini.coordy += trasY;
	}
	//Identify ptos_sec that matches the ini of esec
	k = 0;
	while (!equal_pto(ptos_sect[k], esec.ini))
		k++;
	next = k - 1;
	if (k - 1 < 0)
		next = ptos_sect.size() - 1;
	for (int i = 0; i < e_pza.size(); i++) //For all points in the piece, add those that are in line with the adjacent edge
			{
		int match = Position(ptos_sect[next], ptos_sect[k], e_pza[i].ini);
		if (match == 0)
			FpIs++;
	}

	//=======================================================
	if (IpFs > FpIs)
		best = 0; //IniPz-EndSec more matches than the other position
	else
		best = 1; //EndPz-IniSec more matches than the other position
	return best; //If best = -1 still a tie.
}
