//
//  LastBin.cpp
//  BS_Ideas
//
//  Created by Marta Cabo Nodar on 17/07/15.
//  Copyright (c) 2015 Marta Cabo. All rights reserved.
//

#include "classes_BPGC_HetBins.h"

double
ArrangeHor(vector<PIEZA> &pzas, NODE &node);
double
ArrangeVert(vector<PIEZA> &pzas, NODE &node);
bool
orden_area(PIEZA i, PIEZA j);
PIEZA
create_convexhull_in_section(IRR_BIN &sect);
PIEZA
create_rectencl_in_section(IRR_BIN &s);
bool
MoveDownInSect(PIEZA &ch, IRR_BIN &sec, double slide);
bool
MoveLeftInSect(PIEZA &ch, IRR_BIN &sec, double slide);
void LastBinRefinement(list<NODE> &tree) {
	list<NODE>::iterator it_node;
	it_node = tree.end();
	it_node--;
	double hor_util = (-1) * GRANDE;
	double ver_util = (-1) * GRANDE;

	while (it_node != tree.begin()) {
		double maxlength = -1;
		double maxheight = -1;
		if ((it_node->get_IDdisp()).empty()) //If node is the last bin on a solution
		{
			int id = it_node->getID();

			NODE lastbin = *it_node; //Copy of last node
			it_node->CopyPiecesInSect();
			//get pieces from last node
			vector<PIEZA> pzas;
			for (int i = 0; i < it_node->getPI().size(); i++) {
				PIEZA p = *(it_node->getPI())[i];
				pzas.push_back(p);
			}
			//========================
			sort(pzas.begin(), pzas.end(), orden_area);
			hor_util = ArrangeHor(pzas, *it_node);
			if (hor_util == (-1) * GRANDE) //If pieces did not fit horizontally
			{
				it_node->CopyPiecesInSect();
				maxheight = -1;
				for (int id = 0; id < it_node->getNumSect(); id++) {
					bool is_feasible = true;
					IRR_BIN sec = it_node->get_ib(id);
					PIEZA ch = create_convexhull_in_section(sec);
//					PIEZA ch = create_rectencl_in_section(sec); //Rectangle enclosure for the rectangle instances.

					double slide = 0;
					while (is_feasible) {
						is_feasible = MoveDownInSect(ch, sec, slide);
						slide = 5;
					}
					for (int i = 0; i < (*ch.obtener_puntos()).size(); i++) {
						double p = (*ch.obtener_puntos())[i].coordy;
						if (maxheight < p)
							maxheight = (*ch.obtener_puntos())[i].coordy;
					}
					if (equal_double(maxheight, it_node->getW(), TOL))
						break;
				}
				hor_util = maxheight / it_node->getW();
				if (hor_util < 0)
					hor_util = 1.0;
			} else {
				//Calculate the maximum height of all pieces placed.
				for (int i = 0; i < pzas.size(); i++) //For each piece
						{
					vector<PUNTO> ptos = *pzas[i].obtener_puntos();
					for (int j = 0; j < ptos.size(); j++) //For each point on each piece.
							{
						if (ptos[j].coordy > maxheight)
							maxheight = ptos[j].coordy;
					}
				}
			}
			NODE lastbin_hor = *it_node;
			*it_node = lastbin;
			ver_util = ArrangeVert(pzas, *it_node);
			if (ver_util == (-1) * GRANDE) //If pieces did not fit vertically
			{
				it_node->CopyPiecesInSect();
				maxlength = -1;
				for (int id = 0; id < it_node->getNumSect(); id++) {
					bool is_feasible = true;
					IRR_BIN sec = it_node->get_ib(id);
					PIEZA ch = create_convexhull_in_section(sec);
//					PIEZA ch = create_rectencl_in_section(sec); //Rectangle enclosure for the rectangle instances.

					double slide = 0;
					while (is_feasible) {
						is_feasible = MoveLeftInSect(ch, sec, slide);
						slide = 5;
					}
					for (int i = 0; i < (*ch.obtener_puntos()).size(); i++) {
						double p = (*ch.obtener_puntos())[i].coordx;
						if (maxlength < p)
							maxlength = (*ch.obtener_puntos())[i].coordx;
					}
					if (equal_double(maxlength, it_node->getL(), TOL))
						break;
				}
				ver_util = maxlength / it_node->getL();
				if (ver_util < 0)
					ver_util = 1.0;
			} else {
				//Calculate the maximum length of all pieces placed.
				for (int i = 0; i < pzas.size(); i++) //For each piece
						{
					vector<PUNTO> ptos = *pzas[i].obtener_puntos();
					for (int j = 0; j < ptos.size(); j++) //For each point on each piece.
							{
						if (ptos[j].coordx > maxlength)
							maxlength = ptos[j].coordx;
					}
				}
			}
			NODE lastbin_vert = *it_node;
			if (ver_util > 0 && ver_util < hor_util + TOL) {
				*it_node = lastbin_vert;
				it_node->FixUtil(ver_util);
				it_node->FixDim(it_node->getW(), maxlength);

			}
			if (hor_util > 0 && hor_util < ver_util + TOL) {
				*it_node = lastbin_hor;
				it_node->FixUtil(hor_util);
				it_node->FixDim(maxheight, it_node->getL());
			}
		}
		it_node--;
	}

}

