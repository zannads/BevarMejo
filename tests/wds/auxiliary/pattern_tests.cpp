#include <gtest/gtest.h>

#include "epanet2_2.h"

#include "bevarmejo/wds/elements/element.hpp"
#include "bevarmejo/wds/auxiliary/pattern.hpp"

#include "bevarmejo/io/streams.hpp"

namespace bevarmejo {
namespace wds {
namespace test {

class PatternTest : public ::testing::Test {
protected:
    EN_Project ph;

    void SetUp() override {
        // Initialize EPANET project
        EN_createproject(&ph);
        std::string inp_file = "../../tests/anytown.inp";
        EN_open(ph, inp_file.c_str(), "", "");
    }

    void TearDown() override {
        EN_deleteproject(ph);
    }
};

TEST_F(PatternTest, ProgrammaticConstructors) {
    auto px = std::make_shared<Pattern>("px");
    
    ASSERT_EQ(px->id(), "px");
    ASSERT_TRUE(px->multipliers().empty());


}

TEST_F(PatternTest, EPANETConstructors) {

    // Using the Anytown network, load a pattern and then use this for copy and move constuctors
    auto p1 = std::make_shared<Pattern>("1");
    auto p2 = std::make_shared<Pattern>("2");

    p1->retrieve_index(ph);
    p2->retrieve_index(ph);

    // Print the indexes
    std::cout << "Pattern 1 index: " << p1->index() << std::endl;
    std::cout << "Pattern 2 index: " << p2->index() << std::endl;

    // Load the multipliers
    p1->retrieve_properties(ph);
    p2->retrieve_properties(ph);

    io::stream_out(std::cout, p1->multipliers());
    io::stream_out(std::cout, p2->multipliers());
}
    

} // namespace test
} // namespace wds
} // namespace bevarmejo