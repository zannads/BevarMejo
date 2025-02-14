#include <cassert>
#include <filesystem>
namespace fsys = std::filesystem;
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>


#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"
#include "bevarmejo/wds/water_distribution_system.hpp"

#include "bevarmejo/utility/io.hpp"
namespace bemeio = bevarmejo::io;

#include "bevarmejo/wds/utility/epanet/en_help.hpp"

#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"



#include "problems/hanoi.hpp"

namespace bevarmejo {
namespace hanoi {
namespace fbiobj {

static const std::string name = "bevarmejo::hanoi::fbiobj";
static const std::string extra_info = "\tFormulation of the Hanoi problem using cost and reliablity.\n";

Problem::Problem(const Json& settings, const bemeio::Paths& lookup_paths)
{
    assert(settings != nullptr);

    // Add here load of problem specific settings.
    auto inp_filename = bevarmejo::io::locate_file(settings["WDS"]["inp"].get<fsys::path>(), lookup_paths);

    m_hanoi= std::make_shared<WDS>(inp_filename);

    // Load from the constexpr array the IDs of the pipes that can be changed.
    m_hanoi->submit_id_sequence(label::__changeable_pipes, std::vector<std::string>(changeable_pipe_ids.begin(), changeable_pipe_ids.end()));

    // Compute the cost of the diameters of the pipes as it is constant and the most expensive operation as it requires the std::pow
    // I do it here to avoid doing it at every fitness evaluation
    for (auto i = 0u; i!= n_available_diams; ++i) {
        m_diams_cost[i] = a * std::pow(available_diams_in[i], b);
    }

    m__name = name;
    m__extra_info = extra_info;
}

std::vector<double> Problem::fitness(const std::vector<double>& dv) const {

    // Apply the dv
    apply_dv(*m_hanoi, dv);

    // calculte the cost as it doesn't depend on any simulation
    double cost = this->cost(dv);

    // Calculate the reliability 
    //     I need to run the sim first 
    sim::solvers::epanet::HydSimSettings settings;

    long h_step = 0;
    int errorcode = EN_gettimeparam(m_hanoi->ph(), EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step = 0;
    errorcode = EN_gettimeparam(m_hanoi->ph(), EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(m_hanoi->ph(), EN_DURATION, &horizon);
    assert(errorcode < 100);

	settings.resolution(h_step);
	settings.report_resolution(r_step);
	settings.horizon(horizon);

	auto results = sim::solvers::epanet::solve_hydraulics(*m_hanoi, settings);

	if (!sim::solvers::epanet::is_successful(results))
	{
		bevarmejo::io::stream_out( std::cerr, "Error in the hydraulic simulation. \n");
		// reset_dv(m__anytown, dvs);
		return std::vector<double>(get_nobj()+get_nec()+get_nic(), 
					std::numeric_limits<double>::max());
	}
    
    // Change sign as reliability needs to be maximized
    double ir = resilience_index_from_min_pressure(*m_hanoi, min_head_m).when_t(0);
    if (ir > 0.) // means it worked
        ir = -ir;
    else // penalty based on head deficit
        ir = pressure_deficiency(*m_hanoi, min_head_m, /*relative=*/ true).when_t(0);

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

double Problem::cost(const std::vector<double>& dv) const
{
    // Calculate the cost of the network
    // C sum_{i=0}^{n_pipe} (pipe_i = a * D_i^b * L_i)

    double value = 0.;
    auto pipes = m_hanoi->subnetwork_with_order<WDS::Pipe>(label::__changeable_pipes);
    auto itp = pipes.begin();
    auto itd = dv.begin();
    assert(pipes.size() == dv.size());
    while (itp != pipes.end()) {
        auto&& [id, pipe] = *itp;
       
        //C  +=   a * D_i^b                                  * L_i
        value += m_diams_cost[static_cast<std::size_t>(*itd)] * pipe.length().value();

        ++itp;
        ++itd;
    }
    
    return value;
}

void Problem::apply_dv(WaterDistributionSystem& a_wds, const std::vector<double>& dv) const
{
    // Apply the decision variables to the network
    // I use the integer of the decision variable to choose the index of the diam 
    // from the available diameters. So between 0 and size of available diameters array.
    a_wds.cache_indices();

    auto pipes = a_wds.subnetwork_with_order<WDS::Pipe>(label::__changeable_pipes);
    auto itp = pipes.begin();
    auto itd = dv.begin();
    assert(pipes.size() == dv.size());
    while (itp != pipes.end()) {
        double diam_mm = available_diams_in.at(static_cast<std::size_t>(*itd))/12.*MperFT*1000.;
        auto&& [id, pipe] = *itp;

        // unfortunatley I have to do the same in EPANET
        int errco = 0;
        errco = EN_setlinkvalue(a_wds.ph_, pipe.EN_index(), EN_DIAMETER, diam_mm);

        ++itp;
        ++itd;
    }
}
    
} // namespace fbiobj
} // namespace hanoi
} // namespace bevarmejo
    