from util import ASTTransformer
from fennec_ast import Type, Operator, VarDef, ArrayDef, Assignment, Modification, \
        If, Block, VarUse, BinaryOp, IntConst, Return, DoWhileLoop, ForLoop, WhileLoop, ExprStatement


class Desugarer(ASTTransformer):
    def __init__(self):
        self.varcache_stack = [{}]

    def makevar(self, name):
        # Generate a variable starting with an underscore (which is not allowed
        # in the language itself, so should be unique). To make the variable
        # unique, add a counter value if there are multiple generated variables
        # of the same name within the current scope.
        # A variable can be tagged as 'ssa' which means it is only assigned once
        # at its definition.
        name = '_' + name
        varcache = self.varcache_stack[-1]
        occurrences = varcache.setdefault(name, 0)
        varcache[name] += 1
        return name if not occurrences else name + str(occurrences + 1)

    def visitFunDef(self, node):
        self.varcache_stack.append({})
        self.visit_children(node)
        self.varcache_stack.pop()

    def visitModification(self, m):
        # from: lhs op= rhs
        # to:   lhs = lhs op rhs
        self.visit_children(m)
        return Assignment(m.ref, BinaryOp(m.ref, m.op, m.value)).at(m)
    
    def visitForLoop(self, m):
        self.visit_children(m)
        
        var_ind_use = VarUse(m.ind_var.name)
        end_var = VarDef(Type.get("int"), self.makevar("end"), m.end)
        while_cond = BinaryOp(var_ind_use, Operator("<"), VarUse(end_var.name))

        var_ind_inc = Assignment(var_ind_use, BinaryOp(var_ind_use, Operator("+"), IntConst(1)))
       
        return [end_var, Block([
            m.ind_var,
            WhileLoop(while_cond, Block([
                m.body,
                var_ind_inc
            ]), True, var_ind_inc)
        ])]
