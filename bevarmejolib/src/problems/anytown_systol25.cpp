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
#include "bevarmejo/evaluation/metrics/hydraulic_functions.hpp"

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
static constexpr bemeio::AliasedKey at_ff_inp {"Anytown fireflow inp"}; // "Anytown fireflow inp"
static constexpr bemeio::AliasedKey opers {"Pump group operations"}; // "Pump group operations"
}

static const std::string hr__exinfo = "Hydraulic Reliability and Operational Efficiency Perspective";
static const std::string mr__exinfo = "Mechanical Reliability Perspective";
static const std::string fr__exinfo = "Firefighting Reliability Perspective";

Problem::Problem(std::string_view a_ud_formulation, const Json& settings, const bemeio::Paths& lookup_paths) :
    __old_HW_coeffs()
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
    m__anytown->submit_id_sequence(anytown::city_pipes__subnet_name, anytown::city_pipes__el_names);
    m__anytown->submit_id_sequence(anytown::exis_pipes__subnet_name, anytown::exis_pipes__el_names);
    m__anytown->submit_id_sequence(anytown::new_pipes__subnet_name, anytown::new_pipes__el_names);
    m__anytown->submit_id_sequence(anytown::pos_tank_loc__subnet_name, anytown::pos_tank_loc__el_names);
    m__anytown->submit_id_sequence(label::__temp_elems);

    // Prepare the simulation settings.
	long h_step = 0;
    int errorcode = EN_gettimeparam(m__anytown->ph(), EN_HYDSTEP, &h_step);
    assert(errorcode < 100);
    long horizon = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_DURATION, &horizon);
    assert(errorcode < 100);

	m__eps_settings.resolution(h_step).horizon(horizon);

    long r_step = 0;
    errorcode = EN_gettimeparam(m__anytown->ph(), EN_REPORTSTEP, &r_step);
    assert(errorcode < 100);

    m__eps_settings.report_resolution(r_step);

    // The mechanical reliability simulations is a instantaneous simulation
    // with 1.3 times the average demand. The anytown file starts at 18:00
    // with demand 1, thus we do a simulation with horizon 0.
    m__mrsim__settings.demand_multiplier(bevarmejo::anytown::peak_flow_multiplier)
        .report_resolution(r_step)
        .resolution(h_step)
        .horizon(0);
    
    if (m__formulation == Formulation::fr)
    {
        // Upload the second network and do more or less the same actions
        assert(settings != nullptr && 
            io::key::at_ff_inp.exists_in(settings)
        );
        const auto inp_filename = settings.at(io::key::at_ff_inp.as_in(settings)).get<fsys::path>();

        // Use "locate_file" to find the inp file in the paths.
        // EPANET does not load correctly the a curve so we need to fix it "manually" here.
        m__ff_anytown = std::make_shared<WDS>(
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
        m__ff_anytown_filename = inp_filename.string();

        // Add the standard subnetworks necessary for this problem and one for temporary elements.
        m__ff_anytown->submit_id_sequence(anytown::city_pipes__subnet_name, anytown::city_pipes__el_names);
        m__ff_anytown->submit_id_sequence(anytown::exis_pipes__subnet_name, anytown::exis_pipes__el_names);
        m__ff_anytown->submit_id_sequence(anytown::new_pipes__subnet_name, anytown::new_pipes__el_names);
        m__ff_anytown->submit_id_sequence(anytown::pos_tank_loc__subnet_name, anytown::pos_tank_loc__el_names);
        m__ff_anytown->submit_id_sequence(label::__temp_elems);

        errorcode = EN_gettimeparam(m__ff_anytown->ph(), EN_HYDSTEP, &h_step);
        assert(errorcode < 100);
        errorcode = EN_gettimeparam(m__ff_anytown->ph(), EN_DURATION, &horizon);
        assert(errorcode < 100);
        errorcode = EN_gettimeparam(m__ff_anytown->ph(), EN_REPORTSTEP, &r_step);
        assert(errorcode < 100);

        // We don't put the 1.3 multiplier because we expect the inp file to have done that already.
        // Otherwise, we would need to add the fireflow divided by 1.3.
        m__ffsim_settings.report_resolution(r_step)
            .pressure_driven_analysis(0.0, anytown::min_pressure_fireflow__psi/PSIperFT*MperFT, 0.5)
            .resolution(h_step)
            .horizon(horizon);
    }
}

