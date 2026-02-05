#include <type_traits>
#include <concepts>
#include <array>
#include <functional>
#include <iostream>
#include <tuple>
#include <variant>
#include <variant>
#include <optional>
// To do: be able to pass any kind of time: ms,us,s,...
// To do: change states with keywords
//To do create library and use it in a main.cpp add a CMakeFile
template<typename F>
concept Function = std::invocable<F>; // concept to check that is a callable function

template <typename S>
concept TypeState = std::is_class_v<S> && std::is_empty_v<S>; // concept to check that is a empty object needed for safety reasons when creating a state

template<typename T>
concept TupleLike = requires{
    typename std::tuple_size<T>::type;
};


template<Function F>
class Cyclic_action{
    private:
        size_t period;
        size_t last_time = 0;
        const F action;
        void execute() const{
            action();
        }
    public:
        constexpr Cyclic_action(F action,size_t period):action(action),period(period){}
        void tick(size_t current_time){
            if(current_time - last_time >= period){
                execute();
                last_time = current_time;
            }
        }
        void set_period(size_t period){
            this->period = period;
        }
        size_t get_period(){
            return period;
        }
        
};
//trait to check if is part of the class Cyclic action. For any type std::value = false_type,
// if receive a function F and is from the clas Cyclic_action value = std::true_type

template<typename T>
struct is_cyclic_action : std::false_type{};

template<Function F>
struct is_cyclic_action<Cyclic_action<F>> : std::true_type{};


//trait to check that every element from a tuple is a cyclic action
//This works the next way: any type will be false by default,
//If the type can be convertible to a tuple and all the types of the tuple satisfy the restriction of is_cyclic_action this will be true
template<typename T>
struct all_cyclic_action : std::false_type{};

template<typename... Ts>
struct all_cyclic_action<std::tuple<Ts...>> : std::bool_constant<(is_cyclic_action<Ts>::value && ...)>{};

template <typename T>
concept TupleCyclicAction = all_cyclic_action<T>::value;

struct isStateMachine{};


template<TypeState To,typename Transition_Function>
requires std::predicate<Transition_Function>
class Transition{
    private:
    Transition_Function check; 
    public:
    using to = To;
    constexpr Transition(Transition_Function check):check(check){};
    std::optional<To> check_transition() const{
        if(check()) return To{};
        return std::nullopt;
    } 
};

template<typename T>
concept HasNestedMachine = !std::is_void_v<T>;

template <TypeState St,Function F_enter,Function F_exit,TupleCyclicAction tuple,typename NestedMachine = void,typename... Transitions> 
requires (std::is_void_v<NestedMachine> || NestedMachine::is_state_machine)
class State{
    private:
        tuple cyclics;  
        const F_exit exit_action;
        const F_enter enter_action;
        const std::tuple<Transitions...> transitions;
        using nested_type = std::conditional_t<
            std::is_void_v<NestedMachine>,
            std::monostate,
            NestedMachine
        >;

        nested_type nested_machine;
        using transition_result = std::variant<St,typename Transitions::to...>;
    public:
        using state = St;
        using transitions_type = std::tuple<Transitions...>;
        constexpr State(const F_exit& exit,const F_enter& enter,tuple& cyclics,nested_type& nested,Transitions&... trans)
        requires(!std::is_void_v<NestedMachine>):
        cyclics(cyclics),exit_action(exit),enter_action(enter), nested_machine(nested),transitions(std::forward<Transitions>(trans)...)
        {}
        
        constexpr State(const F_exit& exit,const F_enter& enter,tuple& cyclics,Transitions&... trans)
        requires(std::is_void_v<NestedMachine>):
        cyclics(cyclics),exit_action(exit),enter_action(enter),transitions(std::forward<Transitions>(trans)...)
        {}
        void constexpr exit(){
            if constexpr (!std::is_void_v<NestedMachine>){
                nested_machine.deactivate();
            }
            exit_action();
        }
        void constexpr enter(){
            enter_action();
            if constexpr (!std::is_void_v<NestedMachine>){
                nested_machine.Start();
            }
        }
        void tick(size_t time){
            std::apply([&](auto&... c){
                (c.tick(time),...);
            },cyclics);
            if constexpr(!std::is_void_v<NestedMachine>){
                nested_machine.update(time);
            }
        }
        template<typename Possible_states>
        Possible_states check_transitions(){
            Possible_states result{St{}};
            bool taken = false;
            std::apply([&](auto const&... t){
            (([&]{
                if(!taken){
                    if(auto r = t.check_transition()){
                        result = *r;
                        taken = true;
                    }
                }
            }()),...);
            }, transitions);
            return result;
        }
};

