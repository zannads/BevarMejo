/*----------------------
Author: DennisZ
descr: Model of the Hanoi problem.
----------------------*/

#include "model_hanoi.hpp"

namespace bevarmejo {

ModelHanoi::ModelHanoi(){
    // Initialize an empty object.
}

ModelHanoi::ModelHanoi(std::string settings_file){
    // Load the file and check
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(settings_file.c_str());
    
    if (result.status != pugi::status_ok){
        throw std::runtime_error(result.description());
    }
    
    std::filesystem::path rootDataFolder{doc.child("rootDataFolder").child_value()};
    
    std::filesystem::path inpFile{doc.child("optProblem").child("hanoi").child("inpFile").child_value()};
    inpFile = rootDataFolder/inpFile;
    
    std::filesystem::path avDiams{doc.child("optProblem").child("modelHanoi").child("avDiams").child_value()};
    avDiams = rootDataFolder/avDiams;
    
    load_availDiam(avDiams.c_str());
    
    _hanoi_ = std::make_shared<bevarmejo::Hanoi>();
    _hanoi_->set_inpfile( inpFile.string() );
    _hanoi_->init();
}

ModelHanoi::ModelHanoi(const ModelHanoi &src) : _hanoi_(src._hanoi_), _av_diams_(src._av_diams_) {};

ModelHanoi::ModelHanoi(ModelHanoi &&src) : _hanoi_(std::move(src._hanoi_)),
_av_diams_(std::move(src._av_diams_)) {};


/*ModelHanoi::~ModelHanoi(){
    std::cout <<"deleting MOdelHAnoi" <<std::endl;
}*/

void ModelHanoi::upload_settings(std::string settingsFile){
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
    
    _hanoi_->set_inpfile( inpFile.string() );
}

// Number of objective functions
std::vector<double>::size_type ModelHanoi::get_nobj() const{
    return 2;
}

// Number of equality constraints
std::vector<double>::size_type ModelHanoi::get_nec() const{
    return 0;
}

// Number of INequality constraints
std::vector<double>::size_type ModelHanoi::get_nic() const{
    return 0;
}

// Number of integer decision variables 
std::vector<double>::size_type ModelHanoi::get_nix() const{
    //TODO
    return 34;
}

// Implementation of the objective function.
std::vector<double> ModelHanoi::fitness(const std::vector<double>& dv) const{
    std::vector<double> fit(2,0);
    
    // Fit the solution
    applySolution(dv);
    
    // Run the simulation
    std::vector<double> nodePressures = _hanoi_->evaluate();
    
    // Get the result of the functions defined on it
    fit[0] = cost();
    fit[1] = minPressure(nodePressures);
    
    return fit;
}

// Implementation of the box bounds.
std::pair<std::vector<double>, std::vector<double>> ModelHanoi::get_bounds() const{
    std::vector<double> minB(34,0.);
    std::vector<double> maxB(34,5.);
    return {minB, maxB};
}

void ModelHanoi::applySolution(const std::vector<double>& dv) const{
    
    int error;
    
    int linkIdx;
    // since ids are only simple numbers I trnasfomr them directly from int to string
    std::string linkFakeName;
    double val{0.};
    
    for( unsigned int i=0; i<dv.size(); ++i){
        linkFakeName = std::to_string(i+1);
        
        error = EN_getlinkindex(_hanoi_->ph_, linkFakeName.c_str(), &linkIdx);
        if (error > 100)
            throw std::runtime_error("Diam not set.");
        
        
        error = EN_setlinkvalue(_hanoi_->ph_, linkIdx, EN_DIAMETER, _av_diams_[dv[i]].millimeters);
        if (error > 100)
            throw std::runtime_error("Diam not set.");
    }
}


double ModelHanoi::cost() const{
    // C_{ij} = 1.1 * D_{ij}^1.5 * L_{ij}
    
    // Get number of link
    int error;
    int nLinks;
    
    error = EN_getcount(_hanoi_->ph_, EN_LINKCOUNT, &nLinks);
    if (error > 100)
        throw std::runtime_error("Number of links not retrieved.");
    
    double totalcost{0.}, diam_mm{0.}, leng_m{0.};
    
    // Per each link, if it is a pipe add to total cost
    
    int linkType;
    int linkIdx;
    // since ids are only simple numbers I trnasfomr them directly from int to string
    std::string linkFakeName;
    
    for (int link = 0; link < nLinks; ++link){
        linkFakeName = std::to_string(link+1);
        
        error = EN_getlinkindex(_hanoi_->ph_, linkFakeName.c_str(), &linkIdx);
        
        error = EN_getlinktype(_hanoi_->ph_, linkIdx, &linkType);
        if (error > 100)
            throw std::runtime_error("Link's type not retrieved.");
        
        if (linkType == EN_PIPE){
            // Get diameter and length (in millimeter and in meters)
            error = EN_getlinkvalue(_hanoi_->ph_, linkIdx, EN_DIAMETER, &diam_mm);
            error = EN_getlinkvalue(_hanoi_->ph_, linkIdx, EN_LENGTH, &leng_m);
            
            double diam_cost{0.};
            std::vector<double>::size_type i = _av_diams_.size();
            while( i ){
                --i;
                if( diam_mm == _av_diams_[i].millimeters ){
                    diam_cost = _av_diams_[i].inches_cost;
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

double ModelHanoi::minPressure(std::vector<double>& pressures) const{
    
    double cumPress{0};
    
    for(int i=0; i<pressures.size(); ++i){
        cumPress += pressures[i]>=30?0:(-pressures[i]+30);
    }
    
    return cumPress;
}

void ModelHanoi::load_availDiam(std::string filename){
    std::ifstream fs(filename);
    if (!fs.is_open()) {
        std::string errorMessage{"Failed to open the file: "};
        errorMessage.append(filename);
        throw std::runtime_error( errorMessage );
    }
    
    std::string sJunk = "";
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
        _av_diams_.push_back(tempData);
    }
    
    //Return to the beginning of the file
    fs.seekg(0, std::ios::beg);
    
    fs.close();
}

} /* namespace bevarmejo */
