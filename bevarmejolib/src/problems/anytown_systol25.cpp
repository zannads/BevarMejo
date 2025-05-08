#include <string>
#include <string_view>

#include "bevarmejo/utility/exceptions.hpp"
#include "bevarmejo/io/aliased_key.hpp"
#include "bevarmejo/io/fsys.hpp"
#include "bevarmejo/io/json.hpp"
#include "bevarmejo/io/labels.hpp"
#include "bevarmejo/io/streams.hpp"
namespace bemeio = bevarmejo::io;
#include "bevarmejo/constants.hpp"
#include "bevarmejo/econometric_functions.hpp"
#include "bevarmejo/hydraulic_functions.hpp"

#include "bevarmejo/wds/water_distribution_system.hpp"
#include "bevarmejo/simulation/solvers/epanet/hydraulic.hpp"
#include "bevarmejo/problem/wds_problem.hpp"

#include "problems/anytown.hpp"
#include "problems/anytown_systol25.hpp"

namespace bevarmejo
{
namespace anytown_systol25
{

namespace io::formulation_name
{
static const std::string hr = "hyd_rel";
static const std::string mr = "mec_rel";
static const std::string fr = "fire_rel";
} // namespace io::formulation_name

namespace io::key
{
static constexpr bemeio::AliasedKey at_eps_inp {"Anytown eps inp"}; // "Anytown eps inp"
}

static const std::string hr__exinfo = "Hydraulic Reliability and Operational Efficiency Perspective";
static const std::string mr__exinfo = "Mechanical Reliability Perspective";
static const std::string fr__exinfo = "Firefighting Reliability Perspective";

static const std::string city_pipes__subnet_name = "city_pipes";
static const std::vector<std::string> city_pipes__el_names = {"2", "3", "4", "27", "28", "29", "30", "31", "32", "33", "34", "35", "37", "38", "41"};
static const std::string exis_pipes__subnet_name = "existing_pipes";
static const std::vector<std::string> exis_pipes__el_names = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "11", "12", "17", "18", "19", "20", "21", "22", "23", "24", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "40", "41"};
static const std::string new_pipes__subnet_name = "new_pipes";
static const std::vector<std::string> new_pipes__el_names = {"110", "113", "114", "115", "116", "125"};
static const std::string pos_tank_loc__subnet_name = "possible_tank_locations";
static const std::vector<std::string> pos_tank_loc__el_names = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "15", "16", "18", "19"};

static const std::vector<bevarmejo::anytown::exi_pipe_option> exi_pipe_options{
    { 6, 26.2, 14.2, 17.0, 12.0 },
    { 8, 27.8, 19.8, 17.0, 12.0 },
    { 10, 34.1, 25.1, 17.0, 12.0 },
    { 12, 41.4, 32.4, 17.0, 13.0 },
    { 14, 50.2, 40.2, 18.2, 14.2 },
    { 16, 58.5, 48.5, 19.8, 15.5 },
    { 18, 66.2, 57.2, 21.6, 17.1 },
    { 20, 76.8, 66.8, 23.5, 20.2 },
    { 24, 109.2, 85.5, 30.1, 1000000 },
    { 30, 142.5, 116.1, 41.3, 1000000 }
};

static const std::vector<bevarmejo::anytown::new_pipe_option> new_pipe_options{
    { 6, 12.8},
    { 8, 17.8},
    { 10, 22.5},
    { 12, 29.2},
    { 14, 36.2},
    { 16, 43.6},
    { 18, 51.5},
    { 20, 60.1},
    { 24, 77.0},
    { 30, 105.5}
};

static const std::vector<bevarmejo::anytown::tank_option> tank_options{
    {50000, 115000},
    {100000, 145000},
    {250000, 325000},
    {500000, 425000},
    {1000000, 600000}
};

static const std::vector<double> pump_group_operations{
    3, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2,
    2, 2, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3
};

static constexpr double max_velocity__m_per_s = 1.5;

Problem::Problem(std::string_view a_ud_formulation, const Json& settings, const bemeio::Paths& lookup_paths)
{
    if (a_ud_formulation == io::formulation_name::hr)
    {
        m__formulation = Formulation::hr;
        m__extra_info = hr__exinfo;   
    }
    else if (a_ud_formulation == io::formulation_name::mr)
    {
        m__formulation = Formulation::mr;
        m__extra_info = mr__exinfo;
    }
    else if (a_ud_formulation == io::formulation_name::fr)
    {
        m__formulation = Formulation::fr;
        m__extra_info = fr__exinfo;
    }
    else
    {
        beme_throw(std::invalid_argument, "Impossible to construct the Anytown Problem.",
        "The provided Anytown SysTol25 formulation is not recognised.",
        "Formulation: ", a_ud_formulation);
    }
    m__name = bemeio::log::nname::beme_l+problem_name+"::"+std::string(a_ud_formulation);

    load_networks(settings, lookup_paths);
    
    load_other_data(settings, lookup_paths);

    m__dv_adapter.reconfigure(this->get_continuous_dvs_mask());    
}

