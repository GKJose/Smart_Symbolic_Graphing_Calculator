
#include <catch2/catch_all.hpp>

#include <iostream>
#include <ticks.hxx>

void printall(std::vector<double>& vec){
    for (auto& v: vec)
        std::cout << v << " ";
    std::cout << "\n";
}

TEST_CASE ("Rounding [rounding]"){
    using namespace RoundExt;
    SECTION("sigdigits"){
        REQUIRE(jround<double>(-25.0, OptNone, Option<int>(2), OptNone) == -25.0);
    }
}


TEST_CASE ("Ticks [ticks]"){
    using namespace Ticks;
    TickInfo info = optimize_ticks(-100.0, 100.0);
    SECTION("postdecimal digits") {
        REQUIRE(postdecimal_digits(1.0) == 0);
        REQUIRE(postdecimal_digits(5.0) == 0);
        REQUIRE(postdecimal_digits(2.0) == 0);
        REQUIRE(postdecimal_digits(2.5) == 1);
        REQUIRE(postdecimal_digits(3.0) == 0);
    }

    SECTION("optimize ticks") {
        TickInfo info = optimize_ticks(-100.0, 100.0);
        REQUIRE(info.ticks.size() == 5);
        REQUIRE(info.ticks[0] == -100.0);
        REQUIRE(info.ticks[1] == -50.0);
        REQUIRE(info.ticks[2] == 0.0);
        REQUIRE(info.ticks[3] == 50.0);
        REQUIRE(info.ticks[4] == 100.0);
        REQUIRE(info.x_min == -100.0);
        REQUIRE(info.x_max == 100.0);

        info = optimize_ticks(-25.0, 25.0);
        printall(info.ticks);
        REQUIRE(info.ticks.size() == 3);
        REQUIRE(info.ticks[0] == -25.0);
        REQUIRE(info.ticks[1] == 0.0);
        REQUIRE(info.ticks[2] == 25.0);
        REQUIRE(info.x_min == -25.0);
        REQUIRE(info.x_max == 25.0);

        info = optimize_ticks(100.0, 350.0);
        REQUIRE(info.ticks.size() == 6);
        REQUIRE(info.ticks[0] == 100.0);
        REQUIRE(info.ticks[1] == 150.0);
        REQUIRE(info.ticks[2] == 200.0);
        REQUIRE(info.ticks[3] == 250.0);
        REQUIRE(info.ticks[4] == 300.0);
        REQUIRE(info.ticks[5] == 350.0);
        REQUIRE(info.x_min == 100.0);
        REQUIRE(info.x_max == 350.0);

        info = optimize_ticks(-520.0, 110.0);
        REQUIRE(info.ticks.size() == 2);
        REQUIRE(info.ticks[0] == -300.0);
        REQUIRE(info.ticks[1] == 0.0);
        REQUIRE(info.x_min == -520.0);
        REQUIRE(info.x_max == 110.0);

        // std::cout << "TICKS: ";
        // for (auto& tick : info.ticks){
        //     std::cout << tick << " ";
        // }
        // std::cout << "\n";
    }
    
}