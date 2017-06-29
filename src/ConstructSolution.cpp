#include "classes_BPGC_HetBins.h"
void
OpenBin(NODE &bin, PIEZA &item);
IRR_BIN
ConvertToIRRBIN(NODE &bin);
PIEZA
create_convexhull_in_section(IRR_BIN &sect);
PIEZA
create_rectencl_in_section(IRR_BIN &sect);
vector<PIEZA*>
CreateList(vector<PIEZA> &item, IRR_BIN &sect);
double
SelectBestMatch(IRR_BIN &bin, vector<PIEZA*> list, PIEZA convex_hull,
		int &best_e1, int &best_e2, double &best_d, int &best_p, int &best_m);
double
GuillotineCutWaste(PIEZA ch, IRR_BIN current_sect, EDGES &best_gc);
bool
FillBin(vector<PIEZA> &item, IRR_BIN &c_sect, EDGES &best_gc, int i);
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
void
RepackLBin(const char *argv, vector<NODE> &bin_sol);

//Function FillBin: Place piece in the bin not from the candidate list.
IRR_BIN
Redefine_NewSection(vector<PUNTO> &rbin, EDGES gc);
bool
PieceInSect(PIEZA &candidate, IRR_BIN &current_sect);
bool FillBin(vector<PIEZA> &item, IRR_BIN &csect, EDGES &gc, int id) {
	bool placed = false;
	IRR_BIN old_sec = csect; //Copy of the section in case no candidates fit in the new section
	vector<PUNTO> sect_points = *csect.get_ptos();
	//Define the new section we are going to place pieces in
	int bin = csect.getBin();
	csect = Redefine_NewSection(sect_points, gc);
	csect.set_ID(id);
	csect.setBin(bin);
	sect_points = *csect.get_ptos();
	//Check wich of the items may fit in the new section in terms of area.
	int count = 0;
	while (count < item.size() && item[count].getArea() > csect.getArea())
		count++; //Find first piece that may fit in terms of area.
	while (!placed && count < item.size()) {
		if (!item[count].is_placed()) {
			placed = PieceInSect(item[count], csect); //Place piece
			count++;
		} else
			count++;
	}
	if (!placed)
		csect = old_sec;
	return placed;
}

//Fucntion that creates the candidate list. 
bool
edge_of_section(EDGES ed, IRR_BIN ib);
double
match_dist(EDGES e1, EDGES e2);

vector<PIEZA*> CreateList(vector<PIEZA> &item, IRR_BIN &sect) {
	int LIST_SIZE = 5;
	vector<PIEZA*> list;
	PIEZA convexhull;
	convexhull = create_convexhull_in_section(sect);
	//convexhull = create_rectencl_in_section(sect); //Rectangle enclosure for the rectangle instances.

	vector<EDGES> *e_ch = convexhull.getEdges(); //Edges of the convex hull.
	vector<EDGES> *e_item;
	double area_ch = convexhull.getArea();
	double area_sect = sect.getArea();
	int p = 0;
	bool is_ed_sect = false;

	while (item[p].is_placed()) {
		p++; //move to the first non placed piece
	}
	while (list.size() < LIST_SIZE && p < item.size()) {
		bool is_item_list = false;
		if (item[p].getArea() + area_ch < area_sect && !item[p].is_placed()) //piece is candidate in terms of area, not being the list and not previusly placed.
				{
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
						if (md > 0.8) //if edges of convex hull and edges of piece are likely in lenght
								{
							list.push_back(&item[p]); //include piece in the list.
							is_item_list = true;
							p++;
							break;
						}
					}
				}
			}
			if (!is_item_list) //Item did not make it to the list, because edges are not of matching distance
				p++; //try next piece.

		} else
			p++; //piece does not fit in terms of area, go to next piece.
	}
	return list;
}

////Function that repacks the last bin, with a more suitable size.
bool
orden_area(PIEZA i, PIEZA j);
void
fill_node(NODE &current_bin, vector<PIEZA> &pzas);
void RepackLBin(const char *argv, vector<NODE> &bin_sol) {

	NODE last_bin;
	//Reads type of bin in file.
	int bsizes = 0;
	string taux;
	string cadena;
	string line;
	vector<string> btype;
	ifstream b_file(argv);
	if (!b_file.is_open()) {
		cout << "ERROR: Unable to open file " << argv << "\n";
		exit(1);
	}
	getline(b_file, line); //'Size'  'L'  'W'
	while (!b_file.eof()) {
		b_file >> cadena; //Reads type of bin.
		taux = cadena;
		btype.push_back(taux);
		getline(b_file, line);
		++bsizes;
	}
	//==========================
	//get pieces from last node
	vector<NODE>::iterator it_lbin;
	it_lbin = bin_sol.end();
	it_lbin--;
	vector<PIEZA> pz_bin;
	for (int i = 0; i < it_lbin->getPI().size(); i++) {
		PIEZA p = *(it_lbin->getPI())[i];
		//Place pieces in its original position
		PUNTO origen;
		origen.coordx = 0.0;
		origen.coordy = 0.0;
		p.set_mirror(false);
		p.obtener_rotacion(0);
		p.setCoord(origen);
		//Add piece to vector of pieces.
		pz_bin.push_back(p);
	}
	//========================
	double area_pz = 0;
	for (int i = 0; i < pz_bin.size(); i++) //calculate area of pieces in last bin.
		area_pz = area_pz + pz_bin[i].getArea();
	int t = 0;
	int area_bin = 0;
	while (t < btype.size() && area_bin < area_pz - TOL) {
		last_bin.initialize_node(argv, bin_sol.size(), btype[t]);
		area_bin = last_bin.getL() * last_bin.getW();
		if (area_bin < area_pz - TOL)
			t++;
	}
	if (equal_double(area_bin, it_lbin->getW() * it_lbin->getL(), TOL)) //the only bin in which the remaining pieces fit, is the large one, so nothing to do.
		return;
	last_bin.set_ID_pzas_disp(pz_bin);
	vector<PIEZA> cp_pzas = pz_bin; //Copy of the pieces in case we need to repack in more than one bin.
	do {
		//order pieces from last bin by area.
		sort(pz_bin.begin(), pz_bin.end(), orden_area);
		//Place first piece
		OpenBin(last_bin, pz_bin[0]);
		if (last_bin.getPI().empty()) {
			t++; //Biggest piece do not fit in bin.
			continue;
		} else
			pz_bin.erase(pz_bin.begin());
		if (pz_bin.empty())
			continue; //There was only one piece to place.
		fill_node(last_bin, pz_bin);
		if (!last_bin.get_IDdisp().empty()) //Not all the pieces fit in the selected bin
		{
			t++; //Move to next size bin
			pz_bin.clear();
			pz_bin = cp_pzas;
			last_bin.empty_bin();
			last_bin.initialize_node(argv, bin_sol.size(), btype[t]);
			last_bin.set_ID_pzas_disp(pz_bin);
		}
	} while (t < btype.size() && !last_bin.get_IDdisp().empty());

	if (last_bin.get_IDdisp().empty()) //Repacked in smaller bin
	{
		bin_sol.pop_back(); //Eliminate the last bin from the solution
		bin_sol.push_back(last_bin); //Add new bin. 
	}
	return;
}
