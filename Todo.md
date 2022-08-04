# Todo
- [X] Start doxygen documentation
- [ ] Add graphical dependencies on doxygen
- [X] Remove _current_cmd on dev structure (global internal variable for module)
- [X] Remove fsm_state on dev structure (global internal variable for module)
- [ ] Add function to change fsm state with virtual module
    - [ ] Function for simulating "connected" bluetooth
    - [ ] Function for simulating "streaming" bluetooth
    - [ ] Function for simulating "disconnected" bluetooth
- [X] Add API function to return the FSM state
- [ ] Add API function to return if module is on Transparent Uart (or not)
- [ ] Get Mac address and device name infos on Dump command
- [ ] Split rn4871ResponseProcess on two functions => for command responding and data receive
- [ ] Generate HTML report file for test coverage
- [ ] Revert commit with Gatt features
# Bugs
- [ ] strtok_r do not work on rn4871ResponseProcess function
- [ ] Get firmware version : incorrect parsing
# Limitations