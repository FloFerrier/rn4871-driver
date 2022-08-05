# Todo
- [X] Start doxygen documentation
- [X] Remove _current_cmd on dev structure (global internal variable for module)
- [X] Remove fsm_state on dev structure (global internal variable for module)
- [X] Get Mac address and device name infos on Dump command
- [X] Add API function to return the FSM state
- [X] Split parsing of dump infos string
- [X] Add API to get services
- [X] Add function to change fsm state with virtual module
- [ ] Work on the FSM state
- [ ] Add API function to return if module is on Transparent Uart (or not)
- [ ] Add graphical dependencies on doxygen
- [ ] Generate HTML report file for test coverage
- [ ] Revert commit with Gatt features
- [ ] Rework virtual module (split functions)
# Bugs
- [X] strtok_r do not work on rn4871ResponseProcess function
=> Fix: this function process only the module response (if correct or not - no parsing)
- [X] Get firmware version : incorrect parsing
=> Fix: a specific function parse the whole response string to extract data
- [X] rn4871SetServices doesn't modify internal value on virtual module