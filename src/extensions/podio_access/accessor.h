// Created by Dmitry Romanov
// Subject to the terms in the LICENSE file found in the top-level directory.
//

#pragma once

/** @example
 *
 *  struct X {
 *  int get_member () const { return member; };
 *  private:
 *      int member = 0;
 *  };
 *
 *  ALLOW_ACCESS(X, member, int);
 *
 *  int main() {
 *      X x;
 *      ACCESS(x, member) = 42; // write 42 to it
 *      std::cout << "proof: " << x.get_member() << std::endl;
 *  }
 */

#include<iostream>
#define CONCATE_(X, Y) X##Y
#define CONCATE(X, Y) CONCATE_(X, Y)
#define ALLOW_ACCESS(CLASS, MEMBER, ...) \
  template<typename Only, __VA_ARGS__ CLASS::*Member> \
  struct CONCATE(MEMBER, __LINE__) { friend __VA_ARGS__ CLASS::*Access(Only*) { return Member; } }; \
  template<typename> struct Only_##MEMBER; \
  template<> struct Only_##MEMBER<CLASS> { friend __VA_ARGS__ CLASS::*Access(Only_##MEMBER<CLASS>*); }; \
  template struct CONCATE(MEMBER, __LINE__)<Only_##MEMBER<CLASS>, &CLASS::MEMBER>
#define ACCESS(OBJECT, MEMBER) \
  (OBJECT).*Access((Only_##MEMBER<std::remove_reference<decltype(OBJECT)>::type>*)nullptr)


