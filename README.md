# Simplex
A C++ implementation of the Simplex Algorithm.

This code is still not able to work with equality constraints. Instead, you have to replace them with two inequality constraints that are equivalent to the equality one. For instance, if your constraint is 2x1 + 3x2 = 2, you have to replace it with both 2x1 + 3x2 <= 2 and 2x1 + 3x2 >= 2.

The examples provided are organized as follows:

Number of Variables
Number of Constraints
maximize objective function
Constraint 1
Constraint 2
Constraint 3
...
Constraint N

Notice that the code only works with the maximization of the objective function. If your problem concerns a minimization approach, just replace the objective function with its negative value. For instance, if your objective function is 3x1 + 2x2 and you want to minimize it, you just have to maximize -3x1 - 2x2.