void Problem::load_other_data(const Json& settings, const bemeio::Paths& lookup_paths)
{
    if (m__formulation == Formulation::hr)
    {
        return; // No need in hydraulic reliability
    }

    // By default we use the ones I set in this file
    auto operations = anytown::pump_group_operations;

    // If it is there, let's override
    if (io::key::opers.exists_in(settings))
    {
        auto j_oper = Json{}; // Json for the operations
        bemeio::expand_if_filepath(settings.at(io::key::opers.as_in(settings)), lookup_paths, 
            j_oper);

        operations = j_oper.get<std::vector<double>>();
    }
        
    assert(operations.size() == 24);

    // Set up the operations if it doesn't need to optimize them.
    assert(m__anytown->n_pumps() == 3);
    for (int i = 0; i < 3; ++i) {
        // no need to go through the pumps, I know the pattern ID
        int pattern_idx = 0;
        int errorcode = EN_getpatternindex(m__anytown->ph_, std::to_string(i+2).c_str(), &pattern_idx);
        assert(errorcode <= 100);

        std::vector<double> temp(k__hours_per_day, 0.0);
        for (int j = 0; j < k__hours_per_day; ++j) {
            temp[j] = operations[j] > i ? 1.0 : 0.0;
        }

        errorcode = EN_setpattern(m__anytown->ph_, pattern_idx, temp.data(), temp.size());
        assert(errorcode <= 100);
    }
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
        anytown::exis_pipes__el_names.size()+ // 35
        anytown::new_pipes__el_names.size()+ // 6
        anytown::max_n_installable_tanks*anytown::fnt3::n_dvs); // 8

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

	const auto results = sim::solvers::epanet::solve_hydraulics(*m__anytown, m__eps_settings);

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
	const auto normdeficit_daily = pressure_deficiency(*m__anytown, anytown::min_pressure__psi*MperFT/PSIperFT, /*relative=*/ true);
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
	auto velocity_violation = (observed_max_velocity <= anytown::max_velocity__m_per_s) ? 0.0 : (observed_max_velocity - anytown::max_velocity__m_per_s) / anytown::max_velocity__m_per_s;
	
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
        fitv[1] = -hydraulic_reliability_perspective();  // I want to maximize the reliability index
        break;

    case Formulation::mr:
        fitv[1] = -mechanical_reliability_perspective();
        break;

    case Formulation::fr:
        fitv[1] = -firefighting_reliability_perspective();
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
        extract_next(anytown::exis_pipes__el_names.size()),
        anytown::exi_pipe_options
    );
    i += anytown::exis_pipes__el_names.size();
    
    bevarmejo::anytown::apply_dv__new_pipes(
        *m__anytown,
        extract_next(anytown::new_pipes__el_names.size()),
        anytown::new_pipe_options
    );
    i += anytown::new_pipes__el_names.size();

    bevarmejo::anytown::fnt3::apply_dv__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs), // 8
        anytown::tank_options,
        anytown::new_pipe_options
    );
    i += bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs;

    // Operations are optimized only in the hydraulic reliability and operational efficiency perspective
    if (m__formulation == Formulation::hr)
    {
        bevarmejo::anytown::apply_dv__pumps(
            *m__anytown,
            extract_next(k__hours_per_day)
        );
        i += k__hours_per_day;
    }

    // In the firefighting case I have to apply the dvs also to the network used to simulate the fire events
    if (m__formulation == Formulation::fr)
    {
        m__ff_anytown->cache_indices();
        i = 0;
        bevarmejo::anytown::fep2::apply_dv__exis_pipes(
            *m__ff_anytown,
            __old_HW_coeffs,
            extract_next(anytown::exis_pipes__el_names.size()),
            anytown::exi_pipe_options
        );
        i += anytown::exis_pipes__el_names.size();
        
        bevarmejo::anytown::apply_dv__new_pipes(
            *m__ff_anytown,
            extract_next(anytown::new_pipes__el_names.size()),
            anytown::new_pipe_options
        );
        i += anytown::new_pipes__el_names.size();

        bevarmejo::anytown::fnt3::apply_dv__tanks(
            *m__ff_anytown,
            extract_next(bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs), // 8
            anytown::tank_options,
            anytown::new_pipe_options
        );
        i += bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs;

        // However, for the tanks, the min level must be moved to 0 so that we can simulate the fireflow events...
        // For each tank in the temp elements, set the min level to 0 and then the min volume
        // (EN doesn't change that automatically because it consider them independent)
        m__ff_anytown->cache_indices();
        auto& temp_elems = m__ff_anytown->id_sequence(label::__temp_elems);
        for (std::size_t i = anytown::max_n_installable_tanks; i; --i)
        {
            auto new_tank_id = std::string("T")+std::to_string(i-1);

            if (temp_elems.contains(new_tank_id))
            {
                auto& tank = m__ff_anytown->tank(new_tank_id);
                int errorcode = EN_setnodevalue(m__ff_anytown->ph(), tank.EN_index(), EN_MINLEVEL, 0.0);
                assert(errorcode <= 100);

                errorcode = EN_setnodevalue(m__ff_anytown->ph(), tank.EN_index(), EN_MINVOLUME, 0.0);
                assert(errorcode <= 100);

                tank.min_level(0.0);
                tank.min_volume(0.0);
            }
        }
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
        extract_next(anytown::exis_pipes__el_names.size()),
        anytown::exi_pipe_options
    );
    i += anytown::exis_pipes__el_names.size();

    capital_cost += bevarmejo::anytown::cost__new_pipes(
        *m__anytown,
        extract_next(anytown::new_pipes__el_names.size()),
        anytown::new_pipe_options
    );
    i += anytown::new_pipes__el_names.size();

    capital_cost += bevarmejo::anytown::fnt3::cost__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs),
        anytown::tank_options,
        anytown::new_pipe_options
    );
    i += bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs;

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

    // All constraints are satisfied, now check the reliability index
	const auto ir_daily = resilience_index_from_min_pressure(*m__anytown, bevarmejo::anytown::min_pressure__psi*MperFT/PSIperFT);
	auto value = ir_daily.integrate_forward();

	value = ir_daily.back().first != 0 ? value / ir_daily.back().first : value;

    return value;
}

