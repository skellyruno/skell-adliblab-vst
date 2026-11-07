// Minimal algorithm-only JUCE compatibility shim. NOT for building the plugin.
#pragma once
#include <algorithm>
#include <cmath>
#include <string>
namespace juce {
using String=std::string;
template<typename T>struct MathConstants{static constexpr T pi=(T)3.14159265358979323846;};
template<typename T>constexpr T jmin(T a,T b){return a<b?a:b;}
template<typename T>constexpr T jmax(T a,T b){return a>b?a:b;}
template<typename T>constexpr T jlimit(T lo,T hi,T x){return jmax(lo,jmin(hi,x));}
template<typename T>constexpr T jmap(T x,T a,T b){return a+(b-a)*x;}
template<typename T>constexpr T jmap(T x,T inputA,T inputB,T outputA,T outputB){return outputA+(x-inputA)*(outputB-outputA)/(inputB-inputA);}
template<typename T>constexpr bool isPositiveAndBelow(T v,T end){return v>=0&&v<end;}
}
