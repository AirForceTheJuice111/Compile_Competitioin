#ifndef _TrExp_HH
#define _TrExp_HH

#include <iostream> // IWYU pragma: keep
#include <variant>  // IWYU pragma: keep
#include "temp.hh"
#include "treep.hh"

using namespace std;

// This is for patchlist
class Patch_list { // a list of tree::Label* that need to be backpatched to the same label, used for true/false lists in conditional jumps. eg. for if (a < b) { ... } else { ... }, the true_list of the condition will have the label for the then branch, and the false_list will have the label for the else branch. When we generate code for the condition, we don't know what those labels are yet, so we add "placeholder" labels to the patch list. Then when we generate code for the branches, we can patch all those placeholder labels to the actual labels for the then/else branches.
  public:
    vector<tree::Label *> *patch_list;
    Patch_list() {
        patch_list = new vector<tree::Label *>();
    }
    ~Patch_list() {
        delete patch_list;
    }
    void add_patch(tree::Label *label) { // this label should be named -1
        patch_list->push_back(label);
    }
    void patch(tree::Label *label) { // patch all labels in this patch list to the given label (by setting their num field)
        for (auto l : *patch_list)
            l->num = label->num;
    }
    // concatenate another patch list into this patch list
    void add(Patch_list *p2) {
        patch_list->insert(patch_list->end(), p2->patch_list->begin(), p2->patch_list->end());
    }
};

class Tr_ex; // Tiger_IR expression wrapper, can be converted to Ex, Cx, or Nx
class Tr_nx; // Tiger_IR statement wrapper, can be converted to Ex, Cx, or Nx
class Tr_cx; // Tiger_IR conditional jump wrapper, can be converted to Ex, Cx, or Nx

class Tr_Exp {
  public:
    virtual Tr_ex *unEx(Temp_map *tm) = 0; // convert to Ex wrapper (expression)
    virtual Tr_cx *unCx(Temp_map *tm) = 0; // convert to Cx wrapper (conditional jump)
    virtual Tr_nx *unNx(Temp_map *tm) = 0; // convert to Nx wrapper (statement)
};

// for Tr_Exp classes
class Tr_ex : public Tr_Exp {
  public:
    tree::Exp *exp;
    Tr_ex(tree::Exp *e) {
        exp = e;
    }
    Tr_cx *unCx(Temp_map *tm) override;
    Tr_nx *unNx(Temp_map *tm) override;
    Tr_ex *unEx(Temp_map *tm) override;
};

class Tr_nx : public Tr_Exp {
  public:
    tree::Stm *stm;
    Tr_nx(tree::Stm *s) {
        stm = s;
    }
    Tr_cx *unCx(Temp_map *tm) override;
    Tr_nx *unNx(Temp_map *tm) override;
    Tr_ex *unEx(Temp_map *tm) override;
};

class Tr_cx : public Tr_Exp { // conditional jump wrapper, contains true/false patch lists and the conditional jump statement
  public:
    Patch_list *true_list;
    Patch_list *false_list;
    tree::Stm *stm; // the tree::Cjump statement that will jump to the true label if condition is true, and jump to the false label if condition is false. The labels in this statement will be patched later using the patch lists.
    Tr_cx(Patch_list *t, Patch_list *f, tree::Stm *s) {
        true_list = t;
        false_list = f;
        stm = s;
    }
    Tr_ex *unEx(Temp_map *tm) override;
    Tr_nx *unNx(Temp_map *tm) override;
    Tr_cx *unCx(Temp_map *tm) override;
};

#endif