auto Problem::mechanical_reliability_perspective() const -> double
{
    assertm(m__formulation == Formulation::mr, "This functions should be run only for the mr formulation");

    // We do the mr simulation, get the results and calculate the mechanical reliability estimator
    
    const auto results = sim::solvers::epanet::solve_hydraulics(*m__anytown, m__mrsim__settings);
    
    // If it fails hard (> 100) or can't satisfy the pressures for some reasons (> 0)
    // We return minimum reliability (0.0)
    if (!sim::solvers::epanet::is_successful(results))
	{
        return 0.0;
    }

    // If it worked, we simply calculate and return the mre..

    auto mre = eval::metrics::PaezFilion::mechanical_reliability_estimator(*m__anytown);

    // if only one value, simply return that
    if (mre.back().first == 0)
    {
        return mre.value();
    }
    
    return mre.integrate_forward()/ mre.back().first;
}

auto Problem::firefighting_reliability_perspective() const -> double
{
    assertm(m__formulation == Formulation::fr, "This functions should be run only for the fr formulation");

    // We calculate the firefighting reliability as the expected value of the ratio of supply over demand.
    // Therefore for each scenario, we calculate the aggregated values of supply, integrate over time.
    // We are assuming equal probability for each scenario as Anytown doesn't provide this
    // data.
    constexpr double pi = 1.0/anytown::fireflow_test_values.size();
    // Because of these assumptions, we can calculate it in real time...
    // Let's note that if a simulation fails, we regard the entire simulation as failed to not make the EA exploit weird behaviours that could emerge.
    double ff_rel = 0.0;
    
    for (const auto& ff_test : anytown::fireflow_test_values)
    {
        // 1. Apply the additional demand;
        // 2. simulate;
        // 3. extract the values of total supply and demand, add the contribution to the reliability;
        // 4. remove the additional demand.

        // Let's find where to apply it. We also assume that every Anytown node has 1 demand only (let's check to make sure we don't mess it up).
        // This will also help us in removing the demand after the simulation.
        auto ph = m__ff_anytown->ph();
        auto& junc = m__ff_anytown->junction(std::string(ff_test.junction_name));
        assert(junc.demands().size() == 1);
        assert([&]() -> bool {
            int n_demands = 0;
            int errorcode = EN_getnumdemands(ph, junc.EN_index(), &n_demands);
            return errorcode == 0 && n_demands == 1;  // Success code AND exactly one demand
        }());

        // Add a constand demand equal to the required fire flow.
        // I don't have the interface to add the demand to my class yet.
        int errorcode = EN_adddemand(ph, junc.EN_index(),
            ff_test.flow__gpm, "", "beme_fireflow");

        // 2. --------------------
        const auto results = sim::solvers::epanet::solve_hydraulics(*m__ff_anytown, m__ffsim_settings);

        // 3. --------------------
        // Extract the values, but only if it was actually fully feasible. Otherwise, highest penalty (aka + 0, aka do nothing)
        // It should not happen because it is PDA, but anyway...
        if (sim::solvers::epanet::is_successful(results))
        {
            auto total_d = eval::metrics::total_water_demand(*m__ff_anytown);
            auto total_c= eval::metrics::total_water_consumption(*m__ff_anytown);

            ff_rel += pi*total_c.integrate_forward()/total_d.integrate_forward();
        }

        // 4. -------------------
        // We just make sure in debug mode that it always adds it as the second one...
        assert([&]() -> bool {
            int dem_idx = 0;
            int errorcode = EN_getdemandindex(ph, junc.EN_index(), "beme_fireflow", &dem_idx);
            return errorcode == 0 && dem_idx == 2;
        }());
        errorcode = EN_deletedemand(ph, junc.EN_index(), 2);
        assert(errorcode <= 100);
    }

    return ff_rel;
}

