//
//==========================================================
// Beam search algorithm for the 2DBPPGC and Heterogeneus Bins.
//
//  Created by Marta Cabo Nodar on 14/06/16.
//  Copyright (c) 2016 Marta Cabo. All rights reserved.
//
//==========================================================

#include "classes_BPGC_HetBins.h"
#include <mach-o/dyld.h>	/* _NSGetExecutablePath */

//Declaración de funciones necesarias
vector<PIEZA> read_pieces(char *name);
bool orden_area(PIEZA i , PIEZA j);
void PrintSolution(char *instance_name, TREE &bs_sol, double runtime, double &A);
void PrintTree(char *instance_name, TREE &bs_sol, double runtime);

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)


int main(int argc, const char * argv[]) {
    
//    char path[1024];
//    uint32_t size = sizeof(path);
//    if (_NSGetExecutablePath(path, &size) == 0)
//        printf("executable path is %s\n", path);
//    else
//        printf("buffer too small; need size %u\n", size);
    
    ofstream fp;
    
    fp.open("Results.txt", ios_base::app);
    if (!fp.is_open())
    {
        cout<< "ERROR: Unable to open Results file\n";
        exit(1);
    }
    for(int k = 2; k<argc; k++){//For all instances. 
    //Reads instance name
    char instance_name [120];
    strcpy(instance_name,argv[k]);
    //Read pieces
    vector<PIEZA> item;
    item = read_pieces(instance_name);
    //============================
    // Order pieces
    //By area
    sort(item.begin(), item.end(), orden_area);
    double AreaPz = 0;
    for (int i = 0; i<item.size(); i++)
        AreaPz = AreaPz + item[i].getArea();

    //============================
    TREE bs_sol;
    clock_t start = clock();
    bs_sol.build_solution(argv[1], item);
    clock_t finish = clock();
    double runtime = (finish-start)/CLOCKS_PER_MS;//Running time in milliseconds.
    runtime  = runtime/1000;//Running time in seconds, without loosing precission.
    
    //Print solution
    //===================================================
    PrintSolution(instance_name,bs_sol, runtime, AreaPz);
//    PrintTree(instance_name,bs_sol, runtime);
    //===================================================

    //Identify solution node
    //======================
    list<NODE>::iterator it_child;
    list<NODE> tree;
    tree = bs_sol.get_tree();
    it_child = tree.end();
    it_child --;
    int bestID = -1;
    double bestUsage = -GRANDE;
    while(it_child != tree.begin())
    {
        
        if(it_child->get_IDdisp().empty())
        {
            //Find %Utilization on that branch
            NODE branch;
            branch = *it_child;
            double AreaB = 0;
            int level = 0;
            while(branch.get_Pred() != NULL)
            {
                //OF: %Utilization = Area Pzas /(Area Bins)
                AreaB = AreaB + branch.getL()*branch.getW();
                //============================================
                branch = *branch.get_Pred();
            }
            double Util =AreaPz/AreaB;
            if(bestUsage <Util)
            {
                //Keep the info for the branch with best OF
                bestUsage = Util;
                bestID = it_child->getID();
            }
        }
        it_child --;
    }

    //Finding Bin Mix
    it_child = tree.end();
    it_child--;
    int no_bins = 0;
    vector<string> bin_mix;
    while (it_child != tree.begin())
    {
        if(it_child->getID() == bestID)
        {
            NODE child;
            child = *it_child;
            while(child.get_Pred() != NULL)//While not at FATHER level
            {
                string s = child.getSize();
                bin_mix.push_back(s);
                child = *child.get_Pred();
                no_bins ++;
            }
        }
        it_child--;
        
    }
   
    cout<<"\nInstance: "<<instance_name<<"\n";
    cout<<"Number of bins: "<<no_bins<<"\n";
    cout<<"Bin Utilization: "<<bestUsage<<"\n";
    cout<<"Running time: "<<runtime<<"\n";
    fp<<instance_name<<"\t"<<no_bins<<"\t"<<bestUsage<<"\t"<<runtime<<"\t";
    fp<<"[ ";
    for(int i = 0;i<bin_mix.size()-1;i++)
        fp<< bin_mix[i]<<" - ";
    fp<<bin_mix[bin_mix.size()-1]<<"]\n";
    }
    return 0;
}
