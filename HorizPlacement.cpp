//
//  HorizPlacement.cpp
//  BS_Ideas
//
//  Created by Marta Cabo Nodar on 24/08/15.
//  Copyright (c) 2015 Marta Cabo. All rights reserved.
//

#include "classes_BPGC_HetBins.h"

double calculate_rotation_match (EDGES &e1,EDGES &e2);
IRR_BIN  ConvertToIRRBIN(NODE &bin);
PIEZA create_convexhull_in_section(IRR_BIN &sect);
PIEZA create_rectencl_in_section(IRR_BIN &sect);
double HorAligMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA convex_hull, int &best_e1, int &best_e2, double &best_d, int &best_p, int &best_m);
double match_dist(EDGES e1, EDGES e2);
double convexhull_Height(PIEZA P1, PIEZA P2);
void Attach(PIEZA item1, PIEZA &item2,int v1, int v2);
bool edge_of_section(EDGES ed, IRR_BIN ib);
bool Feasible_Slide_InSect(IRR_BIN bin, PIEZA item1,PIEZA &item2, int v1, int v2, double dis);
double VertAligMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA convex_hull, int &best_e1, int &best_e2, double &best_d, int &best_p, int &best_m);
double convexhull_Length(PIEZA P1, PIEZA P2);
double VertGuillCut(IRR_BIN &ib_node, vector<PIEZA*> last_pzas, PIEZA convex_hull, double nodeW, double nodeL, double &best_rot, int &best_p, int &best_i, EDGES &vcg);
void trans_piece_fin_ini(PIEZA &item2, EDGES e1, EDGES e2);
double rectencl_Height(PIEZA P1, PIEZA P2);


double ArrangeHor(vector<PIEZA> &pzas, NODE &node)
{
	double hor_util = (-1) * GRANDE;
	NODE c_node = node; //Copy of node, in case nothing gets modify we can retrieve it.
	c_node.empty_bin();
	c_node.setID(node.getID());
	c_node.set_pred(*node.get_Pred());
	double nodeW = node.getW();
	double nodeL = node.getL();
	//Pieces in original position
	for(int i = 0; i<pzas.size(); i++)
		{
		PUNTO origen;
		origen.coordy = 0.0;
		origen.coordx = 0.0;
		pzas[i].set_mirror(false);
		pzas[i].obtener_rotacion(0);
		pzas[i].setCoord(origen);
		}
	vector<PIEZA*> last_pzas;
	for(int i = 0; i<pzas.size(); i++)
		last_pzas.push_back(&pzas[i]);
	//==========================
	EDGES base_bin;
	base_bin.ini.coordx = node.getL();base_bin.ini.coordy = 0.0;
	base_bin.fin.coordx = 0.0; base_bin.fin.coordy = 0.0;
	base_bin.set_mod();
	double rot_angle = -1;
    double height_bm = -1; //Height of the convex hull of all pieces placed.
    double height_gc = -1;//Height of the highest piece when placed one next to the other
    double height = -1;
	// Place first piece with longest edge as base.
	
	//Falta comprobar si es mejor con la pieza reflejada.
	
	// First no mirrors.
	vector<EDGES> edges = *last_pzas[0]->getEdges();
	int longest_edge = argmax_mod(edges);
	rot_angle = calculate_rotation_match(edges[longest_edge], base_bin);
	last_pzas[0]->obtener_rotacion(rot_angle);
	if(!last_pzas[0]->Fits_Bin(c_node.getL(), c_node.getW())) //Piece does not fit horizontally.
		return hor_util;
	IRR_BIN ib_node = ConvertToIRRBIN(c_node);
	ib_node.set_ID(c_node.getID());
	ib_node.add_piece(*last_pzas[0]);
    height = last_pzas[0]->getw();
	//Delete pza 0 from last_pzas
	last_pzas.erase(last_pzas.begin());
	while (!last_pzas.empty())//Not all the pieces have been placed.
		{
		int best_ec =-1;//Best edge of the ch
		int best_ep = -1;//Best edge of the piece
		int best_p_bm = -1;//Best piece from list
		int best_m = -1; // = 0 piece not mirrored
		double best_d = -1;// best slide distance.
		double best_rot = -1;
		int best_p_vc = -1;
		int best_i_vc = -1;
		EDGES vcg;
		PIEZA convex_hull = create_convexhull_in_section(ib_node);
		//PIEZA convex_hull = create_rectencl_in_section(ib_node); //Rectangle enclosure for the rectangle instances.

		height_bm = HorAligMatch(ib_node, last_pzas, convex_hull, best_ec, best_ep, best_d, best_p_bm, best_m);
		if(height_gc < GRANDE) height_gc = VertGuillCut(ib_node, last_pzas, convex_hull, nodeW, nodeL, best_rot, best_p_vc, best_i_vc, vcg);//Devuelve la altura total si la pieza se coloca a la dcha del actual convex hull.
		if (height_bm == GRANDE && height_gc == GRANDE) return hor_util; //Piece did not fit

		if(height_bm <= height_gc)
			{
			//Place using best match
			height = height_bm;
			if(best_m == 1)
				{
				last_pzas[best_p_bm]->set_mirror(true);
				}
			Attach(convex_hull, *last_pzas[best_p_bm], best_ec, best_ep);
			Feasible_Slide_InSect(ib_node, convex_hull, *last_pzas[best_p_bm], best_ec,best_ep , best_d);
			//Get the edges of the convex hull to include the guillotine cut.
			vector<EDGES> ed_ch = *convex_hull.getEdges();
			ib_node.add_GC(ed_ch[best_ec]);//Include the guillotine cut in the section
			ib_node.add_piece(*last_pzas[best_p_bm]);//Include the new piece in the current sect
			last_pzas.erase(last_pzas.begin()+best_p_bm);
			}
		else //Place using the guillotine cut.
			{
			height = height_gc;
			last_pzas[best_p_vc]->obtener_rotacion(best_rot);
			EDGES e_pza = (*last_pzas[best_p_vc]->getEdges())[best_i_vc];
			trans_piece_fin_ini(*last_pzas[best_p_vc], vcg, e_pza);
			ib_node.add_piece(*last_pzas[best_p_vc]);//Include the new piece in the current sect
			last_pzas.erase(last_pzas.begin()+best_p_vc);
			}
		}
	hor_util = height/nodeW;
	//Copy pieces from ib_node to node;
	for(int p = 0; p<ib_node.get_pzas()->size(); p++)
		{
		vector<PIEZA*> pz_sect = *ib_node.get_pzas();
		c_node.include_piece(*pz_sect[p]);
		vector<EDGES>gc = *ib_node.get_ibGC();
		for(int g =0; g<gc.size(); g++) c_node.add_GC(gc[g]);
		}
	node = c_node;
	return hor_util;
}


