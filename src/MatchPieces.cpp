#include "classes_BPGC_HetBins.h"

double
match_dist(EDGES e1, EDGES e2);
double
convexhull_Utilization(PIEZA P1, PIEZA P2);
double
rectencl_Utilization(PIEZA P1, PIEZA P2);
void
Attach(PIEZA item1, PIEZA &item2, int v1, int v2);
bool
edge_of_section(EDGES ed, IRR_BIN ib);
bool
Feasible_Slide_InSect(IRR_BIN bin, PIEZA item1, PIEZA &item2, int v1, int v2,
		double dis);

double SelectBestMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA convex_hull,
		int &best_e1, int &best_e2, double &best_d, int &best_p, int &best_m) {
	//This function finds the best match between a set of pieces and all the others in the bin.
	//It uses ideas from EJORS paper (Han et al.)
	PIEZA citem;
	bool is_ed_bin = false;
	double MaxUtil = -1;
	double dist = 0;
	bool is_sym = true;
	bool slide_feas = false;
	vector<EDGES> *e_ch = convex_hull.getEdges();  //Edges of the convex hull.
	for (int p = 0; p < list.size(); p++) //For each piece in the list, find the best match
			{
		citem = *list[p];
		vector<EDGES> *e_citem = list[p]->getEdges();	//edges of the piece
		for (int i = 0; i < (*e_ch).size(); i++)//for each edge on the convex hull
				{
			is_ed_bin = edge_of_section((*e_ch)[i], bin);
			if (!is_ed_bin)	//The edge to match is not in the edge of the bin.
			{
				//for(int m = 0; m<= 0; m++) //Rectangle instances, no mirror considered.
				for (int m = 0; m <= 1; m++) {
					for (int j = 0; j < (*e_citem).size(); j++)	//for each edge of the piece
							{
						citem = *list[p];//initialise citem so we do the rotations and traslations with the piece in its original position.
						if (m == 1) //Perform a reflection on piece item to check if the mirrored piece fits better.
								{
							is_sym = citem.set_mirror(true);
						}
						if (m == 1 && is_sym)
							continue; //Mirrorred piece is the same as not mirrrored. Already checked its position.
						double md = match_dist((*e_ch)[i], (*e_citem)[j]); //Calculate md
						if ((*e_ch)[i].mod - (*e_citem)[j].mod > 0)
							dist = (*e_ch)[i].mod - (*e_citem)[j].mod; //Calculate dist
						else
							dist = 0;
						double slide = 0;
						slide_feas = false;
						if (md > THRES + TOL) {
							Attach(convex_hull, citem, i, j); //Attach edge j from piece to edge i of convex hull
							for (double d = 0; d <= dist; d += slide_ep)
							{
								if (slide_feas)
									slide = slide_ep;
								else if (d != 0)
									slide += slide_ep;
								slide_feas = Feasible_Slide_InSect(bin,
										convex_hull, citem, i, j, slide);
								if (slide_feas) //Slide edge j from piece to edge i of convex hull only if feasible
								{
									double Util = convexhull_Utilization(
											convex_hull, citem);
									//double Util = rectencl_Utilization(convex_hull, citem);
									if (Util > MaxUtil + TOL) //If it is a good utilization of the new convex hull area, store best values
									{
										MaxUtil = Util;
										best_d = d;
										best_e1 = i;
										best_e2 = j;
										best_m = m;
										best_p = p;
									}
								}
							}
						}
					}
				}

			}
		}

	}
	return MaxUtil;
}
