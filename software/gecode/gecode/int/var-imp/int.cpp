/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2002
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

namespace Gecode { namespace Int {

  forceinline bool
  IntVarImp::closer_min(int n) const {
    unsigned int l = static_cast<unsigned int>(n - dom.min());
    unsigned int r = static_cast<unsigned int>(dom.max() - n);
    return l < r;
  }

  int
  IntVarImp::med(void) const {
    // Computes the median
    if (fst() == nullptr)
      return (dom.min()+dom.max())/2 - ((dom.min()+dom.max())%2 < 0 ? 1 : 0);
    unsigned int i = size() / 2;
    if (size() % 2 == 0)
      i--;
    const RangeList* p = nullptr;
    const RangeList* c = fst();
    while (i >= c->width()) {
      i -= c->width();
      const RangeList* n=c->next(p); p=c; c=n;
    }
    return c->min() + static_cast<int>(i);
  }

  bool
  IntVarImp::in_full(int m) const {
    if (closer_min(m)) {
      const RangeList* p = nullptr;
      const RangeList* c = fst();
      while (m > c->max()) {
        const RangeList* n=c->next(p); p=c; c=n;
      }
      return (m >= c->min());
    } else {
      const RangeList* n = nullptr;
      const RangeList* c = lst();
      while (m < c->min()) {
        const RangeList* p=c->prev(n); n=c; c=p;
      }
      return (m <= c->max());
    }
  }

  /*
   * "Standard" tell operations
   *
   */

  ModEvent
  IntVarImp::lq_full(Space& home, int m) {
    assert((m >= dom.min()) && (m <= dom.max()));
    int old_max = dom.max();
    ModEvent me = ME_INT_BND;
    if (range()) { // Is already range...
      dom.max(m);
      if (assigned()) me = ME_INT_VAL;
    } else if (m < fst()->next(nullptr)->min()) { // Becomes range...
      dom.max(std::min(m,fst()->max()));
      fst()->dispose(home,nullptr,lst());
      fst(nullptr); holes = 0;
      if (assigned()) me = ME_INT_VAL;
    } else { // Stays non-range...
      RangeList* n = nullptr;
      RangeList* c = lst();
      unsigned int h = 0;
      while (m < c->min()) {
        RangeList* p = c->prev(n); c->fix(n);
        h += (c->min() - p->max() - 1);
        n=c; c=p;
      }
      holes -= h;
      int max_c = std::min(m,c->max());
      dom.max(max_c); c->max(max_c);
      if (c != lst()) {
        n->dispose(home,lst());
        c->next(n,nullptr); lst(c);
      }
    }
    IntDelta d(dom.max()+1,old_max);
    return notify(home,me,d);
  }

  ModEvent
  IntVarImp::gq_full(Space& home, int m) {
    assert((m >= dom.min()) && (m <= dom.max()));
    int old_min = dom.min();
    ModEvent me = ME_INT_BND;
    if (range()) { // Is already range...
      dom.min(m);
      if (assigned()) me = ME_INT_VAL;
    } else if (m > lst()->prev(nullptr)->max()) { // Becomes range...
      dom.min(std::max(m,lst()->min()));
      fst()->dispose(home,nullptr,lst());
      fst(nullptr); holes = 0;
      if (assigned()) me = ME_INT_VAL;
    } else { // Stays non-range...
      RangeList* p = nullptr;
      RangeList* c = fst();
      unsigned int h = 0;
      while (m > c->max()) {
        RangeList* n = c->next(p); c->fix(n);
        h += (n->min() - c->max() - 1);
        p=c; c=n;
      }
      holes -= h;
      int min_c = std::max(m,c->min());
      dom.min(min_c); c->min(min_c);
      if (c != fst()) {
        fst()->dispose(home,p);
        c->prev(p,nullptr); fst(c);
      }
    }
    IntDelta d(old_min,dom.min()-1);
    return notify(home,me,d);
  }

  ModEvent
  IntVarImp::eq_full(Space& home, int m) {
    dom.min(m); dom.max(m);
    if (!range()) {
      bool failed = false;
      RangeList* p = nullptr;
      RangeList* c = fst();
      while (m > c->max()) {
        RangeList* n=c->next(p); c->fix(n); p=c; c=n;
      }
      if (m < c->min())
        failed = true;
      while (c != nullptr) {
        RangeList* n=c->next(p); c->fix(n); p=c; c=n;
      }
      assert(p == lst());
      fst()->dispose(home,p);
      fst(nullptr); holes = 0;
      if (failed)
        return fail(home);
    }
    IntDelta d;
    return notify(home,ME_INT_VAL,d);
  }