void Problem::load_networks(const Json& settings, const bemeio::Paths& lookup_paths)
{
    assert(settings != nullptr && 
        io::key::at_eps_inp.exists_in(settings)
    );
    const auto inp_filename = settings.at(io::key::at_eps_inp.as_in(settings)).get<fsys::path>();

    // Use "locate_file" to find the inp file in the paths.
    // EPANET does not load correctly the a curve so we need to fix it "manually" here.
    m__anytown = std::make_shared<WDS>(
        bemeio::locate_file</*log = */true>(inp_filename, lookup_paths),
        [](EN_Project ph) {
            // change curve ID 2 to a pump curve
            assert(ph != nullptr);
            std::string curve_id = "2";
            int curve_idx = 0;
            int errorcode = EN_getcurveindex(ph, curve_id.c_str(), &curve_idx);
            assert(errorcode <= 100);
    
            errorcode = EN_setcurvetype(ph, curve_idx, EN_PUMP_CURVE);
            assert(errorcode <= 100);
        }
    );

    // The EPANET object was correctly created so the filepath was correct
    m__anytown_filename = inp_filename.string();

    // Add the standard subnetworks necessary for this problem and one for temporary elements.
    m__anytown->submit_id_sequence(city_pipes__subnet_name, city_pipes__el_names);
    m__anytown->submit_id_sequence(exis_pipes__subnet_name, exis_pipes__el_names);
    m__anytown->submit_id_sequence(new_pipes__subnet_name, new_pipes__el_names);
    m__anytown->submit_id_sequence(pos_tank_loc__subnet_name, pos_tank_loc__el_names);
    m__anytown->submit_id_sequence(label::__temp_elems);

    // Set up the operations if it doesn't need to optimize them.
    assert(m__anytown->n_pumps() == 3);
    for (int i = 0; i < 3; ++i) {
        // no need to go through the pumps, I know the pattern ID
        int pattern_idx = 0;
        int errorcode = EN_getpatternindex(m__anytown->ph_, std::to_string(i+2).c_str(), &pattern_idx);
        assert(errorcode <= 100);

        std::vector<double> temp(k__hours_per_day, 0.0);
        for (int j = 0; j < k__hours_per_day; ++j) {
            temp[j] = pump_group_operations[j] > i ? 1.0 : 0.0;
        }

        errorcode = EN_setpattern(m__anytown->ph_, pattern_idx, temp.data(), temp.size());
        assert(errorcode <= 100);
    }

    // Now remains to decide what to do for the FireFlow case here...
}

void Problem::load_other_data(const Json& settings, const bemeio::Paths& lookup_paths)
{
    // For now nothing to do here...
    return;
}


// PAGMO FUNCTIONS

auto Problem::get_nobj() const -> std::vector<double>::size_type
{
	return 2ul; 
}

auto Problem::get_nec() const -> std::vector<double>::size_type
{
	return 0ul;
}

auto Problem::get_nic() const -> std::vector<double>::size_type
{
	return 0ul;
}

auto Problem::get_nix() const -> std::vector<double>::size_type
{
	auto mask = get_continuous_dvs_mask();
	return std::count(mask.begin(), mask.end(), false);
}

auto Problem::get_continuous_dvs_mask() const -> std::vector<bool>
{
    // All of these formulations have only integer decision variable.
    // The only thing that changes is how many, which depends on the formulation.
    std::size_t s = (
        exis_pipes__el_names.size()+ // 35
        new_pipes__el_names.size()+ // 6
        bevarmejo::anytown::max_n_installable_tanks*4); // 8

    if (m__formulation == Formulation::hr)
    {
        s += k__hours_per_day;
    }
    return std::vector<bool>(s, false);
}

