#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // For automatic conversion of std::vector to Python lists
#include "SFMLGame.hpp"
#include "action.hpp"

namespace py = pybind11;

PYBIND11_MODULE(pybind_sfml_game, m) {
    m.doc() = "SFML game environment module exposed via pybind11";

    // Bind the SFMLGame class.
    py::class_<SFMLGame>(m, "SFMLGame")
        .def(py::init<>())
        .def("reset", &SFMLGame::reset, "Reset the environment and return the initial state")
        .def("step", &SFMLGame::step, "Take an action and return (state, reward, done)")
        .def("get_state", &SFMLGame::get_state, "Return the current state")
        .def("set_agent_mode", &SFMLGame::set_agent_mode, "Set agent mode (true) or human mode (false)")
        .def("is_open", &SFMLGame::is_open, "Check if the window is still open");

    // Bind the Action struct.
    py::class_<Action>(m, "Action")
        .def(py::init<float, float, int>(),
            py::arg("delta_x") = 0.0f, py::arg("delta_y") = 0.0f, py::arg("shoot") = 0)
        .def_readwrite("delta_x", &Action::delta_x)
        .def_readwrite("delta_y", &Action::delta_y)
        .def_readwrite("shoot", &Action::shoot)
        .def("to_flat_array", &Action::toFlatArray, "Return a flat array representation of the action");

    // Expose the helper function to generate a random action.
    m.def("generate_random_action", &generateRandomAction, "Generate a random Action");
}