  ModEvent
  IntVarImp::nq_full(Space& home, int m) {
    assert(!((m < dom.min()) || (m > dom.max())));
    ModEvent me = ME_INT_DOM;
    if (range()) {
      if ((m == dom.min()) && (m == dom.max()))
        return fail(home);
      if (m == dom.min()) {
        dom.min(m+1);
        me = assigned() ? ME_INT_VAL : ME_INT_BND;
      } else if (m == dom.max()) {
        dom.max(m-1);
        me = assigned() ? ME_INT_VAL : ME_INT_BND;
      } else {
        RangeList* f = new (home) RangeList(dom.min(),m-1);
        RangeList* l = new (home) RangeList(m+1,dom.max());
        f->prevnext(nullptr,l);
        l->prevnext(f,nullptr);
        fst(f); lst(l); holes = 1;
      }
    } else if (m < fst()->next(nullptr)->min()) { // Concerns the first range...
      int f_max = fst()->max();
      if (m > f_max)
        return ME_INT_NONE;
      int f_min = dom.min();
      if ((m == f_min) && (m == f_max)) {
        RangeList* f_next = fst()->next(nullptr);
        dom.min(f_next->min());
        if (f_next == lst()) { // Turns into range
          // Works as at the ends there are only nullptr pointers
          fst()->dispose(home,f_next);
          fst(nullptr); holes = 0;
          me = assigned() ? ME_INT_VAL : ME_INT_BND;
        } else { // Remains non-range
          f_next->prev(fst(),nullptr);
          fst()->dispose(home); fst(f_next);
          holes -= dom.min() - f_min - 1;
          me = ME_INT_BND;
        }
      } else if (m == f_min) {
        dom.min(m+1); fst()->min(m+1);
        me = ME_INT_BND;
      } else if (m == f_max) {
        fst()->max(m-1); holes += 1;
      } else {
        // Create new hole
        RangeList* f = new (home) RangeList(f_min,m-1);
        f->prevnext(nullptr,fst());
        fst()->min(m+1); fst()->prev(nullptr,f);
        fst(f); holes += 1;
      }
    } else if (m > lst()->prev(nullptr)->max()) { // Concerns the last range...
      int l_min = lst()->min();
      if (m < l_min)
        return ME_INT_NONE;
      int l_max = dom.max();
      if ((m == l_min) && (m == l_max)) {
        RangeList* l_prev = lst()->prev(nullptr);
        dom.max(l_prev->max());
        if (l_prev == fst()) {
          // Turns into range
          l_prev->dispose(home,lst());
          fst(nullptr); holes = 0;
          me = assigned() ? ME_INT_VAL : ME_INT_BND;
        } else { // Remains non-range
          l_prev->next(lst(),nullptr);
          lst()->dispose(home); lst(l_prev);
          holes -= l_max - dom.max() - 1;
          me = ME_INT_BND;
        }
      } else if (m == l_max) {
        dom.max(m-1); lst()->max(m-1);
        me = ME_INT_BND;
      } else if (m == l_min) {
        lst()->min(m+1); holes += 1;
      } else { // Create new hole
        RangeList* l = new (home) RangeList(m+1,l_max);
        l->prevnext(lst(),nullptr);
        lst()->max(m-1); lst()->next(nullptr,l);
        lst(l); holes += 1;
      }
    } else { // Concerns element in the middle of the list of ranges
      RangeList* p;
      RangeList* c;
      RangeList* n;
      if (closer_min(m)) {
        assert(m > fst()->max());
        p = nullptr;
        c = fst();
        do {
          n=c->next(p); p=c; c=n;
        } while (m > c->max());
        if (m < c->min())
          return ME_INT_NONE;
        n=c->next(p);
      } else {
        assert(m < lst()->min());
        n = nullptr;
        c = lst();
        do {
          p=c->prev(n); n=c; c=p;
        } while (m < c->min());
        if (m > c->max())
          return ME_INT_NONE;
        p=c->prev(n);
      }
      assert((fst() != c) && (lst() != c));
      assert((m >= c->min()) && (m <= c->max()));
      holes += 1;
      int c_min = c->min();
      int c_max = c->max();
      if ((c_min == m) && (c_max == m)) {
        c->dispose(home);
        p->next(c,n); n->prev(c,p);
      } else if (c_min == m) {
        c->min(m+1);
      } else {
        c->max(m-1);
        if (c_max != m) {
          RangeList* l = new (home) RangeList(m+1,c_max);
          l->prevnext(c,n);
          c->next(n,l);
          n->prev(c,l);
        }
      }
    }
    IntDelta d(m,m);
    return notify(home,me,d);
  }



  /*
   * Copying variables
   *
   */

  forceinline
  IntVarImp::IntVarImp(Space& home, IntVarImp& x)
    : IntVarImpBase(home,x), dom(x.dom.min(),x.dom.max()) {
    holes = x.holes;
    if (holes) {
      int m = 1;
      // Compute length
      {
        RangeList* s_p = x.fst();
        RangeList* s_c = s_p->next(nullptr);
        do {
          m++;
          RangeList* s_n = s_c->next(s_p); s_p=s_c; s_c=s_n;
        } while (s_c != nullptr);
      }
      RangeList* d_c = home.alloc<RangeList>(m);
      fst(d_c); lst(d_c+m-1);
      d_c->min(x.fst()->min());
      d_c->max(x.fst()->max());
      d_c->prevnext(nullptr,nullptr);
      RangeList* s_p = x.fst();
      RangeList* s_c = s_p->next(nullptr);
      do {
        RangeList* d_n = d_c + 1;
        d_c->next(nullptr,d_n);
        d_n->prevnext(d_c,nullptr);
        d_n->min(s_c->min()); d_n->max(s_c->max());
        d_c = d_n;
        RangeList* s_n=s_c->next(s_p); s_p=s_c; s_c=s_n;
      } while (s_c != nullptr);
      d_c->next(nullptr,nullptr);
    } else {
      fst(nullptr);
    }
  }

  IntVarImp*
  IntVarImp::perform_copy(Space& home) {
    return new (home) IntVarImp(home,*this);
  }

  /*
   * Dependencies
   *
   */
  void
  IntVarImp::subscribe(Space& home, Propagator& p, PropCond pc,
                       bool schedule) {
    IntVarImpBase::subscribe(home,p,pc,dom.min()==dom.max(),schedule);
  }

  void
  IntVarImp::reschedule(Space& home, Propagator& p, PropCond pc) {
    IntVarImpBase::reschedule(home,p,pc,dom.min()==dom.max());
  }

  void
  IntVarImp::subscribe(Space& home, Advisor& a, bool fail) {
    IntVarImpBase::subscribe(home,a,dom.min()==dom.max(),fail);
  }

}}

// STATISTICS: int-var