auto Problem::fitness(
	const std::vector<double>& pagmo_dv
) const -> std::vector<double>
{
    // This function need to return the fitness function.
    // Let's pre-allocate in case something doesn't work out.
	std::vector<double> fitv(get_nobj()+get_nec()+get_nic(), std::numeric_limits<double>::max());

    // First thing first reconvert back from the pagmo ordering to the beme one.
	const auto dvs = m__dv_adapter.from_pagmo_to_beme(pagmo_dv);

    apply_dv(dvs);

	sim::solvers::epanet::HydSimSettings settings;

	long h_step = 0;
    int errorcode = EN_gettimeparam(m__anytown->ph(), EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long r_step = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_DURATION, &horizon);
    assert(errorcode < 100);

	settings.resolution(h_step);
	settings.report_resolution(r_step);
	settings.horizon(horizon);

	const auto results = sim::solvers::epanet::solve_hydraulics(*m__anytown, settings);

	if (!sim::solvers::epanet::is_successful_with_warnings(results))
	{
		bemeio::stream_out( std::cerr, "Error in the hydraulic simulation. \n");
		reset_dv(dvs);
		return std::move(fitv);
	}

    // Objective function 1: 
    // NET PRESENT COST
    fitv[0] = cost(dvs);

    // Objective function 2:
    // It is divided in 3 parts:
    // A: [2 -> 1]
    // Unfeasible solutions in the EPS: share of unfeasible timesteps.
    // B: [1 -> 0]
    // Feasible solutions in the EPS: EPS constraint violations
    //      - total normalized pressure deficit
    //      - max velocity normalized
    // C: [0 -> -1]
    // CUSTOMIZED PART!
    // Feasible solutions in the EPS and satisfying the constraint violations.
    // 
    double value = std::numeric_limits<double>::max();

    // Part A
    const auto n_steps = results.size();
    const auto n_correct_steps = std::count(results.values().begin(), results.values().end(), 0);

    if (n_correct_steps < n_steps)
    {
        // Kind of like the error in the hyd simulation.
        reset_dv(dvs);
        fitv[1] = 1.0 + ((double)n_steps - (double)n_correct_steps) / (double)n_steps;
        return std::move(fitv);
    }

    // Part B
    
    // Get the cumulative deficit of all junctions and normalize it if EPS.
	const auto normdeficit_daily = pressure_deficiency(*m__anytown, bevarmejo::anytown::min_pressure_psi*MperFT/PSIperFT, /*relative=*/ true);
	auto pressure_violation = normdeficit_daily.integrate_forward();
    pressure_violation = (normdeficit_daily.back().first != 0) ? (pressure_violation / normdeficit_daily.back().first) : pressure_violation;

    // Get the maximum pipe velocity violation of the constraint
    double observed_max_velocity = 0.0;
	for (const auto& [id, pipe] : m__anytown->pipes())
	{
		for (const auto& vel : pipe.velocity().values())
		{
			if (vel > observed_max_velocity)
			{
				observed_max_velocity = vel;
			}
		}
	}
	auto velocity_violation = (observed_max_velocity <= max_velocity__m_per_s) ? 0.0 : (observed_max_velocity - max_velocity__m_per_s) / max_velocity__m_per_s;
	
    // We must do the average of the violations because we want it to be at 0 when both are at 0.
    auto total_violation = (pressure_violation+velocity_violation) / 2.0;
    
    if (total_violation > 0.0)
    {
        // No need to do extra simulations, simply return the violation as this solution is not good.
        reset_dv(dvs);
        fitv[1] = total_violation;
        return std::move(fitv);
    }

    // We calculated anything we need for objective function 1 and 2 that is common between all formulations,
    // Now, we need to specialize.
    switch (m__formulation)
    {
    case Formulation::hr:
        fitv[1] = hydraulic_reliability_perspective();
        break;

    case Formulation::mr:
        fitv[1] = mechanical_reliability_perspective();
        break;

    case Formulation::fr:
        fitv[1] = firefighting_reliability_perspective();
        break;
    
    default:
        break;
    }

    reset_dv(dvs);
    return std::move(fitv);
}

auto Problem::apply_dv(const std::vector<double>& dvs) const -> void
{
    m__anytown->cache_indices();

    // Helper Lambda to iterate over the dv
    std::size_t i = 0.0;
	auto extract_next = [&dvs, &i](std::size_t n) { return std::vector(dvs.begin()+i, dvs.begin()+i+n); };

    bevarmejo::anytown::fep2::apply_dv__exis_pipes(
        *m__anytown,
        __old_HW_coeffs,
        extract_next(exis_pipes__el_names.size()),
        exi_pipe_options
    );
    i += exis_pipes__el_names.size();
    
    bevarmejo::anytown::apply_dv__new_pipes(
        *m__anytown,
        extract_next(new_pipes__el_names.size()),
        new_pipe_options
    );
    i += new_pipes__el_names.size();

    bevarmejo::anytown::fnt3::apply_dv__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*4), // 8
        tank_options,
        new_pipe_options
    );
    i += bevarmejo::anytown::max_n_installable_tanks*4;

    // Operations are optimized only in the hydraulic reliability and operational efficiency perspective
    if (m__formulation == Formulation::hr)
    {
        bevarmejo::anytown::apply_dv__pumps(
            *m__anytown,
            extract_next(k__hours_per_day)
        );
        i += k__hours_per_day;
    }

}

