/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2008
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

#include <gecode/kernel.hh>

namespace Gecode {

  Region::Pool::Pool(void)
    : c(new Chunk), n_c(2U) {
    c->next = new Chunk; c->next->next = nullptr;
  }
  Region::Chunk*
  Region::Pool::chunk(void) {
    Chunk* n;
    {
      Support::Lock l(m);
      if (c != nullptr) {
        assert(n_c > 0U);
        n = c; c = c->next; n_c--;
      } else {
        n = new Region::Chunk;
      }
      n->reset();
    }
    return n;
  }
  void
  Region::Pool::chunk(Chunk* u) {
    Support::Lock l(m);
    if (n_c == Kernel::MemoryConfig::n_hc_cache) {
      delete u;
    } else {
      u->next = c; c = u;
      n_c++;
    }
  }
  Region::Pool::~Pool(void) {
    Support::Lock l(m);
    // If that were the case there would be a memory leak!
    assert(c != nullptr);
    do {
      Chunk* n=c->next;
      delete c;
      c=n;
    } while (c != nullptr);
  }

  Region::Pool& Region::pool(void) {
    thread_local static Region::Pool _p;
    return _p;
  }

  void*
  Region::heap_alloc(size_t s) {
    void* p = heap.ralloc(s);
    if (hi == nullptr) {
      hi = p;
      assert(!Support::marked(hi));
    } else if (!Support::marked(hi)) {
      HeapInfo* h = static_cast<HeapInfo*>
        (heap.ralloc(sizeof(HeapInfo)+(4-1)*sizeof(void*)));
      h->n=2; h->size=4;
      h->blocks[0]=hi; h->blocks[1]=p;
      hi = Support::mark(h);
    } else {
      HeapInfo* h = static_cast<HeapInfo*>(Support::unmark(hi));
      if (h->n == h->size) {
        HeapInfo* n = static_cast<HeapInfo*>
          (heap.ralloc(sizeof(HeapInfo)+(2*h->n-1)*sizeof(void*)));
        n->size = 2*h->n;
        n->n = h->n;
        memcpy(&n->blocks[0], &h->blocks[0], h->n*sizeof(void*));
        hi = Support::mark(n);
        heap.rfree(h);
        h = n;
      }
      h->blocks[h->n++] = p;
    }
    return p;
  }

  void
  Region::heap_free(void) {
    assert(hi != nullptr);
    if (Support::marked(hi)) {
      HeapInfo* h = static_cast<HeapInfo*>(Support::unmark(hi));
      for (unsigned int i=0U; i<h->n; i++)
        heap.rfree(h->blocks[i]);
      heap.rfree(h);
    } else {
      heap.rfree(hi);
    }
  }

}

// STATISTICS: kernel-memory
