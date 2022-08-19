# Todo
- [X] Start doxygen documentation
- [X] Remove _current_cmd on dev structure (global internal variable for module)
- [X] Remove fsm_state on dev structure (global internal variable for module)
- [X] Get Mac address and device name infos on Dump command
- [X] Add API function to return the FSM state
- [X] Split parsing of dump infos string
- [X] Add API to get services
- [X] Add function to change fsm state with virtual module
- [X] Add API function to return if module is on Transparent Uart (or not)
- [X] Rework virtual module (split functions)
- [X] Add API function to return the module's mode (Data or Command mode)
- [X] Add log management on the driver
- [ ] Work on the FSM state
- [ ] Add graphical dependencies on doxygen
- [ ] Automate generation of HTML report file for test coverage
- [ ] Improve memory management
- [ ] Improve log file management with CMake
- [ ] Improve framework (Doxygen and test coverage location)
- [ ] Revert commit with Gatt features
# Bugs
- [X] strtok_r do not work on rn4871ResponseProcess function
=> Fix: this function process only the module response (if correct or not - no parsing)
- [X] Get firmware version : incorrect parsing
=> Fix: a specific function parse the whole response string to extract data
- [X] rn4871SetServices doesn't modify internal value on virtual module
- [ ] Potential Risk: If sequence "$$$" is not properly done due to hardware module