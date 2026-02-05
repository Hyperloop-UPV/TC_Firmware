#include <bits/stdc++.h>
#define FAST
class math{
    public:
    template<std::size_t N>
    static consteval  std::size_t factorial(){
        std::size_t fact = N;
        for(int i = N -1; i > 0;i--){
            fact *= i;
        }
        return fact;
    }
    template<float num,int potencia>
    static consteval float pow(){
        float result = 1;
        for(int i = 0; i < potencia;i++){
            result *= num;
        }
        for(int i = 0; i > potencia;i--){
            result /= num;
        }
        return result;
    }
    template<std::size_t N>
    static constexpr std::size_t n_suma(){
        #ifndef FAST
            std::size result = 0;
            for(int i = 1; i <= N; i++){
                result += i;
            }
            return result;
        #else
            return (N+1)*(N/2);
        #endif 
    }
};

int main(){
    constexpr std::array<std::size_t,5> factoriales{math::factorial<1>(),math::factorial<2>(),math::factorial<3>(),math::factorial<4>(),math::factorial<5>()};
    constexpr float potencia_positiva = math::pow<static_cast<float>(3.0),4>();
    constexpr float potencia_negativa = math::pow<static_cast<float>(3.0),-1>();
    constexpr std::size_t suma_10 = math::n_suma<10>();
    std::size_t* ptr_100 = new std::size_t;
    *ptr_100 = math::n_suma<100>();
    std::cout<<"Primeros 5 factoriales"<<std::endl;
    for(const auto& f : factoriales){
        std::cout << f << " ";
    }
    std::cout << std::endl << " 3 elevado a 4: "<< potencia_positiva << "    3 elevado a -1: " <<potencia_negativa;
    std::cout<<std::endl<<"Suma de los primeros 10 numeros naturales: " << suma_10 << std::endl << "Suma de los primeros 100 numeros naturales: " << *ptr_100 << std::endl;
    delete ptr_100;
    return 0;
}

