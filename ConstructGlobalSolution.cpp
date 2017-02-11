//
//  ConstructGlobalSolution.cpp
//  BS_HetBins
//
//  Created by Marta Cabo Nodar on 27/06/16.
//  Copyright © 2016 Marta Cabo Nodar. All rights reserved.
//

#include "classes_BPGC_HetBins.h"
void OpenBin(NODE &bin, PIEZA &item);
void fill_node(NODE &current_bin, vector<PIEZA> &pzas);
vector<PIEZA> set_available_pzas(NODE &father, vector<PIEZA> &all_pzas);
double AreaToPack(NODE node, vector <PIEZA> pzas);

//pza holds all non-placed pieces on the branch we are exploring.
// (proyeccion)
double Construct_Solution(const char *bin_name, vector <PIEZA> pzas, double level,double &A_bin, double &Amax)
{
    vector<NODE> bin_sol; //(incluir esto también en la mejor solución)
    vector<NODE> current_bin;//Vector of size btype_size for each type of bin, at each level.
    //Reads type of bin in file.
    int bsizes = 0;
    string taux;
    string cadena;
    string line;
    vector<string> btype;
    ifstream b_file(bin_name);
    if(!b_file.is_open())
    {
        cout<<"ERROR: Unable to open file "<<bin_name<<"\n";
        exit(1);
    }
    getline(b_file,line); //'Size'  'L'  'W'
    //Create a btype vector, that holds each type of bin 0 = 'S', 1= 'M'...
    while(!b_file.eof())
    {
        NODE naux;
        b_file >> cadena;//Reads type of bin.
        taux = cadena;
        btype.push_back(taux);
        getline(b_file, line);
        ++bsizes;
        naux.initialize_node(bin_name, bsizes, btype[bsizes-1]);
        current_bin.push_back(naux);
    }
    //==========================
    //Placing the first piece in the first bin that fits.
    vector<int> ID_placed;
    while(ID_placed.size()<pzas.size())    //Until all pieces are placed.
    {
        int b = 0;
        int p = 0;
        //Access to the first piece not in the ID_placed
        while(find(ID_placed.begin(),ID_placed.end(),pzas[p].getID()) != ID_placed.end())
            p++;
        while(pzas[p].getArea() > current_bin[b].get_waste() && b<current_bin.size())
            b++;
        double best_bin_use = -GRANDE;
        double worst_bin_use = GRANDE;

        int best_binIdx = 0;
        for(int bb  = b ; bb<current_bin.size();bb++)
        {
            //open a bin
            if( bin_sol.empty())//Update pzas disponibles
                current_bin[bb].set_ID_pzas_disp(pzas);
            else //some pieces have been placed,
                //Need to retrieve them and update pzas disponibles.
            {
                vector<PIEZA> p_disp;
                p_disp = set_available_pzas(bin_sol[bin_sol.size()-1], pzas);
                current_bin[bb].set_ID_pzas_disp(p_disp);
            }
            OpenBin(current_bin[bb], pzas[p]);
            if(!current_bin[bb].getPI().empty())
            {
                //Fill the bin
                fill_node(current_bin[bb], pzas);
                //============================================================
                //(A) Keep the best of all current_bin in terms of %utilization.
                //=========================================================
//                if (current_bin[bb].getPropUtil() > best_bin_use)
//                {
//                    best_bin_use = current_bin[bb].getPropUtil();
//                    best_binIdx = bb;
//                }
                //=========================================================
                //(B) Keep the best of all current_bin in terms of Absolute Waste.
                //==============================================================
//                if (current_bin[bb].get_waste() < worst_bin_use)
//                {
//                    worst_bin_use = current_bin[bb].get_waste();
//                    best_binIdx = bb;
//                }
               //============================================================
               //(C) Keep the best of all current bin, in terms of (AbsWaste/#pieces in bin)*#pieces to pack.
               //============================================================
//               double of_select = 0 ;
//                of_select = current_bin[bb].get_waste()/current_bin[bb].getNumPiezas();
//                of_select = of_select * current_bin[bb].get_IDdisp().size();
//                if (of_select < worst_bin_use)
//                {
//                    worst_bin_use = of_select;
//                    best_binIdx = bb;
//                }
                //============================================================
                //(D) Keep the best of all current bin, in terms of  (%Utilization/#pieces in bin)*#pieces to pack
                //============================================================
//                double of_select = 0 ;
//                of_select = current_bin[bb].getPropUtil()/current_bin[bb].getNumPiezas();
//                of_select = of_select * current_bin[bb].get_IDdisp().size();
//                if (of_select > best_bin_use)
//                {
//                    best_bin_use = of_select;
//                    best_binIdx = bb;
//                }
                //============================================================
                //(E) Keep the min (Abs waste/ total Area of pieces in bin) × (area pieces to pack /#pieces left to pack)
                //============================================================
//                double of_select = 0 ;
//                of_select = current_bin[bb].get_waste();
//                double area_bin=0;
//                for(int i = 0; i<current_bin[bb].getNumPiezas(); i++)
//                {
//                    PIEZA p = *current_bin[bb].getPI()[i];
//                    area_bin = area_bin + p.getArea();
//                }
//                of_select = of_select/area_bin;
//                //Find area of all pieces with ID in ID_disp
//                double area_to_pack;
//                if(!current_bin[bb].get_IDdisp().empty())
//                {
//                    area_to_pack = AreaToPack(current_bin[bb], pzas);
//                    of_select = of_select *area_to_pack/current_bin[bb].get_IDdisp().size();
//                }
//                else of_select = 0.0;
//                if (of_select <worst_bin_use)
//                {
//                    worst_bin_use = of_select;
//                    best_binIdx = bb;
//                }
                //============================================================
                //(E) Keep the min suma[(Area pza)^2/Amax(por empaquetar)
                //Discussed with Toni
                //============================================================
                double of_select = 0 ;
                double area_pzas_bin=0;
                for(int i = 0; i<current_bin[bb].getNumPiezas(); i++)
                {
                    PIEZA p = *current_bin[bb].getPI()[i];
                    area_pzas_bin = area_pzas_bin + pow(p.getArea(),2);
                }
                double area_bin = current_bin[bb].getW()*current_bin[bb].getL();
                of_select = area_pzas_bin /(area_bin*Amax);
                
//                //Find area of all pieces with ID in ID_disp
//                double area_to_pack;
//                if(!current_bin[bb].get_IDdisp().empty())
//                {
//                    area_to_pack = AreaToPack(current_bin[bb], pzas);
//                    area_to_pack = area_to_pack+area_pzas_bin;
//                    of_select = (1.0/pow(area_pzas_bin/area_to_pack,0.5))* current_bin[bb].get_waste();
//                }
//                else of_select = 0.0;
                if (of_select > best_bin_use)
                {
                    best_bin_use = of_select;
                    best_binIdx = bb;
                }

                //============================================================
                //(F) Keep the min (Abs waste/ av Area of pieces in bin) × (area pieces to pack /#pieces left to pack)
                //============================================================
//                double of_select = 0 ;
//                of_select = current_bin[bb].get_waste();
//                double av_area_bin=0;
//                for(int i = 0; i<current_bin[bb].getNumPiezas(); i++)
//                {
//                    PIEZA p = *current_bin[bb].getPI()[i];
//                    av_area_bin = av_area_bin + p.getArea();
//                }
//                av_area_bin = av_area_bin/current_bin[bb].getNumPiezas();
//                of_select = of_select/av_area_bin;
//                //Find area of all pieces with ID in ID_disp
//                double area_to_pack;
//                if(!current_bin[bb].get_IDdisp().empty())
//                {
//                    area_to_pack = AreaToPack(current_bin[bb], pzas);
//                    of_select = of_select *area_to_pack/current_bin[bb].get_IDdisp().size();
//                }
//                else
//                    of_select = 0.0;
//                if (of_select <worst_bin_use)
//                {
//                    worst_bin_use = of_select;
//                    best_binIdx = bb;
//                }
                //============================================================
                //(G) Keep the min (Abs waste/ av Area of pieces in bin) × (area pieces to pack)
                //============================================================
//                double of_select = 0 ;
//                of_select = current_bin[bb].get_waste();
//                double av_area_bin=0;
//                for(int i = 0; i<current_bin[bb].getNumPiezas(); i++)
//                {
//                    PIEZA p = *current_bin[bb].getPI()[i];
//                    av_area_bin = av_area_bin + p.getArea();
//                }
//                av_area_bin = av_area_bin/current_bin[bb].getNumPiezas();
//                of_select = of_select/av_area_bin;
//                //Find area of all pieces with ID in ID_disp
//                double area_to_pack;
//                area_to_pack = AreaToPack(current_bin[bb], pzas);
//                of_select = of_select *area_to_pack;
//                of_select = of_select/1000; //Divide by 1000 to reduce the magnitude of the selecting value, so comparisons with a really big number will have meaning. 
//                if (of_select <worst_bin_use)
//                {
//                    worst_bin_use = of_select;
//                    best_binIdx = bb;
//                }
            }//endif
        }//endfor bb
        //Add current_bin to bin_sol and update placed pzas.
        vector<PIEZA*> pbin = current_bin[best_binIdx].getPI();
        for (int p = 0; p<pbin.size();p++)
        {
            ID_placed.push_back(pbin[p]->getID());
        }
        bin_sol.push_back(current_bin[best_binIdx]);
        for( int i = 0; i<current_bin.size();i++)
            current_bin[i].empty_bin() ;
    }//end while
    double best_of = 0;
    double ABins = 0;
    for(int i = 0; i<bin_sol.size()-1;i++)
    {
//        cout<<"\nBin: "<<bin_sol[i].getSize()<< " Pieces: ";
//        for(int j = 0; j<bin_sol[i].getPI().size();j++)
//        {
//            int IDpza = (bin_sol[i].getPI())[j]->getID();
//            cout<< IDpza<<" " ;
//        }
//        cout<<"\n";
//        //===========================================
        //Objective Function: Area Bin
        //===========================================
        ABins = ABins + bin_sol[i].getL()*bin_sol[i].getW();
    }
    int nBins = bin_sol.size()-1;
    ABins = ABins + bin_sol[nBins].getPropUtil()*bin_sol[nBins].getW()*bin_sol[nBins].getL();
    best_of = A_bin + ABins;
    return best_of;
}

double AreaToPack(NODE node,vector <PIEZA> pzas)
{
    double area=0;
    for (int i=0; i<node.get_IDdisp().size();i++)
    {
        int id_to_pack = node.get_IDdisp()[i];
        int p = 0;
        while(pzas[p].getID() != id_to_pack) p++;
        area = area +  pzas[p].getArea();

    }
    
    return area;
}
