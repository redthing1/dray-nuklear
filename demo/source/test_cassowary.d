module test_cassowary;

import cassowary;

unittest {
    // first, create a solver:
    amcw_Solver* S = amcw_newsolver(null, null);

    // create some variable:
    amcw_Var* l = amcw_newvariable(S);
    amcw_Var* m = amcw_newvariable(S);
    amcw_Var* r = amcw_newvariable(S);

    // create the constraint: 
    amcw_Constraint* c1 = amcw_newconstraint(S, AMCW_REQUIRED);
    amcw_Constraint* c2 = amcw_newconstraint(S, AMCW_REQUIRED);

    // c1: m is in middle of l and r:
    //     i.e. m = (l + r) / 2, or 2*m = l + r
    amcw_addterm(c1, m, 2.0);
    amcw_setrelation(c1, AMCW_EQUAL);
    amcw_addterm(c1, l, 1.0);
    amcw_addterm(c1, r, 1.0);
    // apply c1
    amcw_add(c1);

    // c2: r - l >= 100
    amcw_addterm(c2, r, 1.0);
    amcw_addterm(c2, l, -1.0);
    amcw_setrelation(c2, AMCW_GREATEQUAL);
    amcw_addconstant(c2, 100.0);
    // apply c2
    amcw_add(c2);

    // now we set variable l to 20
    amcw_suggest(l, 20.0);

    // and see the value of m and r:
    amcw_updatevars(S);

    // r should by 20 + 100 == 120:
    assert(amcw_value(r) == 120.0);

    // and m should in middle of l and r:
    assert(amcw_value(m) == 70.0);

    // done with solver
    amcw_delsolver(S);
}
