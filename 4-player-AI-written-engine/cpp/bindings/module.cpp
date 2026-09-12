#include "hokm/engine.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
using namespace hokm;
PYBIND11_MODULE(_core, m) {
    py::enum_<Suit>(m, "Suit")
        .value("CLUBS", Suit::Clubs)
        .value("DIAMONDS", Suit::Diamonds)
        .value("HEARTS", Suit::Hearts)
        .value("SPADES", Suit::Spades);
    py::enum_<Phase>(m, "Phase")
        .value("CHOOSE_TRUMP", Phase::ChooseTrump)
        .value("PLAY", Phase::Play)
        .value("ROUND_OVER", Phase::RoundOver)
        .value("MATCH_OVER", Phase::MatchOver);
    py::class_<ChooseTrump>(m, "ChooseTrump")
        .def(py::init<Suit>())
        .def_readonly("suit", &ChooseTrump::suit);
    py::class_<PlayCard>(m, "PlayCard").def(py::init<Card>()).def_readonly("card", &PlayCard::card);
    py::class_<PlayedCard>(m, "PlayedCard")
        .def_readonly("player", &PlayedCard::player)
        .def_readonly("card", &PlayedCard::card);
    py::class_<RoundResult>(m, "RoundResult")
        .def_readonly("winner", &RoundResult::winner)
        .def_readonly("points", &RoundResult::points)
        .def_readonly("tricks", &RoundResult::tricks)
        .def_readonly("hakem", &RoundResult::hakem);
    auto o = py::class_<Observation>(m, "Observation");
#define OBS(field) o.def_readonly(#field, &Observation::field)
    OBS(player);
    OBS(team);
    OBS(hakem);
    OBS(dealer);
    OBS(trump);
    OBS(actor);
    OBS(leader);
    OBS(phase);
    OBS(hand);
    OBS(trick);
    OBS(history);
    OBS(hand_sizes);
    OBS(tricks);
    OBS(score);
    OBS(result);
    OBS(match_winner);
    OBS(legal_mask);
#undef OBS
    auto s = py::class_<State>(m, "DebugState");
#define STATE(field) s.def_readonly(#field, &State::field)
    STATE(phase);
    STATE(hakem);
    STATE(trump);
    STATE(actor);
    STATE(leader);
    STATE(cursor);
    STATE(hands);
    STATE(deck);
    STATE(trick);
    STATE(trick_size);
    STATE(history);
    STATE(history_size);
    STATE(tricks);
    STATE(score);
    STATE(result);
    STATE(match_winner);
    STATE(initial_seating);
#undef STATE
    py::class_<Engine>(m, "Engine")
        .def(py::init<std::uint64_t, int>(), py::arg("seed") = 0, py::arg("initial_hakem") = -1)
        .def("reset", &Engine::reset, py::arg("seed"), py::arg("initial_hakem") = -1)
        .def_property_readonly("phase", &Engine::phase)
        .def_property_readonly("current_actor", &Engine::current_actor)
        .def("legal_actions", &Engine::legal_actions)
        .def("legal_action_mask", &Engine::legal_action_mask)
        .def("apply", &Engine::apply)
        .def("apply_id", &Engine::apply_id)
        .def("observe", &Engine::observe)
        .def("start_next_round", &Engine::start_next_round)
        .def_property_readonly("round_result", &Engine::round_result)
        .def_property_readonly("match_winner", &Engine::match_winner)
        .def("clone", &Engine::clone)
        .def("debug_state", &Engine::debug_state)
        .def("load_deal", &Engine::load_deal, py::arg("deck"), py::arg("hakem"),
             py::arg("score") = std::array<int, 2>{0, 0}, py::arg("future_seed") = 0)
        .def("check_invariants", &Engine::check_invariants);
    m.def("encode", &encode);
    m.def("decode", &decode);
    m.def("card_suit", &suit);
    m.def("card_rank", &rank);
}