//Trait to check if a class is a state or not
template<typename T>
struct is_State : std::false_type {};

template <TypeState St,Function F_enter,Function F_exit,TupleCyclicAction tuple,typename NestedMachine,typename... Transitions> 
struct is_State<State<St,F_enter,F_exit,tuple,NestedMachine,Transitions...>> : std::true_type {};

template<typename T>
concept IsState = is_State<T>::value;

//trait to check if the transitions goes to a valid state
template <typename S>
using state_type_t = typename S::state;

//Check if a state is inside a template of states
template<typename S, typename... tuple>
struct contains : std::bool_constant<(std::is_same_v<S,tuple> || ...)>{}; 

template<typename StateT, typename Tuple>
concept ValidTransitions =
    []<typename... Ts, typename... ValidStates>
    (std::tuple<Ts...>*, std::tuple<ValidStates...>*) {
        return (contains<typename Ts::to, ValidStates...>::value && ...);
    }(
        static_cast<typename StateT::transitions_type*>(nullptr),
        static_cast<Tuple*>(nullptr)
    );


template<typename InitialState,typename...OtherStates>
class StateMachine{ 
    
    using valid_states = std::tuple<state_type_t<InitialState>,state_type_t<OtherStates>...>;
    
    
    static_assert(IsState<InitialState>,"Your initial state is not a valid State");
    static_assert((IsState<OtherStates> && ...), "Yours sates must be State<> types");

   
    static_assert(ValidTransitions<InitialState,valid_states>,"Invalid transition");
    static_assert((ValidTransitions<OtherStates,valid_states> && ...),"Invalid transition");
    
 

    std::tuple<InitialState,OtherStates...> states;
    using StateVariant = std::variant<typename InitialState::state,typename OtherStates::state...>;
    StateVariant current_state;
    
    public:
    static constexpr bool is_state_machine = true;
    constexpr StateMachine(InitialState& ini,OtherStates&... otherstate):
        states(ini,otherstate...),current_state(typename InitialState::state{}) {}
    
    template<typename St>
    auto& get_state_object(){
        return std::get<[]<std::size_t... I>(std::index_sequence<I...>){
            constexpr std::size_t index = ((std::is_same_v<typename std::tuple_element_t<I,decltype(states)>::state,St> ? I : 0) + ...);
            return index;
        }(std::make_index_sequence<std::tuple_size_v<decltype(states)>>())>(states);
    }
    //to check the actual state
    template<typename St>
    bool is() const{
        return std::holds_alternative<St>(current_state);
    }
    void Start(){
        std::visit([&](auto current_st){
            using St = decltype(current_st);
            auto& state_obj = get_state_object<St>();
            state_obj.enter();
        },current_state);
    }
    void update(size_t current_time){
        std::visit([&](auto current_st){
            using St = decltype(current_st);
            auto& state_obj = get_state_object<St>();
            state_obj.tick(current_time);
            auto next = state_obj.template check_transitions<StateVariant>();
            if(!std::holds_alternative<St>(next)){
                state_obj.exit();
                std::visit([&](auto new_st){
                    using St = decltype(new_st);
                    auto& next_obj = get_state_object<decltype(new_st)>();
                    next_obj.enter();
                },next); 
                current_state = next;
            }
           
        },current_state);
    }
    void deactivate(){
        std::visit([&](auto current_st){
            using St = decltype(current_st);
            auto& state_obj = get_state_object<St>();
            state_obj.exit();
            current_state = typename InitialState::state{}; 
        },current_state);
    }
};
//Actions initial
void initial_enter_action(){
    std::cout<< "initial enter action\n";
}
void initial_cyclic_action(){
    std::cout<< "initial cyclic action 1\n";
}
void initial_cyclic_action2(){
    std::cout << "initial cyclic action 2\n";
}
void initial_exit_action(){
    std::cout<< "Saliendo de initial\n";
}