//Function that similar to Best Match, places pieces together, this time, the matching criteria is that the total height of the resulting convex hull is as low as possible.
double HorAligMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA convex_hull, int &best_e1, int &best_e2, double &best_d, int &best_p, int &best_m)
{
	//This function finds the best match between a set of pieces and all the others in the bin.
	//It uses ideas from EJORS paper (Han et al.)
	PIEZA  citem;
	bool is_ed_bin = false;
	double MaxHeight = GRANDE;
	double dist=0;
	//bool matched = false;
	bool is_sym = true;
	bool slide_feas = false;
	vector<EDGES> *e_ch = convex_hull.getEdges();//Edges of the convex hull.
	for(int p = 0; p<list.size(); p++)//For each piece in the list, find the best match
		{
		citem = *list[p];
		vector<EDGES> *e_citem = list[p]->getEdges();//edges of the piece
		for(int i = 0; i< (*e_ch).size(); i++)//for each edge on the convex hull
			{
			is_ed_bin = edge_of_section((*e_ch)[i], bin);
			if(!is_ed_bin)//The edge to match is not in the edge of the bin.
				{
				for(int m = 0; m<= 1; m++)
					{
					for(int j = 0; j< (*e_citem).size();j++)//for each edge of the piece
						{
						citem = *list[p];//initialise citem so we do the rotations and traslations with the piece in its original position.
						if(m == 1) //Perform a reflection on piece item to check if the mirrored piece fits better.
							{
							is_sym = citem.set_mirror(true);
							}
						if(m == 1 && is_sym) continue; //Mirrorred piece is the same as not mirrrored. Already checked its position.
						double md = match_dist((*e_ch)[i], (*e_citem)[j]);//Calculate md
						if((*e_ch)[i].mod-(*e_citem)[j].mod> 0) dist = (*e_ch)[i].mod-(*e_citem)[j].mod;//Calculate dist
						else dist = 0;
						double slide = 0;
						slide_feas = false;
						if(md>0-TOL)
							{
							Attach(convex_hull, citem, i, j);//Attach edge j from piece to edge i of convex hull
							for(double d = 0; d<=dist; d+=slide_ep)
								{
								if (slide_feas) slide = slide_ep;
								else if(d!=0) slide += slide_ep;
								slide_feas = Feasible_Slide_InSect(bin, convex_hull, citem, i, j, slide);
								if(slide_feas)//Slide edge j from piece to edge i of convex hull only if feasible
									{
									double Height = convexhull_Height(convex_hull, citem);
									//double Height = rectencl_Height(convex_hull, citem);
								
									if(Height < MaxHeight-TOL)
										{
										//matched = true;
										MaxHeight = Height;
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
	return MaxHeight;
}



double VertGuillCut(IRR_BIN &ib_node, vector<PIEZA*> list, PIEZA convex_hull, double node_w, double node_l, double &best_rot, int &best_p, int &best_i, EDGES &v_gc)
{
	double MaxHeight = GRANDE;
	PIEZA item;
	double xmax = -1;
	double ymin = GRANDE;
	//Calculamos la coordenada más a la derecha de la envoltura convexa. En caso de empate, nos quedamos con la más baja (?).
	vector<PUNTO> ch_ptos = *convex_hull.obtener_puntos();
	for(int i = 0; i<ch_ptos.size(); i++)
		{
		if(ch_ptos[i].coordx > xmax)
			xmax = ch_ptos[i].coordx;
		if(ch_ptos[i].coordy < ymin)
			ymin = ch_ptos[i].coordy;
		}
	v_gc.ini.coordx = xmax; v_gc.ini.coordy = ymin;
	v_gc.fin.coordx = xmax; v_gc.fin.coordy = node_w;
	v_gc.set_mod();
	for(int p = 0; p<list.size(); p++)
		{
		for(int i = 0; i< list[p]->getEdges()->size(); i++)//for each edge on the piece
			{
			bool fit = true;
			item = *list[p];
			vector<EDGES> e_item = *item.getEdges();//edges of the piece
			double alpha = calculate_rotation_match(e_item[i], v_gc);
			//Rotamos y trasladamos citem para coincida con el corte vertical.
			item.obtener_rotacion(alpha);
			e_item = *item.getEdges();
			trans_piece_fin_ini(item, v_gc,e_item[i]);
			//Check if the piece is inside the bin.
			for(int j = 0; j<(item.obtener_puntos())->size(); j++)
				{
				vector<PUNTO> i_ptos = *item.obtener_puntos();
				if(i_ptos[j].coordx<0-TOL ||i_ptos[j].coordx > node_l +TOL ||i_ptos[j].coordy <0-TOL || i_ptos[j].coordy > node_w+TOL)
					{
					item = *list[p]; //Piece does not fit in bin, back to original position.
					fit = false;
					break;
					}
				}
			if(fit)
				{
				//Keep rotation that minimizes height.
				double height;
				if(item.getw()<convex_hull.getw()) height =convex_hull.getw();
				else height = item.getw();
				if(height<MaxHeight)
					{
					MaxHeight = height;
					best_rot = alpha;
					best_p = p;
					best_i = i;
					}
				}
			
			}
		}
	
	return MaxHeight;
}

bool MoveDownInSect(PIEZA &ch,IRR_BIN &sec,double slide)
{
	bool is_feas = true;
	//Move all points slide units down
	vector<EDGES> e_pza = *ch.getEdges();
	for(int i = 0; i<e_pza.size(); i++)
		{
		e_pza[i].ini.coordy = e_pza[i].ini.coordy - slide;
		if(e_pza[i].ini.coordy < 0-TOL)//Movement out of the bin.
			return false;
		}
	//Check feasibility
	vector<PUNTO> ptos_Sec = *sec.get_ptos();
	int last_pto = ptos_Sec.size()-1;
	for(int i= 0; i<e_pza.size();i++)
		{
		for(int j = 0; j<ptos_Sec.size()-1;j++)
			{
			int side = Position(ptos_Sec[j], ptos_Sec[j+1], e_pza[i].ini);
			if(side == 1)
				return false;
				//the point is to the left of the section (outside)
			}
		//Last edge of section
		int side = Position(ptos_Sec[last_pto], ptos_Sec[0], e_pza[i].ini);
		if(side == 1)
			return false;
			//the point is to the left of the section (outside)
		}
	if(is_feas)
		{
		ch.translate(0, -slide);//Translate the convex hull
		for(int i = 0; i<sec.get_pzas()->size(); i++)
			{
			PIEZA *pz =(*sec.get_pzas())[i];
			pz->translate(0, -slide);//Translate all the pieces in the section.
			}
		}
	return is_feas;
}