auto Problem::cost(const std::vector<double>& dvs) const -> double
{
    // Capital cost of interventions plus operational cost
    double capital_cost = 0.0;
	
	std::size_t i = 0;
	auto extract_next = [&dvs, &i](std::size_t n) { return std::vector(dvs.begin()+i, dvs.begin()+i+n); };

    capital_cost += bevarmejo::anytown::fep2::cost__exis_pipes(
        *m__anytown,
        extract_next(exis_pipes__el_names.size()),
        exi_pipe_options
    );
    i += exis_pipes__el_names.size();

    capital_cost += bevarmejo::anytown::cost__new_pipes(
        *m__anytown,
        extract_next(new_pipes__el_names.size()),
        new_pipe_options
    );
    i += new_pipes__el_names.size();

    capital_cost += bevarmejo::anytown::fnt3::cost__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*4),
        tank_options,
        new_pipe_options
    );
    i += bevarmejo::anytown::max_n_installable_tanks*4;

    double energy_cost_per_day = bevarmejo::anytown::cost__energy_per_day(*m__anytown);
	double yearly_energy_cost = energy_cost_per_day * bevarmejo::k__days_ina_year;

    // NPV requires initial capital investment to be positive when exiting
    // but the cash flows positive when entering. 
    // It returns positive when you earn something, but we want the cost as the word implies, so the opposite.
    return -bevarmejo::net_present_value(
        capital_cost,
        bevarmejo::anytown::discount_rate,
        -yearly_energy_cost,
        bevarmejo::anytown::amortization_years);

}

auto Problem::hydraulic_reliability_perspective() const -> double
{
    assertm(m__formulation == Formulation::hr, "This functions should be run only for the hr formulation");

    beme_throw(std::runtime_error, "Error",
        "The function is not implemented yet");

    return 0.0;
}

auto Problem::mechanical_reliability_perspective() const -> double
{
    assertm(m__formulation == Formulation::mr, "This functions should be run only for the mr formulation");

    beme_throw(std::runtime_error, "Error",
        "The function is not implemented yet");

    return 0.0;
}

auto Problem::firefighting_reliability_perspective() const -> double
{
    assertm(m__formulation == Formulation::fr, "This functions should be run only for the fr formulation");

    beme_throw(std::runtime_error, "Error",
        "The function is not implemented yet");

    return 0.0;
}

auto Problem::reset_dv(const std::vector<double>& dvs) const -> void
{
    m__anytown->cache_indices();

    // Helper Lambda to iterate over the dv
    std::size_t i = 0.0;
    auto extract_next = [&dvs, &i](std::size_t n) { return std::vector(dvs.begin()+i, dvs.begin()+i+n); };

    bevarmejo::anytown::fep2::reset_dv__exis_pipes(
        *m__anytown,
        extract_next(exis_pipes__el_names.size()),
        __old_HW_coeffs
    );
    i += exis_pipes__el_names.size();
    
    bevarmejo::anytown::reset_dv__new_pipes(
        *m__anytown,
        extract_next(new_pipes__el_names.size())
    );
    i += new_pipes__el_names.size();

    bevarmejo::anytown::fnt3::reset_dv__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*4)
    );
    i += bevarmejo::anytown::max_n_installable_tanks*4;

    // Operations are optimized only in the hydraulic reliability and operational efficiency perspective
    if (m__formulation == Formulation::hr)
    {
        bevarmejo::anytown::reset_dv__pumps(
            *m__anytown,
            extract_next(k__hours_per_day)
        );
        i += k__hours_per_day;
    }       
}

auto Problem::save_solution(
    const std::vector<double>& pagmo_dv,
    const fsys::path& out_file
) const -> void
{
    auto dvs = m__dv_adapter.from_pagmo_to_beme(pagmo_dv);

	apply_dv(dvs);

	int errco = EN_saveinpfile(this->m__anytown->ph_, out_file.string().c_str());
	assert(errco <= 100);

    // Eventually other save... 

	reset_dv(dvs);
}

auto to_json(Json& j, const bevarmejo::anytown_systol25::Problem& prob) -> void
{
    // Reset, just in case.
	j = Json{};
		
	j[io::key::at_eps_inp()] = prob.m__anytown_filename;

	j["extra_info"] = prob.get_extra_info();
}

// TODO:
// [ ] fnt3 (bounds, apply, cost, reset)

} // namespace anytown_systol25
} // namespace bevarmejo
