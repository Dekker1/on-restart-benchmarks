/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2012
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int.hh>

namespace Gecode {

  IntAction::IntAction(Home home, const IntVarArgs& x, double d,
                       bool p, bool f, IntBranchMerit bm) {
    ViewArray<Int::IntView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }

  void
  IntAction::init(Home home, const IntVarArgs& x, double d,
                  bool p, bool f, IntBranchMerit bm) {
    ViewArray<Int::IntView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }


  BoolAction::BoolAction(Home home, const BoolVarArgs& x, double d,
                         bool p, bool f, BoolBranchMerit bm) {
    ViewArray<Int::BoolView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }

  void
  BoolAction::init(Home home, const BoolVarArgs& x, double d,
                   bool p, bool f, BoolBranchMerit bm) {
    ViewArray<Int::BoolView> y(home,x);
    Action::init(home,y,d,p,f,bm);
  }

}

// STATISTICS: int-branch