auto Problem::reset_dv(const std::vector<double>& dvs) const -> void
{
    m__anytown->cache_indices();

    // Helper Lambda to iterate over the dv
    std::size_t i = 0.0;
    auto extract_next = [&dvs, &i](std::size_t n) { return std::vector(dvs.begin()+i, dvs.begin()+i+n); };

    bevarmejo::anytown::fep2::reset_dv__exis_pipes(
        *m__anytown,
        extract_next(anytown::exis_pipes__el_names.size()),
        __old_HW_coeffs
    );
    i += anytown::exis_pipes__el_names.size();
    
    bevarmejo::anytown::reset_dv__new_pipes(
        *m__anytown,
        extract_next(anytown::new_pipes__el_names.size())
    );
    i += anytown::new_pipes__el_names.size();

    bevarmejo::anytown::fnt3::reset_dv__tanks(
        *m__anytown,
        extract_next(bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs)
    );
    i += bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs;

    // Operations are optimized only in the hydraulic reliability and operational efficiency perspective
    if (m__formulation == Formulation::hr)
    {
        bevarmejo::anytown::reset_dv__pumps(
            *m__anytown,
            extract_next(k__hours_per_day)
        );
        i += k__hours_per_day;
    }

    if (m__formulation == Formulation::fr)
    {
        m__ff_anytown->cache_indices();
        i = 0.0;

        bevarmejo::anytown::fep2::reset_dv__exis_pipes(
            *m__ff_anytown,
            extract_next(anytown::exis_pipes__el_names.size()),
            __old_HW_coeffs
        );
        i += anytown::exis_pipes__el_names.size();
        
        bevarmejo::anytown::reset_dv__new_pipes(
            *m__ff_anytown,
            extract_next(anytown::new_pipes__el_names.size())
        );
        i += anytown::new_pipes__el_names.size();

        bevarmejo::anytown::fnt3::reset_dv__tanks(
            *m__ff_anytown,
            extract_next(bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs)
        );
        i += bevarmejo::anytown::max_n_installable_tanks*anytown::fnt3::n_dvs;
    }
}

auto Problem::get_bounds() const -> std::pair<std::vector<double>, std::vector<double>>
{
    std::vector<double> lb;
	std::vector<double> ub;

	const auto append_bounds= [&lb, &ub](auto&& bounds_func, auto&&... args) {
		auto [lower, upper] = bounds_func(std::forward<decltype(args)>(args)...);
		lb.insert(lb.end(), lower.begin(), lower.end());
		ub.insert(ub.end(), upper.begin(), upper.end());
	};

    append_bounds(bevarmejo::anytown::fep2::bounds__exis_pipes, std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>(anytown::exis_pipes__subnet_name), anytown::exi_pipe_options);

    append_bounds(bevarmejo::anytown::bounds__new_pipes, std::as_const(*m__anytown).subnetwork_with_order<WDS::Pipe>(anytown::new_pipes__subnet_name), anytown::new_pipe_options);

    append_bounds(bevarmejo::anytown::fnt3::bounds__tanks, std::as_const(*m__anytown).subnetwork_with_order<WDS::Junction>(anytown::pos_tank_loc__subnet_name), anytown::tank_options, anytown::new_pipe_options);

     // Operations are optimized only in the hydraulic reliability and operational efficiency perspective
    if (m__formulation == Formulation::hr)
    {
        append_bounds(bevarmejo::anytown::bounds__pumps, std::as_const(*m__anytown).pumps());
    }

    // We added the bound in the beme order, so now we use the adpater to map them to the pagmo order.
	return {m__dv_adapter.from_beme_to_pagmo(lb), m__dv_adapter.from_beme_to_pagmo(ub)};
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

    if (prob.m__formulation == Formulation::fr)
    {
        j[io::key::at_ff_inp()] = prob.m__ff_anytown_filename;
    }

	j["extra_info"] = prob.get_extra_info();
}

} // namespace anytown_systol25

} // namespace bevarmejo
