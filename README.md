A C++ program to simulate the cellular automaton in three dimensions. Finds the best scalability (between OpenMP and MPI) that can be achieved.

In the standard game, cells that have exactly F<sub>l</sub> = F<sub>u</sub> = 6 neighbors are born in the next time step.

Cells that have E<sub>l</sub> = 5 or E<sub>u</sub> = 7 living neighbors will survive to the next time step.

Cells that only fewer than E<sub>l</sub> = 5 neighbors will die before the next time step from loneliness.

Cells that have more than E<sub>u</sub> = 7 neighbors, will die before the next time step from overpopulation (or lack of resources).

The game, if uses the above rule, then 
(E<sub>l</sub>,E<sub>u</sub>,F<sub>l</sub>,F<sub>u</sub>) = (5,7,6,6)


For the boundary of the playing field, different types of behavior can be considered. Either the boundary consists of dead cells (simpler) or the board is made circular (periodic boundary conditions).

