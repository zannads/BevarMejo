
#include "epanet2_2.h"
#include "types.h"

#include "bevarmejo/io.hpp"

#include "water_distribution_system.hpp"

namespace bevarmejo {
namespace wds {

WaterDistributionSystem::WaterDistributionSystem(const std::filesystem::path& inp_file) :
    ph_(nullptr),
    _inp_file_(inp_file),
    _elements_(),
    _nodes_(),
    _links_(),
    m__aux_elements_(),
    _junctions_(),
    _tanks_(),
    _reservoirs_(),
    _pipes_(),
    _pumps_(),
    _subnetworks_(),
    _groups_(),
    m__config_options()
    {
        // I need at least a time series for the constant and one for the results
        m__time_series_map.emplace("Constant", aux::TimeSeries(m__config_options.times.global));
        m__time_series_map.emplace("Results", aux::TimeSeries(m__config_options.times.global));

        load_from_inp_file(inp_file);
    }

} // namespace wds
} // namespace bevarmejo