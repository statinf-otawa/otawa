#include "State.h"

namespace global {
  
State::State() {
  //cout << " Construct State " << (void*)this << endl ;
  bottom = false ;
}
State::State(const State& s) {
  //cout << " Clone State " << (void*)this << " from " << &s << " valant " << s << endl ;
  *this = s ;
 // cout << "val " << *this << endl ;
}

bool State::getBottom() const {
  return bottom ;
}

void State::setBottom(bool b) {
  bottom = b ;
}

bool State::isBottom() const {
  return count() == 0 && bottom ;
}

Output& operator<< (Output& o, State const& s) {
  for(avl::Map<MemID, PotentialValue>::PairIterator it(s); it; it++)
    o << (*it).fst << " = " << (*it).snd << endl ;
  return o  ;
}

}  // global
