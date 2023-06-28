/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem.
----------------------*/

#include "model_hanoi.hpp"
using namespace::std;

model_hanoi::model_hanoi(){
    // Initialize an empty object.
}

void model_hanoi::upload_settings(std::string settingsFile){
    // Load the file and check
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(settingsFile.c_str());
        
        if (result.status != pugi::status_ok){
            throw std::runtime_error(result.description());
        }
        
    std::filesystem::path rootDataFolder{doc.child("rootDataFolder").child_value()};
    
    std::filesystem::path inpFile{doc.child("optProblem").child("hanoi").child("inpFile").child_value()};
    inpFile = rootDataFolder/inpFile;
    
    std::filesystem::path avDiams{doc.child("optProblem").child("modelHanoi").child("avDiams").child_value()};
    avDiams = rootDataFolder/avDiams;
    
    load_availDiam(avDiams.c_str());
    
    Hanoi.set_inpFile(inpFile.c_str());
    
    Hanoi.init();
}

// Number of objective functions
std::vector<double>::size_type model_hanoi::get_nobj() const{
	return 2;
}

// Number of equality constraints
std::vector<double>::size_type model_hanoi::get_nec() const{
	return 0;
}

// Number of INequality constraints
std::vector<double>::size_type model_hanoi::get_nic() const{
	return 0;
}

// Number of integer decision variables 
std::vector<double>::size_type model_hanoi::get_nix() const{
	//TODO
	return 34;
}

// Implementation of the objective function.
    std::vector<double> model_hanoi::fitness(const std::vector<double>& dv) const{
        std::vector<double> fit(2,0);
        
        // Fit the solution
        applySolution(dv);

    	// Run the simulation
        std::vector<double> nodePressures = Hanoi.evaluate();

    	// Get the result of the functions defined on it
        fit[0] = cost();
        fit[1] = minPressure(nodePressures);
        
    	return fit;
    }

    // Implementation of the box bounds.
    std::pair<std::vector<double>, std::vector<double>> model_hanoi::get_bounds() const{
        vector<double> minB(34,0.);
        vector<double> maxB(34,5.);
    	return {minB, maxB};
    }

void model_hanoi::applySolution(const std::vector<double>& dv) const{

    int error;
    
    int linkIdx;
    // since ids are only simple numbers I trnasfomr them directly from int to string
    string linkFakeName;
    double val{0.};
    
    for( unsigned int i=0; i<dv.size(); ++i){
        linkFakeName = to_string(i+1);
        
        error = EN_getlinkindex(Hanoi.ph, linkFakeName.c_str(), &linkIdx);
        if (error > 100)
            throw std::runtime_error("Diam not set.");
        
        
        error = EN_setlinkvalue(Hanoi.ph, linkIdx, EN_DIAMETER, av_diams[dv[i]].millimeters);
        if (error > 100)
            throw std::runtime_error("Diam not set.");
    }
}


double model_hanoi::cost() const{
    // C_{ij} = 1.1 * D_{ij}^1.5 * L_{ij}
    
    // Get number of link
    int error;
    int nLinks;
    
    error = EN_getcount(Hanoi.ph, EN_LINKCOUNT, &nLinks);
    if (error > 100)
        throw std::runtime_error("Number of links not retrieved.");
    
    double totalcost{0.}, diam_mm{0.}, leng_m{0.};
    
    // Per each link, if it is a pipe add to total cost
    
    int linkType;
    int linkIdx;
    // since ids are only simple numbers I trnasfomr them directly from int to string
    string linkFakeName;
    
    for (int link = 0; link < nLinks; ++link){
        linkFakeName = to_string(link+1);
        
        error = EN_getlinkindex(Hanoi.ph, linkFakeName.c_str(), &linkIdx);
        
        error = EN_getlinktype(Hanoi.ph, linkIdx, &linkType);
        if (error > 100)
            throw std::runtime_error("Link's type not retrieved.");
        
        if (linkType == EN_PIPE){
            // Get diameter and length (in millimeter and in meters)
            error = EN_getlinkvalue(Hanoi.ph, linkIdx, EN_DIAMETER, &diam_mm);
            error = EN_getlinkvalue(Hanoi.ph, linkIdx, EN_LENGTH, &leng_m);
            
            double diam_cost{0.};
            std::vector<double>::size_type i = av_diams.size();
            while( i ){
                --i;
                if( diam_mm == av_diams[i].millimeters ){
                    diam_cost = av_diams[i].inches_cost;
                    i = 0;
                }
            }
            
            // Compute
            totalcost += diam_cost*leng_m;
        }
    }
    totalcost *= 1.1;
        
    return totalcost;
}

double model_hanoi::minPressure(vector<double>& pressures) const{
    
    double cumPress{0};
    
    for(int i=0; i<pressures.size(); ++i){
        cumPress += pressures[i]>=30?0:(-pressures[i]+30);
    }
    
    return cumPress;
}

void model_hanoi::load_availDiam(const char* filename){
    ifstream fs(filename);
    if (!fs.is_open()) {
        string errorMessage{"Failed to open the file: "};
        errorMessage.append(filename);
        throw std::runtime_error( errorMessage );
    }
    
    string sJunk = "";
    //Look for the <#DATA> key
    while (sJunk != "#DATA")
    {
        fs >> sJunk;
    }
    int rows;
    fs >> rows;
    
    diamData tempData{0., 0., 0.};
    for (int i=0; i<rows; ++i){
        fs >> tempData.inches;
        fs >> tempData.millimeters;
        fs >> tempData.inches_cost;
        av_diams.push_back(tempData);
    }
    
    //Return to the beginning of the file
    fs.seekg(0, ios::beg);
    
    fs.close();
}


void model_hanoi::clear(){
    Hanoi.clear();
}
