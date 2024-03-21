#ifndef BEVARMEJOLIB__ECONOMETRIC_FUNCTIONS_HPP
#define BEVARMEJOLIB__ECONOMETRIC_FUNCTIONS_HPP

#include <cmath>
#include <vector>

namespace bevarmejo {

// net present value (NPV)
inline double net_present_value(const double initial_investment, const double discount_rate, const std::vector<double>& cash_flows) {
    double npv = -initial_investment;
    for (std::size_t i = 0; i < cash_flows.size(); ++i) {
        npv += cash_flows[i] / std::pow(1 + discount_rate, i + 1);
    }
    return npv;
}

inline double net_present_value(const double initial_investment, const double discount_rate, const double cash_flow, const int number_of_periods = 1){
    std::vector<double>cash_flows(number_of_periods, cash_flow);
    return bevarmejo::net_present_value(initial_investment, discount_rate, cash_flows);
}

} /* namespace bevarmejo */

#endif /* BEVARMEJOLIB__ECONOMETRIC_FUNCTIONS_HPP */