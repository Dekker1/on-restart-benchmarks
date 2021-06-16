#include <chuffed/core/propagator.h>

// y = |ub(x) - lb(y) + 1|
class RangeSize : public Propagator {
public:
    IntVar* x;
    IntVar* y;

    RangeSize(IntVar* _x, IntVar* _y) : x(_x), y(_y) {
        priority = 1;
        x->attach(this, 0, EVENT_LU);
    }

    bool propagate() {
        if (y->getMin() < 1)
            setDom((*y), setMin, 1, y->getMinLit());
        setDom((*y), setMax, x->getMax() - x->getMin() + 1, x->getMinLit(), x->getMaxLit());
        return true;
    }
};

void range_size(IntVar* x, IntVar* y) {
    new RangeSize(x, y);
}

class LastVal : public Propagator {
public:
    IntVar* x;
    int* v;

    LastVal(IntVar* _x, int* _v) : x(_x), v(_v) {
        priority = 0;
        x->attach(this, 0, EVENT_F);
    }

    void wakeup(int i, int c) {
        assert(x->isFixed());
        pushInQueue();
    }

    bool propagate() {
        (*v) = x->getVal();
        return true;
    }
};

void last_val(IntVar* x, int* v) {
    new LastVal(x, v);
}