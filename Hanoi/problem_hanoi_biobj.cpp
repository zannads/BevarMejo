#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/wds/user_defined_elements_group.hpp"
#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/elements/network_element.hpp"
#include "bevarmejo/wds/elements/node.hpp"
#include "bevarmejo/wds/elements/link.hpp"
#include "bevarmejo/wds/elements/junction.hpp"
#include "bevarmejo/wds/auxiliary/demand.hpp"
#include "bevarmejo/wds/elements/source.hpp"
#include "bevarmejo/wds/elements/tank.hpp"
#include "bevarmejo/wds/elements/pipe.hpp"
#include "bevarmejo/wds/elements_group.hpp"

#include "bevarmejo/io.hpp"
#include "bevarmejo/wds/epanet_helpers/en_help.hpp"

#include "problem_hanoi_biobj.hpp"

namespace fsys = std::filesystem;
namespace bevarmejo {
namespace hanoi {
namespace fbiobj {

Problem::Problem(const json& settings, const std::vector<std::filesystem::path>& lookup_paths) {
    assert(settings != nullptr);

    // Add here load of problem specific settings.
    auto file = bevarmejo::io::locate_file(fsys::path{settings["WDS"]["inp"]}, lookup_paths);
    if (!file.has_value()) {
        throw std::runtime_error("The provided inp file does not exist in the lookup paths. Check the settings file.\n");
	}
    auto inp_filename = file.value();

    m_hanoi= std::make_shared<WDS>(inp_filename);

    // Load the "UDEG" from the constexpr array to have the pipes always in the same order (element group uses a set so it is not guaranteed)
    wds::Subnetwork changeable_pipes;
    for (const auto& id : changeable_pipe_ids) {
        // find it in the network
        auto it = m_hanoi->pipes().find(id);
        assert(it != m_hanoi->pipes().end());
        // add it to the subnetwork
        changeable_pipes.insert(*it);
    }
    assert(changeable_pipes.size() == n_dv); // instead of checking the result of the insert singularly
    m_hanoi->add_subnetwork(label::__changeable_pipes, changeable_pipes);

    // Compute the cost of the diameters of the pipes as it is constant and the most expensive operation as it requires the std::pow
    // I do it here to avoid doing it at every fitness evaluation
    for (auto i = 0u; i!= n_available_diams; ++i) {
        m_diams_cost[i] = a * std::pow(available_diams_in[i], b);
    }
}

std::vector<double> Problem::fitness(const std::vector<double>& dv) const {

    // Apply the dv
    apply_dv(*m_hanoi, dv);

    // calculte the cost as it doesn't depend on any simulation
    double cost = this->cost(dv);

    // Calculate the reliability 
    //     I need to run the sim first 
    try
    {
        m_hanoi->run_hydraulics();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return {std::numeric_limits<double>::max(), std::numeric_limits<double>::max()};
    }
    
    // Change sign as reliability needs to be maximized
    double ir = resilience_index_from_min_pressure(*m_hanoi, min_head_m).at(0);
    if (ir > 0.) // means it worked
        ir = -ir;
    else // penalty based on head deficit
        ir = pressure_deficiency(*m_hanoi, min_head_m, /*relative=*/ true).at(0);

    // Return the fitness
    return {cost, ir};
}

std::pair<std::vector<double>, std::vector<double>> Problem::get_bounds() const {
    // I use the integer of the decision variable to choose the index of the diam 
    // from the available diameters. So between 0 and size of available diameters array.
    std::vector<double> lb(n_dv, 0.);
    std::vector<double> ub(n_dv, n_available_diams-1);

    return std::make_pair(lb, ub);
}

double Problem::cost(const std::vector<double>& dv) const {
    // Calculate the cost of the network
    // C sum_{i=0}^{n_pipe} (pipe_i = a * D_i^b * L_i)

    double cost = 0.;
    
    auto pipes = m_hanoi->subnetwork(label::__changeable_pipes);
    auto itp = pipes.begin();
    auto itd = dv.begin();
    assert(pipes.size() == dv.size());
    while (itp != pipes.end()) {
        auto p_pipe = std::dynamic_pointer_cast<wds::Pipe>(itp->lock());
        assert(p_pipe != nullptr);

        //C  +=   a * D_i^b                                  * L_i
        cost += m_diams_cost[static_cast<std::size_t>(*itd)] * p_pipe->length().value();

        ++itp;
        ++itd;
    }
    
    return cost;
}

void Problem::apply_dv(wds::WaterDistributionSystem& a_wds, const std::vector<double>& dv) const {
    // Apply the decision variables to the network
    // I use the integer of the decision variable to choose the index of the diam 
    // from the available diameters. So between 0 and size of available diameters array.
    a_wds.cache_indices();

    auto pipes = a_wds.subnetwork(label::__changeable_pipes);
    auto itp = pipes.begin();
    auto itd = dv.begin();
    assert(pipes.size() == dv.size());
    while (itp != pipes.end()) {
        double diam_mm = available_diams_in.at(static_cast<std::size_t>(*itd))/12.*MperFT*1000.;
        auto p_pipe = std::dynamic_pointer_cast<wds::Pipe>(itp->lock());
        p_pipe->diameter(diam_mm);

        // unfortunatley I have to do the same in EPANET
        int errco = 0;
        errco = EN_setlinkvalue(a_wds.ph_, p_pipe->index(), EN_DIAMETER, diam_mm);

        ++itp;
        ++itd;
    }
}
    
} // namespace fbiobj
} // namespace hanoi
} // namespace bevarmejo
    