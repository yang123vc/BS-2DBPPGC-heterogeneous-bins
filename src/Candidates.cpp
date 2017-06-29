#include "classes_BPGC_HetBins.h"

PIEZA
create_convexhull_in_section(IRR_BIN &sect);
PIEZA
create_rectencl_in_section(IRR_BIN &sect);
double
match_dist(EDGES e1, EDGES e2);
bool
edge_of_section(EDGES ed, IRR_BIN ib);
double
SelectBestMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA ch, int &b_e1,
		int &b_e2, double &b_d, int &b_p, int &b_m);
double
GuillotineCutWaste(PIEZA ch, IRR_BIN current_sect, EDGES &best_gc);
void
Attach(PIEZA item1, PIEZA &item2, int v1, int v2);
bool
Feasible_Slide_InSect(IRR_BIN bin, PIEZA item1, PIEZA &item2, int v1, int v2,
		double dis);
bool
Place_in_NewSection(IRR_BIN &current_sect, vector<PIEZA*> &candidates,
		EDGES &best_gc, int id);
vector<PUNTO>
Redefine_IrrBin_withPieces(vector<PUNTO> &rbin, EDGES gc);
bool
NextBestCG_candidates(vector<PIEZA*> &item, IRR_BIN &csect, EDGES &gc);
int
pos_disp(vector<PIEZA> &item, vector<int> &ID);

vector<PIEZA*> CreateList(vector<PIEZA> &item, IRR_BIN &sect, int LIST_SIZE,
		double thres, vector<int> ID_disp) {
	vector<PIEZA*> list;
	PIEZA convexhull;
	convexhull = create_convexhull_in_section(sect);
	//convexhull = create_rectencl_in_section(sect);//Rectangle enclosure for the rectangle instances.
	vector<EDGES> *e_ch = convexhull.getEdges(); //Edges of the convex hull.
	vector<EDGES> *e_item;
	double area_ch = convexhull.getArea();
	double area_sect = sect.getArea();
	int p = 0;
	int pzas_rem = ID_disp.size();
	bool is_ed_sect = false;
	while (list.size() < LIST_SIZE && !ID_disp.empty()) {
		p = pos_disp(item, ID_disp); //First available piece
		bool is_item_list = false;
		if (item[p].getArea() + area_ch < area_sect && !item[p].is_placed()) //piece is candidate in terms of area, not being the list and not previusly placed.
				{
			if (pzas_rem <= LIST_SIZE) //All remaining pieces are candidates. 
					{
				list.push_back(&item[p]);
				continue;
			}
			e_item = item[p].getEdges();
			//Compare each edge of the convex hull with each edge of the piece to see if there is a possible match and include the piece in the list.
			for (int i = 0; i < (*e_ch).size() && !is_item_list; i++) //For each edge of the convex hull
					{
				is_ed_sect = edge_of_section((*e_ch)[i], sect);
				if (!is_ed_sect) //The edge to match is not in the edge of the bin.
				{
					for (int j = 0; j < (*e_item).size() && !is_item_list; j++) //For each edge of the piece
							{
						double md = match_dist((*e_ch)[i], (*e_item)[j]); //Calculate md
						if (md >= thres) //if edges of convex hull and edges of piece are likely in lenght
								{
							list.push_back(&item[p]); //include piece in the list.
							is_item_list = true;
							break;
						}
					}
				}
			}
			if (!is_item_list) //Item did not make it to the list, because edges are not of matching distance
			{
				continue;
			}
		}
	}
	return list;
}

//Function that places candidates in a bin (section) based on three criteria
bool Place_Candidates(NODE &node, IRR_BIN &sect, vector<PIEZA*> &list,
		bool &CH_crit, bool &bestGC_crit, bool &NextGC_crit, EDGES &best_gc,
		int &sec_mod_id) {
	bool placed = false;
	PIEZA convex_hull;
	IRR_BIN old_sec = sect;
	double ch_util = -1; //best utilization of the convex hull when the next piece is attached.
	int best_i = -1; //Best edge of the ch
	int best_j = -1; //Best edge of the piece
	int best_p = -1; //Best piece from list
	int best_m = -1; // = 0 piece not mirrored
	double best_d = -1; // best slide distance.
	double gc_util = -1; //best utilization of the bin, when the guillotine cut is performed.

	convex_hull = create_convexhull_in_section(sect);
	//convex_hull = create_rectencl_in_section(sect); //Rectangle enclosure for the rectangle instances.
	ch_util = SelectBestMatch(sect, list, convex_hull, best_i, best_j, best_d,
			best_p, best_m);
	gc_util = GuillotineCutWaste(convex_hull, sect, best_gc);
	gc_util = 1 - gc_util;
	if (ch_util != -1 && ch_util > gc_util) {
		//Use CH-Criterion to place piece
		if (best_m == 1) //mirror piece
			list[best_p]->set_mirror(true);
		Attach(convex_hull, *list[best_p], best_i, best_j); //Attach best values of edge i from convex hull and edge j from piece
		Feasible_Slide_InSect(sect, convex_hull, *list[best_p], best_i, best_j,
				best_d); //Slide edges along best distance
		//Get the edges of the convex hull to include the guillotine cut.
		vector<EDGES> ed_ch = *convex_hull.getEdges();
		sect.add_GC(ed_ch[best_i]);  //Include the guillotine cut in the section
		sect.add_piece(*list[best_p]); //Include the new piece in the current sect
		placed = true;
		CH_crit = true;
	} else {
		//Use BestGC-Criterion: Place candidates in an empty section.
		placed = Place_in_NewSection(sect, list, best_gc, node.getNumSect());
		if (placed) {
			sec_mod_id = old_sec.getID();
			sect.add_GC(best_gc);	//Add the guillotine cut to the section
			bestGC_crit = true;
		}
	}
	if (!placed && node.getNumSect() > 1) {
		//Use GC-Criterion if there is more than one section
		PUNTO gc_empty;
		gc_empty.coordx = -1;
		gc_empty.coordy = -1;
		//========================================
		for (int i = 0; i < node.getNumSect(); i++) {
			sect = node.get_ib(i);
			convex_hull = create_convexhull_in_section(sect);//Creates the convex hull of pieces in the section
//			convex_hull = create_rectencl_in_section(sect); //Rectangle enclosure for the rectangle instances.
			GuillotineCutWaste(convex_hull, sect, best_gc);
			if (equal_pto(best_gc.ini, gc_empty)
					&& equal_pto(best_gc.fin, gc_empty))//No more gc possible in current section, move to next
				continue;
			placed = NextBestCG_candidates(list, sect, best_gc);
			if (placed) {
				sec_mod_id = i;
				sect.set_ID(node.getNumSect());
				sect.add_GC(best_gc);	//Add the guillotine cut to the section
				NextGC_crit = true;
				break;		//Out of the loop
			}
		}
	}
	if (!placed)
		sect = old_sec;
	return placed;
}

int pos_disp(vector<PIEZA> &item, vector<int> &ID) {
	int p = 0;
	bool piece_disp = false;
	vector<int>::iterator it;
	while (!piece_disp) {
		it = find(ID.begin(), ID.end(), item[p].getID());
		if (it != ID.end()) {
			ID.erase(it);
			piece_disp = true;
		} else
			p++; //move to the first available piece
	}
	return p;

}