//Operational Actions
void Operational_enter_action(){
    std::cout<< "Operational enter action\n";
}
void Operational_cyclic_action(){
    std::cout<< "Operational cyclic action 1\n";
}
void Operational_cyclic_action2(){
    std::cout << "Operational cyclic action 2\n";
}
void Operational_exit_action(){
    std::cout<< "Saliendo de Operational\n";
}
// Inactive Actions
void Inactive_enter_action(){
    std::cout<< "Inactive enter action\n";
}
void Inactive_cyclic_action(){
    std::cout<< "Inactive cyclic action 1\n";
}
void Inactive_cyclic_action2(){
    std::cout << "Inactive cyclic action 2\n";
}
void Inactive_exit_action(){
    std::cout<< "Saliendo de Inactive\n";
}

//Active Actions
void Active_enter_action(){
    std::cout<< "Active enter action\n";
}
void Active_cyclic_action(){
    std::cout<< "Active cyclic action 1\n";
}
void Active_cyclic_action2(){
    std::cout << "Active cyclic action 2\n";
}
void Active_exit_action(){
    std::cout<< "Saliendo de Active\n";
}
int val = 1;
//transitions
bool Initial_to_operational(){
    return !(val % 10);
}
bool Operational_to_Initial(){
    return !(val % 10);
}
bool Inactive_to_Active(){
    return !(val % 3);
}
bool Active_to_Inactive(){
    return !(val % 3);
}
class Initial{};
class Operational{};
class Active{};
class Inactive{};

Transition<Operational, decltype(&Initial_to_operational)> t1{Initial_to_operational};
Transition<Initial, decltype(&Operational_to_Initial)> t2{Operational_to_Initial};

Transition<Active,decltype(&Inactive_to_Active)> t3{Inactive_to_Active};
Transition<Inactive,decltype(&Active_to_Inactive)> t4{Active_to_Inactive};

Cyclic_action c_init1{initial_cyclic_action, 1000};
Cyclic_action c_init2{initial_cyclic_action2, 2000};

Cyclic_action c_op1{Operational_cyclic_action, 1000};
Cyclic_action c_op2{Operational_cyclic_action2, 2000};

Cyclic_action c_act1{Active_cyclic_action,1000};
Cyclic_action c_act2{Active_cyclic_action2,2000};

Cyclic_action c_inac1{Inactive_cyclic_action,1000};
Cyclic_action c_inac2{Inactive_cyclic_action2,2000};

auto init_cyclics = std::make_tuple(c_init1);
auto op_cyclics   = std::make_tuple(c_op1);
auto inac_cyclics = std::make_tuple(c_inac1);
auto ac_cyclics = std::make_tuple(c_act1);
//STATES and submachine
State<
    Inactive,
    decltype(&Inactive_enter_action),
    decltype(&Inactive_exit_action),
    decltype(inac_cyclics),
    void,
    decltype(t3)> inactive_state{
    Inactive_exit_action,
    Inactive_enter_action,
    inac_cyclics,
    t3
};

State<
    Active,
    decltype(&Active_enter_action),
    decltype(&Active_exit_action),
    decltype(ac_cyclics),
    void,
    decltype(t4)> Active_state{
    Active_exit_action,
    Active_enter_action,
    ac_cyclics,
    t4
};

StateMachine smNested{inactive_state,Active_state};


State<
    Initial,
    decltype(&initial_enter_action),
    decltype(&initial_exit_action),
    decltype(init_cyclics),
    void,
    decltype(t1)> initial_state{
    initial_exit_action,
    initial_enter_action,
    init_cyclics,
    t1
};

State<
    Operational,
    decltype(&Operational_enter_action),
    decltype(&Operational_exit_action),
    decltype(op_cyclics),
    decltype(smNested),
    decltype(t2)
> operational_state{
    Operational_exit_action,
    Operational_enter_action,
    op_cyclics,
    smNested,
    t2
};

StateMachine sm{ initial_state, operational_state };


int main(){
    sm.Start();
    for(val = 1; val < 50; val++){
        std::cout << val << "\n";
        sm.update(val*1000);
    }
    return 0;
}